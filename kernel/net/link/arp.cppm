export module xv.net:arp;
import :ethernet;
import :interface;
import xv.sync;

namespace xv::net
{
class ArpCache final : public EthernetProtocol
{
public:

  constexpr ArpCache(EthernetLayer &link, const NetworkInterface &interface)
      : link_(link), interface_(interface)
  {
  }

  void initialize();
  int send(Packet &packet, unsigned next_hop);
  void receive(Packet &packet) override;
  void tick(unsigned long now);
  bool neighbor(unsigned index, NeighborInfo &result);
  void clear();

private:

  struct RequestFlow;

  struct SendFlow;
  struct ReceiveFlow;
  struct TickFlow;

  void request(unsigned address);

  struct Neighbor
  {
    unsigned address = 0;
    unsigned char mac[6]{};
    unsigned long expires = 0;
  } entries_[16]{};

  struct Pending
  {
    void *page = nullptr;
    unsigned length = 0, address = 0, retries = 0;
    unsigned long deadline = 0;
    unsigned generation = 0;
    Packet::Context context{};
  } pending_[8]{};

  xv::sync::SpinLock lock_;
  EthernetLayer &link_;
  const NetworkInterface &interface_;
};
} // namespace xv::net
