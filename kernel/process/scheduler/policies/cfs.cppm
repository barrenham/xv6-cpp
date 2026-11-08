module;
#include "kernel/process/scheduler/policies/scheduling_policy.h"
module xv.process:cfs_policy;

import :scheduler;
import :policy_queue;

namespace xv::process
{
// Initial boot policy; live changes, exclusion and accounting use the shared
// PolicyReadyQueue adapter and the architecture-independent RunQueue model.
class CfsPolicy final : public PolicyReadyQueue
{
public:
  constexpr CfsPolicy() : PolicyReadyQueue(SchedulerKind::Cfs) {}
};
} // namespace xv::process
