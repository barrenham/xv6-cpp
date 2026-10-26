module;

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <vector>
#include "native_wait.h"

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
thread_local bool require_pinned_cpu = false;
std::atomic<unsigned> pinned_cpu_lookups{0};
std::atomic<bool> interruption{false};
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
  if (require_pinned_cpu)
  {
    if (interrupt_depth == 0) fail("CPU lookup with interrupts enabled");
    pinned_cpu_lookups.fetch_add(1);
  }
  return &interrupt_depth;
}

int current_pid()
{
  static thread_local int pid = next_pid.fetch_add(1);
  return pid;
}

bool interrupted()
{
  return interruption.load();
}

void prepare_interruptible_sleep(void *channel)
{
  prepare_sleep(channel);
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

// Shared scheduler emulation for native filesystem wait/wakeup paths.
extern "C" void xv_test_prepare_sleep(void *channel)
{
  xv::sync::detail::prepare_sleep(channel);
}

extern "C" void xv_test_sleep()
{
  xv::sync::detail::sleep_process();
}

extern "C" void xv_test_wakeup(void *channel)
{
  xv::sync::detail::wake_processes(channel);
}

extern "C" void xv_test_set_interrupted(bool value)
{
  xv::sync::detail::interruption.store(value);
}

namespace xv::sync
{
void push_interrupts_off() { detail::push_interrupts_off(); }
void pop_interrupts_off() { detail::pop_interrupts_off(); }
}

extern "C" int xv_test_interrupt_depth()
{ return xv::sync::detail::interrupt_depth; }
extern "C" void xv_test_require_pinned_cpu(bool enabled)
{ xv::sync::detail::require_pinned_cpu = enabled; }
extern "C" unsigned xv_test_pinned_cpu_lookups()
{ return xv::sync::detail::pinned_cpu_lookups.load(); }
