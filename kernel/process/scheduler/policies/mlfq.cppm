module;
#include "kernel/process/scheduler/policies/scheduling_policy.h"
module xv.process:mlfq_policy;

import :scheduler;
import :policy_queue;

namespace xv::process
{
// Initial three-level feedback policy; shared accounting preserves CPU allotments
// across yields, and the model performs periodic priority boosting.
class MlfqPolicy final : public PolicyReadyQueue
{
public:
  constexpr MlfqPolicy() : PolicyReadyQueue(SchedulerKind::Feedback) {}
};
} // namespace xv::process
