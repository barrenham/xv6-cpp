module;
#include "kernel/process/scheduler/policies/scheduling_policy.h"
module xv.process;
import :policy_queue;
import :storage_platform;

namespace xv::process
{
void PolicyReadyQueue::initialize() { lock_.init("policy ready queue"); }

void PolicyReadyQueue::enqueue(Process &process)
{
  if (!process_locked(process)) detail::storage_fail("policy queue: process not locked");
  xv::sync::ScopedLock guard(lock_);
  if (queued(process) || !policy_.enqueue(entity(process), detail::scheduling_time()))
    detail::storage_fail("policy queue: invalid enqueue");
  queued(process) = true;
}

Process *PolicyReadyQueue::take(CpuState &cpu)
{
  xv::sync::ScopedLock guard(lock_);
  auto *entry = policy_.take(detail::scheduling_time(), [&cpu](const xv::scheduling::Entity &entry) {
    return eligible(*static_cast<Process *>(entry.owner), cpu);
  });
  if (!entry) return nullptr;
  auto *process = static_cast<Process *>(entry->owner);
  queued(*process) = false;
  return process;
}

void PolicyReadyQueue::start(Process &process)
{
  if (!process_locked(process)) detail::storage_fail("policy queue: start not locked");
  xv::sync::ScopedLock guard(lock_);
  if (!policy_.start(entity(process), detail::scheduling_time()))
    detail::storage_fail("policy queue: duplicate dispatch");
}

void PolicyReadyQueue::stop(Process &process)
{
  if (!process_locked(process)) detail::storage_fail("policy queue: stop not locked");
  xv::sync::ScopedLock guard(lock_);
  policy_.stop(entity(process), detail::scheduling_time());
}

bool PolicyReadyQueue::tick(Process &process)
{
  xv::sync::ScopedLock guard(lock_);
  return policy_.tick(entity(process), detail::scheduling_time());
}
} // namespace xv::process
