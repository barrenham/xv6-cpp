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

  constexpr E1000(unsigned long base, unsigned irq)
      : registers_(base), base_(base), irq_(irq)
  {
  }

  const char *name() const override
  {
    return "Intel 82540EM Ethernet";
  }

  unsigned irq() const override
  {
    return irq_;
  }

  void initialize() override;

  void locate(unsigned slot)
  {
    slot_ = slot;
    irq_ = 32 + slot % 4;
  }

  void mac_address(unsigned char *destination) const override
  {
    for (unsigned i = 0; i < 6; ++i)
    {
      destination[i] = mac_[i];
    }
  }

  void attach(EthernetReceiver &receiver)
  {
    receiver_ = &receiver;
  }

  bool online() const override
  {
    return ready_;
  }

  bool carrier() const override
  {
    return ready_ && (registers_.read32(8) & 2);
  }

  bool transmit(void *page, unsigned length) override;
  bool promiscuous(bool enable) override;
  void handle_interrupt() override;

private:

  struct InitializeFlow;
  struct TransmitFlow;
  struct InterruptFlow;
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
  unsigned long base_;
  unsigned irq_;
  unsigned slot_ = 32;
  unsigned char mac_[6]{};
  xv::sync::SpinLock lock_;
  alignas(128) TxDescriptor tx_[Count]{};
  alignas(128) RxDescriptor rx_[Count]{};
  void *transmitted_[Count]{};
  void *received_[Count]{};
  unsigned tx_tail_ = 0, rx_next_ = 0;
  EthernetReceiver *receiver_ = nullptr;
  bool ready_ = false;
};
} // namespace xv::drivers
