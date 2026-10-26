export module xv.net:ipv4;
export import :routing;
import :wire;
import xv.sync;
export namespace xv::net
{
struct IpDatagram
{
  unsigned source, destination, protocol, offset, length;
  NetworkInterface *interface;
};
class IpProtocol
{
public:
  virtual void receive(Packet &packet, const IpDatagram &datagram) = 0;
protected:
  ~IpProtocol() = default;
};
class Ipv4Layer final : public IpReceiver
{
public:
  constexpr Ipv4Layer(InterfaceRegistry &interfaces, RoutingTable &routes)
      : interfaces_(interfaces), routes_(routes) {}
  void initialize();
  void attach(unsigned protocol, IpProtocol &handler);
  Packet allocate(unsigned length);
  int send(Packet &packet, unsigned destination, unsigned protocol, unsigned source = 0,
           const NetworkInterface *via = nullptr);
  unsigned source_for(unsigned destination, unsigned requested = 0, const NetworkInterface *via = nullptr) const;
  unsigned mtu_for(unsigned destination, unsigned requested = 0) const;
  bool local(unsigned address) const { return interfaces_.owns(address); }
  void receive(NetworkInterface &interface, Packet &packet) override;
  void tick(unsigned long now);
private:
  void fragment(NetworkInterface &interface, Packet &packet, unsigned header, unsigned total);
  struct Binding { unsigned protocol = 0; IpProtocol *handler = nullptr; } handlers_[4]{};
  struct Assembly
  {
    void *page = nullptr;
    unsigned source = 0, destination = 0, id = 0, protocol = 0, total = 0, received = 0;
    NetworkInterface *interface = nullptr;
    bool last = false;
    unsigned char present[508]{};
    unsigned long deadline = 0;
  } assemblies_[4]{};
  xv::sync::SpinLock lock_;
  unsigned identification_ = 0;
  InterfaceRegistry &interfaces_;
  RoutingTable &routes_;
};
}
