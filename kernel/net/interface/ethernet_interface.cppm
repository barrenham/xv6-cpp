export module xv.net:ethernet_interface;
export import :interface;
import :arp;

export namespace xv::net
{
class EthernetInterface : public NetworkInterface, private EthernetProtocol
{
public:

  constexpr EthernetInterface(unsigned id, const char *name, unsigned mtu = 1500)
      : NetworkInterface(id, name, mtu), link_(mtu), arp_(link_, *this)
  {
  }

  void initialize(xv::drivers::EthernetDevice &device);

  xv::drivers::EthernetReceiver &receiver()
  {
    return link_;
  }

  const MacAddress *mac() const override
  {
    return &link_.mac();
  }

  bool carrier() const override
  {
    return link_.carrier();
  }

  Layer2Port *layer2() override
  {
    return &link_;
  }

  unsigned master() const override
  {
    return link_.master();
  }

  int transmit_bootstrap(Packet &packet) override;
  int transmit(Packet &packet, unsigned next_hop) override;

  bool neighbor(unsigned index, NeighborInfo &result) override
  {
    return arp_.neighbor(index, result);
  }

  void invalidate() override
  {
    arp_.clear();
  }

  void tick(unsigned long now) override
  {
    arp_.tick(now);
  }

private:

  struct TransmitFlow;

  void receive(Packet &packet) override;
  EthernetLayer link_;
  ArpCache arp_;
};
} // namespace xv::net
