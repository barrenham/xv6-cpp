module xv.drivers:e1000;
import :network;
import :interrupt;
import :mmio;
import xv.sync;
namespace xv::drivers
{
class E1000 final : public InterruptDevice, public EthernetDevice
{
public:
  constexpr E1000(unsigned long base, unsigned irq) : registers_(base), irq_(irq) {}
  const char *name() const override { return "Intel 82540EM Ethernet"; }
  unsigned irq() const override { return irq_; }
  void initialize() override;
  void attach(EthernetReceiver &receiver) { receiver_ = &receiver; }
  bool online() const override { return ready_; }
  bool transmit(void *page, unsigned length) override;
  void handle_interrupt() override;
private:
  void reclaim();
  struct TxDescriptor
  {
    unsigned long address;
    unsigned short length;
    unsigned char checksum_offset, command, status, checksum_start;
    unsigned short special;
  };
  struct RxDescriptor
  {
    unsigned long address;
    unsigned short length, checksum;
    unsigned char status, errors;
    unsigned short special;
  };
  static_assert(sizeof(TxDescriptor) == 16 && sizeof(RxDescriptor) == 16);
  static constexpr unsigned Count = 16;
  detail::MmioRegion registers_;
  unsigned irq_;
  xv::sync::SpinLock lock_;
  alignas(128) TxDescriptor tx_[Count]{};
  alignas(128) RxDescriptor rx_[Count]{};
  void *transmitted_[Count]{};
  void *received_[Count]{};
  unsigned tx_tail_ = 0, rx_next_ = 0;
  EthernetReceiver *receiver_ = nullptr;
  bool ready_ = false;
};
}
