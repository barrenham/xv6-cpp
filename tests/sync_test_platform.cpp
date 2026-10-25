module;

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <vector>

module xv.sync;

import :platform;

namespace xv::sync::detail
{
namespace
{
struct Waiter
{
  void *channel = nullptr;
  std::condition_variable changed;
};

std::mutex wait_mutex;
std::vector<Waiter *> waiters;
std::atomic<int> next_pid{1};
thread_local int interrupt_depth = 0;
thread_local Waiter waiter;
} // namespace

void push_interrupts_off()
{
  ++interrupt_depth;
}

void pop_interrupts_off()
{
  if (interrupt_depth <= 0)
  {
    fail("unbalanced interrupts");
  }
  --interrupt_depth;
}

void *current_cpu()
{
  return &interrupt_depth;
}

int current_pid()
{
  static thread_local int pid = next_pid.fetch_add(1);
  return pid;
}

void prepare_sleep(void *channel)
{
  std::lock_guard guard(wait_mutex);
  if (!channel || waiter.channel)
  {
    fail("invalid sleep preparation");
  }
  waiter.channel = channel;
  waiters.push_back(&waiter);
}

void sleep_process()
{
  if (interrupt_depth != 0)
  {
    fail("sleep while holding a spinlock");
  }
  std::unique_lock guard(wait_mutex);
  waiter.changed.wait(guard, [] { return waiter.channel == nullptr; });
  for (auto it = waiters.begin(); it != waiters.end(); ++it)
  {
    if (*it == &waiter)
    {
      waiters.erase(it);
      break;
    }
  }
}

void wake_processes(void *channel)
{
  std::lock_guard guard(wait_mutex);
  for (auto *waiting : waiters)
  {
    if (waiting->channel == channel)
    {
      waiting->channel = nullptr;
      waiting->changed.notify_one();
    }
  }
}

[[noreturn]] void fail(const char *)
{
  std::_Exit(99);
}
} // namespace xv::sync::detail
