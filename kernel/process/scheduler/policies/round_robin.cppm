module xv.process:round_robin;

import :scheduler;
import :ready_queue;

namespace xv::process
{
class RoundRobinScheduler final : public Scheduler
{
public:

  constexpr explicit RoundRobinScheduler(ReadyQueue &ready) : ready_(ready)
  {
  }

  const char *name() const override
  {
    return "round-robin";
  }

  [[noreturn]] void run() override;
  void suspend() override;
  void yield() override;
  void make_runnable(Process &process) override;

private:

  ReadyQueue &ready_;
};
} // namespace xv::process
