module;
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
#include "tests/process_storage_test_hooks.h"

module xv.process;
import :store;
import :cpu;
import :paged_store;
import :fifo_queue;
import :wait_channels;

namespace xv::process::test
{
// Backend lock primitives are protected so alternative storage implementations
// can use the exact same process lifetime protocol without exposing Process data.
class Access : public ProcessStore
{
public:

  using ProcessStore::lock;
  using ProcessStore::registered;
  using ProcessStore::unlock;
};

void check(bool condition, const char *message)
{
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}

class ByAddress final : public ProcessPredicate
{
public:

  explicit ByAddress(Process *expected) : expected_(expected)
  {
  }

  bool matches(Process &process) override
  {
    return &process == expected_;
  }

private:

  Process *expected_;
};

class Count final : public ProcessVisitor
{
public:

  void visit(Process &process) override
  {
    check(process.kernel_stack() != 0, "published descriptor has a stack");
    ++count;
  }

  unsigned count = 0;
};

void basic()
{
  PagedProcessStore store;
  FifoReadyQueue ready;
  store.initialize();
  ready.initialize();
  std::vector<Process *> processes;
  for (int i = 0; i < 192; ++i)
  {
    auto *process = store.allocate();
    check(process, "registry grows beyond 64 descriptors");
    processes.push_back(process);
    ready.enqueue(*process);
    Access::unlock(*process);
  }
  check(store.count() == processes.size(), "dynamic live descriptor count");
  Count count;
  store.visit(count);
  check(count.count == processes.size(), "visitor covers the dynamic registry");
  for (auto *process : processes)
  {
    ByAddress key(process);
    auto *found = store.find(key);
    check(found == process, "addresses remain stable across growth");
    Access::unlock(*found);
    check(ready.take(*cpu_at(0)) == process,
          "FIFO order independent of registry order");
  }
  check(ready.take(*cpu_at(0)) == nullptr, "empty queue");
  for (auto *process : processes)
  {
    Access::lock(*process);
    store.release(process);
  }
  check(store.count() == 0 && xv_storage_live_pages() == 0,
        "descriptors and stacks are fully reclaimed");

  for (int budget = 0; budget <= 1; ++budget)
  {
    xv_storage_fail_after(budget);
    check(!store.allocate(), "descriptor/stack allocation failure is recoverable");
    check(store.count() == 0 && xv_storage_live_pages() == 0,
          "failed growth leaves no partial node or leaked stack");
  }
  xv_storage_fail_after(-1);
  auto *process = store.allocate();
  check(process != nullptr, "growth recovers after allocation failure");
  store.release(process);
}

void affinity()
{
  PagedProcessStore store;
  FifoReadyQueue ready;
  store.initialize();
  ready.initialize();
  auto *first = store.allocate();
  Access::unlock(*first);
  check(first->pin_cpu(-1) == -1 && first->pin_cpu(0) == 0, "affinity validation");
  Access::lock(*first);
  ready.enqueue(*first);
  Access::unlock(*first);
  auto *second = store.allocate();
  ready.enqueue(*second);
  Access::unlock(*second);
  check(ready.take(*cpu_at(1)) == second,
        "ineligible head does not block another hart");
  check(ready.take(*cpu_at(1)) == nullptr, "pinned process stays queued");
  check(ready.take(*cpu_at(0)) == first, "eligible hart claims pinned process");
  Access::lock(*first);
  store.release(first);
  Access::lock(*second);
  store.release(second);
}

void duplicate()
{
  auto child = fork();
  check(child >= 0, "fork native invariant check");
  if (child == 0)
  {
    PagedProcessStore store;
    FifoReadyQueue ready;
    store.initialize();
    ready.initialize();
    auto *process = store.allocate();
    ready.enqueue(*process);
    ready.enqueue(*process);
    std::_Exit(0);
  }
  int status = 0;
  check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
            WEXITSTATUS(status) == 99,
        "duplicate queue publication rejected");
}

class UnusedScheduler final : public Scheduler
{
public:

  const char *name() const override
  {
    return "unused native scheduler";
  }

  [[noreturn]] void run() override
  {
    std::_Exit(98);
  }

  void suspend() override
  {
    std::_Exit(98);
  }

  void yield() override
  {
    std::_Exit(98);
  }

  void make_runnable(Process &) override
  {
    std::_Exit(98);
  }
};

void channels()
{
  PagedProcessStore store;
  store.initialize();
  UnusedScheduler scheduler;
  WaitChannels waits(scheduler);
  waits.initialize();
  int first, second;
  for (int i = 0; i < 500; ++i)
  {
    auto *process = store.allocate();
    Access::unlock(*process);
    waits.prepare(*process, &first);
    waits.wake(&second); // an unrelated channel cannot remove this membership
    check(Access::registered(*process), "unrelated event preserves registration");
    waits.prepare(*process, &second); // replaces and unlinks the previous wait
    waits.wake(&first);
    check(Access::registered(*process), "replaced channel ignores stale event");
    if (i % 2)
    {
      waits.cancel(*process);
    }
    else
    {
      waits.wake(&second); // notify before a process actually sleeps
    }
    Access::lock(*process);
    store.release(process); // rejects dangling memberships
  }
  check(xv_storage_live_pages() == 0,
        "channel replacement/early wake/cancel reclaim safely");
}

void interrupted_channels()
{
  PagedProcessStore store;
  store.initialize();
  UnusedScheduler scheduler;
  WaitChannels waits(scheduler);
  waits.initialize();
  int event;
  auto *process = store.allocate();
  Access::unlock(*process);
  process->request_kill();
  waits.prepare(*process, &event, true);
  check(!Access::registered(*process),
        "fatal notification before prepare cannot sleep");
  check(process->is_killed() && process->termination_status() == -1,
        "legacy termination predicate preserved");
  waits.prepare(*process, &event);
  check(Access::registered(*process), "internal waits remain uninterruptible");
  waits.cancel(*process);
  Access::lock(*process);
  store.release(process);
  check(xv_storage_live_pages() == 0, "interrupted waits release all pages");
}

void concurrent_channels()
{
  PagedProcessStore store;
  store.initialize();
  UnusedScheduler scheduler;
  WaitChannels waits(scheduler);
  waits.initialize();
  int event;
  std::atomic<bool> stop{false};
  std::thread notifier(
      [&]
      {
        while (!stop.load())
        {
          waits.wake(&event);
          std::this_thread::yield();
        }
      });
  std::vector<std::thread> owners;
  for (int i = 0; i < 4; ++i)
  {
    owners.emplace_back(
        [&]
        {
          for (int n = 0; n < 200; ++n)
          {
            auto *process = store.allocate();
            Access::unlock(*process);
            waits.prepare(*process, &event);
            waits.cancel(*process);
            check(!Access::registered(*process), "cancel completes before reclamation");
            Access::lock(*process);
            store.release(process);
          }
        });
  }
  for (auto &owner : owners)
  {
    owner.join();
  }
  stop.store(true);
  notifier.join();
  check(store.count() == 0 && xv_storage_live_pages() == 0,
        "concurrent notifications cannot retain or dereference retired waiters");
}

void concurrent()
{
  PagedProcessStore store;
  FifoReadyQueue ready;
  store.initialize();
  ready.initialize();
  constexpr int Producers = 4, PerProducer = 200, Total = Producers * PerProducer;
  std::atomic<int> consumed{0};
  std::atomic<bool> stop{false};
  std::thread observer(
      [&]
      {
        while (!stop.load())
        {
          Count count;
          store.visit(count);
          std::this_thread::yield();
        }
      });
  std::vector<std::thread> workers;
  for (int i = 0; i < Producers; ++i)
  {
    workers.emplace_back(
        [&]
        {
          for (int n = 0; n < PerProducer; ++n)
          {
            auto *process = store.allocate();
            check(process != nullptr, "concurrent growth");
            ready.enqueue(*process);
            Access::unlock(*process);
          }
        });
  }
  for (int i = 0; i < 2; ++i)
  {
    workers.emplace_back(
        [&, i]
        {
          while (consumed.load() < Total)
          {
            if (auto *process = ready.take(*cpu_at(i)))
            {
              Access::lock(*process);
              store.release(process);
              ++consumed;
            }
            else
            {
              std::this_thread::yield();
            }
          }
        });
  }
  for (auto &worker : workers)
  {
    worker.join();
  }
  stop.store(true);
  observer.join();
  check(consumed == Total && store.count() == 0 && xv_storage_live_pages() == 0,
        "concurrent traversal/queue claims/removal preserve ownership");
}
} // namespace xv::process::test

extern "C" int run_process_storage_contracts()
{
  xv::process::test::basic();
  xv::process::test::affinity();
  xv::process::test::duplicate();
  xv::process::test::channels();
  xv::process::test::interrupted_channels();
  xv::process::test::concurrent_channels();
  xv::process::test::concurrent();
  std::puts("process storage and ready queue contracts: passed");
  return 0;
}
