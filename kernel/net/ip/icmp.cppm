module xv.net:icmp;
import :ipv4;

namespace xv::net
{
class IcmpEchoSink
{
public:
  virtual bool deliver_echo(Packet &packet, const IpDatagram &datagram) = 0;
protected:
  ~IcmpEchoSink() = default;
};

class IcmpProtocol final : public IpProtocol
{
public:

  constexpr explicit IcmpProtocol(Ipv4Layer &ip) : ip_(ip)
  {
  }

  void attach(IcmpEchoSink &sink) { sink_ = &sink; }
  int send_echo(const unsigned char *data, unsigned length, Endpoint source, Endpoint destination);
  void receive(Packet &packet, const IpDatagram &datagram) override;
  void unreachable(Packet &packet, const IpDatagram &datagram);

private:

  struct ReplyFlow;

  Ipv4Layer &ip_;
  IcmpEchoSink *sink_ = nullptr;
};
} // namespace xv::net
