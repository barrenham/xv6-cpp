#include "tests/process_test_hooks.h"
#include <cstdio>
#include <atomic>
#include <mutex>
#include <thread>
#include <cstdlib>
#include <type_traits>
#include <unordered_map>
#include <sys/wait.h>
#include <unistd.h>

// The real resource manager uses only opaque file/inode references. Native
// substitutes count references and assert the filesystem transaction boundary.
import xv.process;

using namespace xv::process;
using file = xv::fs::File;
using inode = xv::fs::Inode;
static std::unordered_map<const void *, int> reference_counts;

template <class T> int &references(T *object)
{
  return reference_counts.try_emplace(object, 1).first->second;
}

static_assert(std::is_abstract_v<Scheduler>);
static_assert(std::is_abstract_v<ExecutableLoader>);
static_assert(!std::is_copy_constructible_v<Process>);
static_assert(!std::is_copy_constructible_v<DescriptorTable>);
static_assert(!std::is_copy_constructible_v<WorkingDirectory>);
// Process now owns a shared exec snapshot; recycling invokes its destructor.
static_assert(!std::is_trivially_destructible_v<Process>);
constinit DescriptorTable boot_table;
constinit WorkingDirectory boot_directory;
static bool transaction = false;
static std::mutex reference_mutex;

static void check(bool condition, const char *message)
{
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}

extern "C" void *test_retain_file(void *reference)
{
  std::lock_guard guard(reference_mutex);
  check(reference && references(reference) > 0, "duplicate live file reference");
  ++references(reference);
  return reference;
}

extern "C" void test_release_file(void *reference)
{
  std::lock_guard guard(reference_mutex);
  check(reference && references(reference) > 0, "close owned file reference");
  --references(reference);
}

extern "C" void *test_retain_inode(void *reference)
{
  std::lock_guard guard(reference_mutex);
  check(reference && references(reference) > 0, "duplicate live inode reference");
  ++references(reference);
  return reference;
}

extern "C" void test_release_inode(void *reference)
{
  std::lock_guard guard(reference_mutex);
  check(transaction, "directory reference released within transaction");
  check(reference && references(reference) > 0, "drop owned inode reference");
  --references(reference);
}

extern "C" [[noreturn]] void test_resource_fail(const char *)
{
  std::_Exit(99);
}

template <class Operation> void expect_panic(Operation operation)
{
  auto child = fork();
  check(child >= 0, "fork test subprocess");
  if (child == 0)
  {
    operation();
    std::_Exit(0);
  }
  int status;
  check(waitpid(child, &status, 0) == child, "wait test subprocess");
  check(WIFEXITED(status) && WEXITSTATUS(status) == 99,
        "invalid inheritance diagnosed");
}

static void descriptor_contract()
{
  DescriptorTable parent, child;
  file first, second;
  check(parent.install(nullptr) == -1, "null reference rejected");
  check(!parent.get(-1) && !parent.take(DescriptorTable::Capacity),
        "invalid descriptor bounds");
  check(parent.install(&first) == 0, "lowest available descriptor");
  check(parent.install(&second) == 1, "next descriptor");
  check(parent.duplicate(0) == 2 && references(&first) == 2, "dup adds a reference");
  check(parent.close(1) && references(&second) == 0, "close removes owned reference");
  check(!parent.close(1), "double close rejected");
  child.inherit_from(parent);
  check(child.get(0) == &first && !child.get(1) && child.get(2) == &first,
        "fork preserves descriptor numbers and holes");
  check(references(&first) == 4, "fork increments every descriptor reference");
  expect_panic([&] { child.inherit_from(parent); });
  expect_panic([&] { parent.inherit_from(parent); });
  auto *taken = child.take(0);
  check(taken == &first && !child.get(0) && references(&first) == 4,
        "take transfers ownership without dropping the reference");
  test_release_file(taken);
  parent.close_all();
  check(references(&first) == 1 && child.get(2) == &first,
        "child survives parent close");
  child.close_all();
  child.close_all();
  check(references(&first) == 0, "all fork references balanced");

  file shared, rejected;
  for (int i = 0; i < DescriptorTable::Capacity; ++i)
  {
    if (i != 0)
    {
      test_retain_file(&shared);
    }
    check(parent.install(&shared) == i, "fill descriptor table");
  }
  const auto held_references = references(&shared);
  check(parent.duplicate(0) == -1 && references(&shared) == held_references,
        "dup on full table does not leak");
  check(parent.install(&rejected) == -1 && references(&rejected) == 1,
        "failed installation retains caller ownership");
  check(parent.close(3) && parent.install(&rejected) == 3, "reuse the lowest hole");
  parent.close_all();
  check(references(&shared) == 0 && references(&rejected) == 0,
        "capacity references balanced");
}

static void directory_contract()
{
  // These are new objects, even when the compiler reuses a previous fixture's
  // stack addresses. Retired file references must not become inode references.
  for (const auto &entry : reference_counts)
  {
    check(entry.second == 0, "previous fixture released all references");
  }
  reference_counts.clear();
  WorkingDirectory parent, child;
  inode root, replacement;
  check(!parent.get() && !parent.duplicate(), "empty directory");
  parent.replace(&root);
  child.inherit_from(parent);
  check(references(&root) == 2 && child.get() == &root,
        "fork inherits current directory");
  expect_panic([&] { child.inherit_from(parent); });
  transaction = true;
  parent.replace(&replacement);
  check(references(&root) == 1 && references(&replacement) == 1 && child.get() == &root,
        "chdir preserves the child's directory reference");
  auto *owned = child.duplicate();
  check(references(&root) == 2, "path lookup receives its own reference");
  test_release_inode(owned);
  child.clear();
  parent.clear();
  parent.clear();
  transaction = false;
  check(references(&root) == 0 && references(&replacement) == 0,
        "directory references balanced");
}

static void
concurrent_resources_contract() {
  reference_counts.clear();
  DescriptorTable table;
  file first, second;
  // Keep a fixture reference alive while descriptor ownership changes.
  test_retain_file(&first);
  check(table.install(&first) == 0, "concurrent descriptor fixture");
  auto *held = table.retain(0);
  check(held == &first && table.close(0) && references(&first) == 2,
        "retained lookup survives concurrent descriptor close");
  test_release_file(held);
  check(!table.retain(0) && !table.retain(-1), "retained lookup validates slots");
  std::atomic<bool> finished{false};
  std::thread reader([&] {
    while (!finished.load())
    {
      if (auto *reference = table.retain(0))
      {
        test_release_file(reference);
      }
    }
  });
  for (unsigned round = 0; round < 2000; ++round)
  {
    auto *next = round & 1 ? &first : &second;
    test_retain_file(next);
    check(table.install(next) == 0 && table.close(0), "shared descriptor replacement");
  }
  finished.store(true);
  reader.join();
  unsigned long old_generation = 0, new_generation = 0;
  test_retain_file(&first);
  check(table.install(&first, 0, 0, &old_generation) == 0 && table.close(0),
        "rollback fixture initial binding");
  test_retain_file(&first);
  check(table.install(&first, 0, 0, &new_generation) == 0 &&
            new_generation != old_generation,
        "same file rebound to same descriptor has a new generation");
  check(!table.take(0, old_generation) && !table.close(0, old_generation) &&
            table.close(0, new_generation),
        "failed publication cannot close another thread's replacement binding");
  test_release_file(&first);
  test_release_file(&second);
  check(references(&first) == 0 && references(&second) == 0,
        "concurrent descriptor references balance");

  reference_counts.clear();
  WorkingDirectory directory;
  inode a, b;
  transaction = true;
  test_retain_inode(&a);
  directory.replace(&a);
  finished.store(false);
  std::thread lookup([&] {
    while (!finished.load())
    {
      auto *reference = directory.duplicate();
      check(reference, "concurrent cwd remains present");
      test_release_inode(reference);
    }
  });
  for (unsigned round = 0; round < 2000; ++round)
  {
    auto *next = round & 1 ? &a : &b;
    test_retain_inode(next);
    directory.replace(next);
  }
  finished.store(true);
  lookup.join();
  directory.clear();
  test_release_inode(&a);
  test_release_inode(&b);
  check(references(&a) == 0 && references(&b) == 0,
        "concurrent cwd references balance");
  transaction = false;
}

static void credential_contract()
{
  Credentials parent, child;
  unsigned groups[] = {10, 20};
  check(parent.set_gid(100) == 0 && parent.set_groups(groups, 2) == 0 && parent.set_mask(0077) == 0022,
        "root credential setup");
  check(parent.set_uid(1000) == 0, "drop all user identities");
  child.inherit(parent);
  auto snapshot = child.snapshot();
  check(snapshot.uid == 1000 && snapshot.effective_uid == 1000 && snapshot.saved_uid == 1000 &&
        snapshot.gid == 100 && snapshot.group_count == 2 && snapshot.groups[1] == 20 && snapshot.mask == 0077,
        "credential inheritance includes groups and mask");
  check(child.set_uid(0) < 0 && child.set_uid(0, true) < 0 && child.set_gid(0) < 0 && child.set_groups(nullptr, 0) < 0,
        "unprivileged credentials cannot escalate");
  Credentials temporary;
  check(temporary.set_uid(1000, true) == 0 && temporary.snapshot().uid == 0 && temporary.set_uid(0, true) == 0,
        "saved privileged identity can be restored");
  check(temporary.set_uid(65535) < 0 && temporary.set_groups(nullptr, 17) < 0, "credential bounds");
  auto foreign = snapshot; foreign.uid = foreign.effective_uid = foreign.saved_uid = 1001;
  check(!Credentials::may_signal(snapshot, foreign) && Credentials::may_signal(snapshot, snapshot) &&
        Credentials::may_signal(temporary.snapshot(), foreign), "signal identity rules");
}

int main()
{
  credential_contract();
  descriptor_contract();
  directory_contract();
  concurrent_resources_contract();
  std::puts("process resource contracts: passed");
}
