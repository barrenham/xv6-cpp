module;
#include "kernel/support/placement_new.hpp"

module xv.process;
import :store;
import :storage_platform;

namespace xv::process
{
Process *ProcessStore::construct(void *storage, int pid, void *owner)
{
  auto *process = new (storage) Process{};
  process->lock.init("process");
  process->lock.acquire();
  process->state = ProcessState::Used;
  process->pid = pid;
  process->storage_node_ = owner;
  return process;
}

void *ProcessStore::storage_node(const Process &process)
{
  return process.storage_node_;
}

bool ProcessStore::has_pid(const Process &process, int pid)
{
  return process.state != ProcessState::Unused && process.pid == pid;
}

void ProcessStore::destroy(Process &process)
{
  process.~Process();
}

void ProcessStore::lock(Process &process)
{
  process.lock.acquire();
}

void ProcessStore::unlock(Process &process)
{
  process.lock.release();
}

void ProcessStore::retire(Process &process)
{
  if (registered(process) || process.membership_.registered() ||
      process.parent || process.first_child_ || process.child_event_mask_ ||
      process.continue_notify_queued_ ||
      process.queued_ || process.scheduling_.queued || process.scheduling_.running ||
      !process.lock.holding() || process.state == ProcessState::Running ||
      process.state == ProcessState::Runnable ||
      process.state == ProcessState::Sleeping || process.state == ProcessState::Stopped)
  {
    detail::storage_fail("process store: retiring a live process");
  }
  process.state = ProcessState::Unused;
  process.lock.release();
}

bool ProcessStore::registered(const Process &process)
{
  return __atomic_load_n(&process.wait_key_, __ATOMIC_ACQUIRE) != nullptr;
}

void ProcessStore::set_stack(Process &process, unsigned long address)
{
  process.kstack = address;
}
} // namespace xv::process
