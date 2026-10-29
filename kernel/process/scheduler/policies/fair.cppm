module xv.process:fair_policy;

import :scheduler;
import :policy_queue;

namespace xv::process
{
// Fixed policy identity; queue exclusion and accounting live in the shared
// PolicyReadyQueue adapter and the architecture-independent RunQueue model.
class FairPolicy final : public PolicyReadyQueue
{
public:
  constexpr FairPolicy() : PolicyReadyQueue(SchedulerKind::Fair) {}
};
} // namespace xv::process
