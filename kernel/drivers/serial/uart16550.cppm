module xv.drivers:uart;

import :serial;
import :mmio;
import xv.sync;

namespace xv::drivers
{
class Uart16550 final : public SerialPort
{
public:

  constexpr Uart16550(unsigned long base, unsigned irq) : registers_(base), irq_(irq)
  {
  }

  const char *name() const override
  {
    return "16550a UART";
  }

  unsigned irq() const override
  {
    return irq_;
  }

  void initialize() override;
  void attach(InputReceiver &receiver) override;
  void write(const char *data, int count) override;
  void put_sync(int character) override;
  void handle_interrupt() override;

private:

  int get();
  detail::MmioRegion registers_;
  unsigned irq_;
  InputReceiver *receiver_ = nullptr;
  xv::sync::SleepLock tx_lock_;
  int tx_channel_ = 0;
};
} // namespace xv::drivers
