module xv.net:icmp;
import :ipv4;

namespace xv::net
{
class IcmpProtocol final : public IpProtocol
{
public:

  constexpr explicit IcmpProtocol(Ipv4Layer &ip) : ip_(ip)
  {
  }

  void receive(Packet &packet, const IpDatagram &datagram) override;
  void unreachable(Packet &packet, const IpDatagram &datagram);

private:

  struct ReplyFlow;

  Ipv4Layer &ip_;
};
} // namespace xv::net
