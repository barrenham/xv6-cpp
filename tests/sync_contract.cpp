#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <thread>
#include <type_traits>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>

#include "native_wait.h"

import xv.sync;

using namespace xv::sync;

static_assert(std::is_abstract_v<Lock>);
static_assert(std::is_base_of_v<Lock, SpinLock>);
static_assert(std::is_base_of_v<Lock, SleepLock>);
static_assert(!std::is_copy_constructible_v<SpinLock>);
static_assert(!std::is_move_constructible_v<SleepLock>);
static_assert(!std::is_copy_constructible_v<ScopedLock>);
static_assert(!std::is_destructible_v<Lock>);

// Compile-time enforcement of the kernel's no-global-constructors contract.
constinit SpinLock static_spin;
constinit SleepLock static_sleep;

static void check(bool condition, const char *message)
{
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}

template <class Operation> void expect_panic(Operation operation)
{
  const auto child = fork();
  check(child >= 0, "fork");
  if (child == 0)
  {
    operation();
    std::_Exit(0);
  }
  int status;
  check(waitpid(child, &status, 0) == child, "waitpid");
  check(WIFEXITED(status) && WEXITSTATUS(status) == 99, "ownership violation");
}

static void early_return(Lock &lock)
{
  ScopedLock guard(lock);
  check(lock.holding(), "guard acquired via abstract interface");
  return;
}

static void contract(Lock &lock, LockKind kind)
{
  lock.init("contract");
  check(lock.kind() == kind, "explicit lock kind");
  check(std::strcmp(lock.name(), "contract") == 0, "lock name");
  check(!lock.holding(), "initially unlocked");
  early_return(lock);
  check(!lock.holding(), "guard releases on early return");

  lock.acquire();
  std::thread observer([&]
                       { check(!lock.holding(), "another thread is not the owner"); });
  observer.join();
  lock.release();

  // This counter has no atomics: every access depends on the actual lock.
  int counter = 0;
  std::vector<std::thread> workers;
  for (int i = 0; i < 6; ++i)
  {
    workers.emplace_back(
        [&]
        {
          for (int n = 0; n < 4000; ++n)
          {
            ScopedLock guard(lock);
            check(lock.holding(), "contending owner");
            ++counter;
          }
        });
  }
  for (auto &worker : workers)
  {
    worker.join();
  }
  check(counter == 24000, "mutual exclusion under contention");

  expect_panic([&] { lock.release(); });
  expect_panic(
      [&]
      {
        lock.acquire();
        lock.acquire();
      });
  expect_panic(
      [&]
      {
        lock.acquire();
        std::thread wrong_owner([&] { lock.release(); });
        wrong_owner.join();
      });
}

int main()
{
  contract(static_spin, LockKind::Spin);
  contract(static_sleep, LockKind::Sleep);

  xv_test_set_interrupted(true);
  check(!static_sleep.acquire_interruptible(),
        "pending signal cancels lock acquisition");
  xv_test_set_interrupted(false);
  check(static_sleep.acquire_interruptible(), "uninterrupted lock acquisition");
  {
    ScopedLock guard(static_sleep, adopt_lock);
    check(static_sleep.holding(), "adopted guard retains ownership");
  }
  check(!static_sleep.holding(), "adopted guard releases ownership");

  // A page-allocated containing object must be placement-constructed, as pipes
  // are in the kernel. Exercise virtual dispatch on non-zero raw storage.
  struct PipeState
  {
    SpinLock lock;
  };

  alignas(PipeState) unsigned char storage[sizeof(PipeState)];
  std::memset(storage, 5, sizeof(storage));
  auto *pipe = new (storage) PipeState;
  contract(pipe->lock, LockKind::Spin);
  pipe->~PipeState();
  std::puts("sync contracts: passed");
}
