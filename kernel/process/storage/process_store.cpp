module;
#include "kernel/support/placement_new.hpp"

module xv.process;
import :store;
import :storage_platform;

namespace xv::process
{
Process *ProcessStore::construct(void *storage)
{
  auto *process = new (storage) Process{};
  process->lock.init("process");
  process->lock.acquire();
  process->state = ProcessState::Used;
  return process;
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
  if (registered(process) || !process.lock.holding() ||
      process.state == ProcessState::Running ||
      process.state == ProcessState::Runnable ||
      process.state == ProcessState::Sleeping)
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
