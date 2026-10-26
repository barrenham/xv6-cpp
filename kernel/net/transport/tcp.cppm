module xv.net:tcp;
import :registry;
namespace xv::net
{
class TcpProtocol final : public IpProtocol
{
public:
  constexpr explicit TcpProtocol(SocketRegistry &registry) : registry_(registry) {}
  void receive(Packet &packet, const IpDatagram &datagram) override;
  int transmit(Endpoint local, Endpoint peer, unsigned sequence, unsigned acknowledgement,
               unsigned flags, unsigned window, const void *data, unsigned length);
private:
  SocketRegistry &registry_;
};
TcpProtocol &tcp_protocol();
}
