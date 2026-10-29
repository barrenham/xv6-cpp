module;
#include "kernel/process/scheduler/policies/scheduling_policy.h"
module xv.process;
import :ready_queue;

namespace xv::process
{
bool ReadyQueue::process_locked(const Process &process)
{
  return process.lock.holding();
}

bool ReadyQueue::eligible(const Process &process, const CpuState &cpu)
{
  auto *affinity = __atomic_load_n(&process.pincpu, __ATOMIC_ACQUIRE);
  return !affinity || affinity == &cpu;
}

Process *&ReadyQueue::next(Process &process)
{
  return process.ready_next_;
}

bool &ReadyQueue::queued(Process &process)
{
  return process.queued_;
}
xv::scheduling::Entity &ReadyQueue::entity(Process &process)
{
  process.scheduling_.owner = &process;
  return process.scheduling_;
}

} // namespace xv::process
