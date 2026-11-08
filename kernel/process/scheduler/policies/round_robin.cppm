module;
#include "kernel/process/scheduler/policies/scheduling_policy.h"
module xv.process:round_robin;

import :scheduler;
import :policy_queue;

namespace xv::process
{
// FIFO dispatch; the queue adapter preserves preemption on every timer tick.
class RoundRobinPolicy final : public PolicyReadyQueue
{
public:
  constexpr RoundRobinPolicy() : PolicyReadyQueue(SchedulerKind::RoundRobin) {}
};
} // namespace xv::process
