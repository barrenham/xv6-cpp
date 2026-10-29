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
  int transmit(const char *data, int count, bool nonblocking = false) override;
  void put_sync(int character) override;
  void drain() override;
  void handle_interrupt() override;
  bool writable(xv::sync::PollTable *table = nullptr) override;

private:

  struct TransmitFlow;
  struct DrainFlow;

  int get();
  void pump();
  static constexpr unsigned TxCapacity = 128;
  char tx_data_[TxCapacity]{};
  unsigned tx_read_ = 0;
  unsigned tx_written_ = 0;
  xv::sync::SpinLock tx_state_;
  detail::MmioRegion registers_;
  unsigned irq_;
  InputReceiver *receiver_ = nullptr;
  xv::sync::SleepLock tx_lock_;
  int tx_channel_ = 0;
};
} // namespace xv::drivers
