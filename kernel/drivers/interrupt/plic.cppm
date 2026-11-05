module xv.drivers:plic;

import :interrupt;
import :mmio;

namespace xv::drivers
{
class Plic final : public InterruptController
{
public:

  constexpr explicit Plic(unsigned long base) : registers_(base)
  {
  }

  const char *name() const override
  {
    return "RISC-V PLIC";
  }

  void configure(unsigned long base, const unsigned *contexts, unsigned count) {
    registers_ = detail::MmioRegion(base); contexts_ = contexts; context_count_ = count;
  }
  void initialize() override;
  void set_priority(unsigned irq, unsigned priority) override;
  void initialize_hart(int hart) override;
  void enable(int hart, unsigned irq) override;
  unsigned claim(int hart) override;
  void complete(int hart, unsigned irq) override;

private:

  unsigned context(int hart) const;
  detail::MmioRegion registers_;
  const unsigned *contexts_ = nullptr;
  unsigned context_count_ = 0;
};
} // namespace xv::drivers
