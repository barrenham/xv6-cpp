module xv.process:scheduler_runtime;

import :scheduler;
import :ready_queue;

namespace xv::process
{
class SchedulerRuntime final : public Scheduler
{
public:

  constexpr explicit SchedulerRuntime(ReadyQueue &ready) : ready_(&ready)
  {
  }

  const char *name() const override
  {
    return ready_->scheduler_name();
  }

  SchedulerKind kind() const { return ready_->policy_kind(); }
  bool change_policy(SchedulerKind kind) { return ready_->change_policy(kind); }
  void snapshot(unsigned &kind, unsigned long &changes, unsigned long &waiting,
                unsigned long &running)
  {
    ready_->policy_snapshot(kind, changes, waiting, running);
  }

  // Boot-only binding; service initialize(kind) guards publication and reuse.
  void select(ReadyQueue &ready) { ready_ = &ready; }

  [[noreturn]] void run() override;
  void suspend() override;
  void yield() override;
  void make_runnable(Process &process) override;
  bool timer_tick() override;

private:

  struct RunFlow;

  ReadyQueue *ready_;
};
} // namespace xv::process
