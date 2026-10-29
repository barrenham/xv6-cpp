export module xv.net:loopback;
export import :interface;
import xv.sync;

export namespace xv::net
{
class LoopbackInterface final : public NetworkInterface
{
public:

  constexpr explicit LoopbackInterface(unsigned id = 1, const char *name = "lo")
      : NetworkInterface(id, name, 1500)
  {
  }

  void initialize();
  bool owns(unsigned value) const override;

  bool accepts_destination(unsigned value,
                           const InterfaceRegistry &registry) const override
  {
    return registry.owns(value);
  }

  int transmit(Packet &packet, unsigned next_hop) override;
  void tick(unsigned long now) override;

private:

  struct TransmitFlow;
  struct DrainFlow;

  xv::sync::SpinLock lock_;

  struct Pending
  {
    void *page = nullptr;
    unsigned length = 0;
    Packet::Context context{};
  } packets_[16]{};

  unsigned head_ = 0, count_ = 0;
};
} // namespace xv::net
