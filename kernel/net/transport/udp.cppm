module xv.net:udp;
import :icmp;
namespace xv::net
{
class DatagramSink
{
public:
  virtual bool deliver(Packet &packet, Endpoint source, Endpoint destination,
                       unsigned offset, unsigned length) = 0;
protected:
  ~DatagramSink() = default;
};
class UdpProtocol final : public IpProtocol
{
public:
  constexpr UdpProtocol(Ipv4Layer &ip, IcmpProtocol &icmp) : ip_(ip), icmp_(icmp) {}
  void attach(DatagramSink &sink);
  void receive(Packet &packet, const IpDatagram &datagram) override;
  int send(Endpoint source, Endpoint destination, const void *data, unsigned length);
private:
  Ipv4Layer &ip_;
  IcmpProtocol &icmp_;
  DatagramSink *sink_ = nullptr;
};
}
