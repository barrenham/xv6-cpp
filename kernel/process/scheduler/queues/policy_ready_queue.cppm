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
    xv::sync::ScopedLock guard(lock_);
    switch (policy_.kind())
    {
    case xv::scheduling::Kind::Cfs: return "cfs";
    case xv::scheduling::Kind::RoundRobin: return "round-robin";
    case xv::scheduling::Kind::Feedback: return "mlfq";
    }
    return "unknown";
  }
  xv::scheduling::Kind policy_kind() const override
  {
    xv::sync::ScopedLock guard(lock_);
    return policy_.kind();
  }
  bool change_policy(xv::scheduling::Kind kind) override;
  void policy_snapshot(unsigned &, unsigned long &, unsigned long &, unsigned long &) override;
  void initialize() override;
  void enqueue(Process &process) override;
  Process *take(CpuState &cpu) override;
  void start(Process &process) override;
  void stop(Process &process) override;
  bool tick(Process &process) override;
private:
  mutable xv::sync::SpinLock lock_;
  xv::scheduling::RunQueue policy_;
};
} // namespace xv::process
