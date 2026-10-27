#include "tests/process_test_hooks.h"
#include <cstdio>
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
static_assert(std::is_trivially_destructible_v<Process>);
constinit DescriptorTable boot_table;
constinit WorkingDirectory boot_directory;
static bool transaction = false;

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
  check(reference && references(reference) > 0, "duplicate live file reference");
  ++references(reference);
  return reference;
}

extern "C" void test_release_file(void *reference)
{
  check(reference && references(reference) > 0, "close owned file reference");
  --references(reference);
}

extern "C" void *test_retain_inode(void *reference)
{
  check(reference && references(reference) > 0, "duplicate live inode reference");
  ++references(reference);
  return reference;
}

extern "C" void test_release_inode(void *reference)
{
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
  std::puts("process resource contracts: passed");
}
