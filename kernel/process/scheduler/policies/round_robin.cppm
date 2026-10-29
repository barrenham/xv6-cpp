module xv.process:round_robin;

import :fifo_queue;

namespace xv::process
{
// FIFO selection and the inherited one-timer-tick preemption contract.
class RoundRobinPolicy final : public FifoReadyQueue
{
public:
  constexpr RoundRobinPolicy() = default;
  const char *name() const override { return "round-robin"; }
};
} // namespace xv::process
