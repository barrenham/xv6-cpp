export module xv.net:ethernet_interface;
export import :interface;
import :arp;
export namespace xv::net
{
class EthernetInterface final : public NetworkInterface, private EthernetProtocol
{
public:
  constexpr EthernetInterface(unsigned id, const char *name, unsigned mtu = 1500)
      : NetworkInterface(id, name, mtu), link_(mtu), arp_(link_, *this) {}
  void initialize(xv::drivers::EthernetDevice &device);
  xv::drivers::EthernetReceiver &receiver() { return link_; }
  const MacAddress *mac() const override { return &link_.mac(); }
  bool carrier() const override { return link_.carrier(); }
  int transmit(Packet &packet, unsigned next_hop) override;
  void tick(unsigned long now) override { arp_.tick(now); }
private:
  void receive(Packet &packet) override;
  EthernetLayer link_;
  ArpCache arp_;
};
}
