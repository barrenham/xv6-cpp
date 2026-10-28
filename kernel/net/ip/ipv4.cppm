export module xv.net:ipv4;
export import :routing;
import :wire;
import :filter_hook;
import :reassembly;

export namespace xv::net
{
struct IpDatagram
{
  unsigned source, destination, protocol, offset, length;
  NetworkInterface *interface;
};

class Ipv4Bootstrap
{
public:

  virtual bool receive_bootstrap(NetworkInterface &interface, Packet &packet,
                                 const IpDatagram &datagram) = 0;

protected:

  ~Ipv4Bootstrap() = default;
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
      : interfaces_(interfaces), routes_(routes)
  {
  }

  void initialize();

  void attach_filter(PacketFilter &filter)
  {
    filter_ = &filter;
  }

  void attach(unsigned protocol, IpProtocol &handler);

  void attach_bootstrap(Ipv4Bootstrap &handler)
  {
    bootstrap_ = &handler;
  }

  int send_bootstrap(NetworkInterface &interface, Packet &packet, unsigned source = 0);
  Packet allocate(unsigned length);
  int send(Packet &packet, unsigned destination, unsigned protocol, unsigned source = 0,
           const NetworkInterface *via = nullptr);
  unsigned source_for(unsigned destination, unsigned requested = 0,
                      const NetworkInterface *via = nullptr) const;
  unsigned mtu_for(unsigned destination, unsigned requested = 0) const;

  bool local(unsigned address) const
  {
    return interfaces_.owns(address);
  }

  void receive(NetworkInterface &interface, Packet &packet) override;
  void tick(unsigned long now);

private:

  void write_header(Packet &packet, unsigned source, unsigned destination,
                    unsigned protocol);
  bool output_allowed(const Packet &packet, const NetworkInterface &interface);

  struct Binding
  {
    unsigned protocol = 0;
    IpProtocol *handler = nullptr;
  } handlers_[4]{};

  Ipv4Reassembly reassembly_;
  unsigned identification_ = 0;
  PacketFilter *filter_ = nullptr;
  Ipv4Bootstrap *bootstrap_ = nullptr;
  InterfaceRegistry &interfaces_;
  RoutingTable &routes_;
};
} // namespace xv::net
