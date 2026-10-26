module xv.net:registry;
import :socket;
import :udp;
import :platform;
namespace xv::net
{
class InetSocket;
class TcpSocket;
class SocketRegistry final : public DatagramSink
{
public:
  constexpr SocketRegistry(Ipv4Layer &ip, UdpProtocol &udp) : ip_(ip), udp_(udp) {}
  void initialize();
  Socket *create(int type, int protocol);
  TcpSocket *create_child();
  bool add(InetSocket &socket);
  void destroy(InetSocket &socket);
  bool port_used(unsigned protocol, unsigned short port, const InetSocket *except = nullptr, unsigned local_address = 0) const;
  unsigned short ephemeral(unsigned protocol);
  bool deliver(Packet &packet, Endpoint source, Endpoint destination, unsigned offset, unsigned length) override;
  void tick(unsigned long now);
  InetSocket *at(unsigned index) { return sockets_[index]; }
  xv::sync::SpinLock &lock() { return lock_; }
  Ipv4Layer &ip() { return ip_; }
  UdpProtocol &udp() { return udp_; }
private:
  xv::sync::SpinLock lock_;
  InetSocket *sockets_[64]{};
  unsigned short next_ephemeral_ = 49152;
  Ipv4Layer &ip_;
  UdpProtocol &udp_;
};
class InetSocket : public Socket
{
public:
  constexpr InetSocket(SocketRegistry &registry, unsigned protocol) : registry_(registry), protocol_(protocol) {}
  int bind(Endpoint local) override;
  int listen(int backlog) override;
  Socket *accept(bool nonblocking, int &error) override;
  int shutdown(int how) override;
  Endpoint local() override;
  Endpoint peer() override;
  int error() override;
  unsigned protocol() const { return protocol_; }
  Endpoint bound() const { return local_; }
  virtual void destroy() = 0;
  virtual void tick(unsigned long) {}
  bool owned() const { return owned_; }
protected:
  int auto_bind(unsigned destination);
  void notify();
  void wait();
  SocketRegistry &registry_;
  const unsigned protocol_;
  Endpoint local_{}, peer_{};
  int error_ = 0;
  bool owned_ = true;
};
bool legacy_port_used(unsigned short port);
SocketRegistry &socket_registry();
}
