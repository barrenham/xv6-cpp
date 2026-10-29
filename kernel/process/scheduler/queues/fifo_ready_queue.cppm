module xv.process:fifo_queue;

import :ready_queue;
import xv.sync;

namespace xv::process
{
class FifoReadyQueue : public ReadyQueue
{
public:

  constexpr FifoReadyQueue() = default;

  const char *name() const override
  {
    return "fifo";
  }

  const char *scheduler_name() const override { return "round-robin"; }

  void initialize() override;
  void enqueue(Process &process) override;
  Process *take(CpuState &cpu) override;

private:

  xv::sync::SpinLock lock_;
  Process *head_ = nullptr;
  Process *tail_ = nullptr;
};
} // namespace xv::process
