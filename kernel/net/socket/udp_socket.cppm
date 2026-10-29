module xv.net:udp_socket;
import :registry;

namespace xv::net
{
class UdpSocket final : public InetSocket
{
public:

  explicit UdpSocket(SocketRegistry &registry) : InetSocket(registry, 17)
  {
  }

  int connect(Endpoint peer, bool nonblocking) override;
  int read(xv::io::Buffer destination, int count) override;
  int write(xv::io::Buffer source, int count) override;
  int send_to(xv::io::Buffer data, unsigned length, const Endpoint *peer) override;
  int receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer) override;
  unsigned poll(xv::sync::PollTable *table) override;
  void close() override;
  void destroy() override;
  bool deliver(Packet &packet, Endpoint source, Endpoint destination, unsigned offset,
               unsigned length);

private:

  struct ConnectFlow;
  struct DeliveryFlow;

  struct SendFlow;
  struct ReceiveFlow;

  struct Message
  {
    void *page = nullptr;
    Endpoint source{};
    unsigned short offset = 0, length = 0;
    Packet::Context context{};
  } messages_[16]{};

  unsigned head_ = 0, count_ = 0;
};
} // namespace xv::net
