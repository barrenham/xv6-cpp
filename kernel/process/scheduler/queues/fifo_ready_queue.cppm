module xv.process:fifo_queue;

import :ready_queue;
import xv.sync;

namespace xv::process
{
class FifoReadyQueue final : public ReadyQueue
{
public:

  constexpr FifoReadyQueue() = default;

  const char *name() const override
  {
    return "fifo";
  }

  void initialize() override;
  void enqueue(Process &process) override;
  Process *take(CpuState &cpu) override;

private:

  xv::sync::SpinLock lock_;
  Process *head_ = nullptr;
  Process *tail_ = nullptr;
};
} // namespace xv::process
