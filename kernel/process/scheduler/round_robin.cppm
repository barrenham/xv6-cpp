module xv.process:round_robin;

import :scheduler;
import :table;

namespace xv::process
{
class RoundRobinScheduler final : public Scheduler
{
public:

  constexpr explicit RoundRobinScheduler(ProcessTable &table) : table_(table)
  {
  }

  const char *name() const override
  {
    return "round-robin";
  }

  [[noreturn]] void run() override;
  void suspend() override;
  void yield() override;
  void prepare_sleep(void *channel);
  void sleep();
  void first_return();

private:

  ProcessTable &table_;
};
} // namespace xv::process
