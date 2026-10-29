module;
#include "kernel/process/scheduler/policies/scheduling_policy.h"
module xv.process:policy_queue;
import :ready_queue;
import xv.sync;

namespace xv::process
{
class PolicyReadyQueue : public ReadyQueue
{
public:
  constexpr explicit PolicyReadyQueue(xv::scheduling::Kind kind) : policy_(kind) {}
  const char *name() const override
  {
    switch (policy_.kind())
    {
    case xv::scheduling::Kind::Fair: return "fair";
    case xv::scheduling::Kind::RoundRobin: return "round-robin";
    case xv::scheduling::Kind::Feedback: return "mlfq";
    }
    return "unknown";
  }
  void initialize() override;
  void enqueue(Process &process) override;
  Process *take(CpuState &cpu) override;
  void start(Process &process) override;
  void stop(Process &process) override;
  bool tick(Process &process) override;
private:
  xv::sync::SpinLock lock_;
  xv::scheduling::RunQueue policy_;
};
} // namespace xv::process
