module;
#include "kernel/io/posix_errno.h"
module xv.process;
import :manager;
import xv.arch;

namespace xv::process
{
namespace
{
struct Bucket
{
  xv::sync::SpinLock lock;
  Process *head = nullptr;
};

constinit Bucket buckets[64];

Bucket &
bucket(const void *group, unsigned long address) {
  auto identity = reinterpret_cast<unsigned long>(group);
  return buckets[((identity >> 12) ^ (address >> 2) ^ (address >> 12)) % 64];
}
} // namespace

int
ProcessManager::wait_word(unsigned long address, unsigned expected,
                          unsigned long deadline) {
  auto &thread = *current();
  unsigned value = 0;
  // Fault before taking the queue lock. Revalidate atomically under that lock
  // before publishing the wait, so a concurrent unlock cannot be lost.
  if (!thread.memory().load_word(address, value))
  {
    return -EFAULT;
  }
  auto waiter = make_poll_wait();
  auto &queue = bucket(thread.resources_, address);
  queue.lock.acquire();
  if (!thread.memory().load_word(address, value))
  {
    queue.lock.release();
    return -EFAULT;
  }
  if (value != expected)
  {
    queue.lock.release();
    return -EAGAIN;
  }
  if (thread.is_killed())
  {
    queue.lock.release();
    return -EINTR;
  }
  if (deadline && xv::arch::r_time() / 10000 >= deadline)
  {
    queue.lock.release();
    return -ETIMEDOUT;
  }
  waiter.watch(&thread.word_wait_address_);
  if (deadline)
  {
    waiter.set_deadline(deadline);
  }
  if (waiter.failed())
  {
    queue.lock.release();
    return -ENOMEM;
  }
  thread.word_waiting_ = true;
  thread.word_wait_address_ = address;
  thread.word_woken_ = false;
  thread.word_wait_next_ = queue.head;
  queue.head = &thread;
  queue.lock.release();
  waiter.sleep();
  queue.lock.acquire();
  auto **link = &queue.head;
  while (*link && *link != &thread)
  {
    link = &(*link)->word_wait_next_;
  }
  if (*link)
  {
    *link = thread.word_wait_next_;
  }
  thread.word_wait_next_ = nullptr;
  thread.word_waiting_ = false;
  thread.word_wait_address_ = 0;
  bool woken = thread.word_woken_;
  thread.word_woken_ = false;
  queue.lock.release();
  if (thread.is_killed())
  {
    return -EINTR;
  }
  return !woken && deadline && xv::arch::r_time() / 10000 >= deadline ? -ETIMEDOUT : 0;
}

int
ProcessManager::wake_word(unsigned long address, unsigned count) {
  if (address % sizeof(unsigned))
  {
    return -EINVAL;
  }
  auto &caller = *current();
  auto &queue = bucket(caller.resources_, address);
  xv::sync::ScopedLock guard(queue.lock);
  unsigned woken = 0;
  for (auto *thread = queue.head; thread && woken < count;
       thread = thread->word_wait_next_)
  {
    if (thread->resources_ == caller.resources_ &&
        thread->word_wait_address_ == address && !thread->word_woken_)
    {
      thread->word_woken_ = true;
      wakeup(&thread->word_wait_address_);
      ++woken;
    }
  }
  return woken;
}
} // namespace xv::process
