module xv.net:registry;
import :socket;
import :udp;
import :tcp;
import :icmp;
import :platform;

namespace xv::net
{
class InetSocket;
class TcpSocket;

class SocketRegistry final : public DatagramSink, public TcpSegmentSink, public IcmpEchoSink
{
public:

  constexpr SocketRegistry(Ipv4Layer &ip, UdpProtocol &udp, TcpProtocol &tcp, IcmpProtocol *icmp = nullptr)
      : ip_(ip), udp_(udp), tcp_(tcp), icmp_(icmp)
  {
  }

  void initialize();
  Socket *create(int type, int protocol);
  void destroy(InetSocket &socket);
  bool port_used(unsigned protocol, unsigned short port,
                 const InetSocket *except = nullptr, unsigned local_address = 0) const;
  unsigned short ephemeral(unsigned protocol);
  unsigned initial_sequence();
  bool deliver(Packet &packet, Endpoint source, Endpoint destination, unsigned offset,
               unsigned length) override;
  bool deliver_echo(Packet &packet, const IpDatagram &datagram) override;
  void tick(unsigned long now);
  bool deliver_segment(const TcpSegment &segment) override;
  TcpSocket *accept_child(TcpSocket &listener);
  bool has_ready_child(const TcpSocket &listener) const;
  void close_children(TcpSocket &listener);

  xv::sync::SpinLock &lock()
  {
    return lock_;
  }

  Ipv4Layer &ip()
  {
    return ip_;
  }

  UdpProtocol &udp()
  {
    return udp_;
  }

  IcmpProtocol &icmp() { return *icmp_; }

  TcpProtocol &tcp()
  {
    return tcp_;
  }

private:

  struct CreationFlow;

  struct DeliveryFlow;

  static constexpr unsigned Capacity = 64;
  bool add(InetSocket &socket);
  TcpSocket *create_tcp();
  TcpSocket *ready_child(const TcpSocket &listener) const;
  unsigned child_count(const TcpSocket &listener) const;
  xv::sync::SpinLock lock_;
  InetSocket *sockets_[Capacity]{};
  unsigned short next_ephemeral_ = 49152;
  unsigned sequence_counter_ = 0x584e0000;
  Ipv4Layer &ip_;
  UdpProtocol &udp_;
  TcpProtocol &tcp_;
  IcmpProtocol *icmp_;
};

class InetSocket : public Socket
{
  struct BindFlow;

public:

  constexpr InetSocket(SocketRegistry &registry, unsigned protocol)
      : registry_(registry), protocol_(protocol)
  {
  }

  int bind(Endpoint local) override;
  int listen(int backlog) override;
  Socket *accept(bool nonblocking, int &error) override;
  int shutdown(int how) override;
  Endpoint local() override;
  Endpoint peer() override;
  int error() override;

  unsigned protocol() const
  {
    return protocol_;
  }

  Endpoint bound() const
  {
    return local_;
  }

  virtual void destroy() = 0;

  virtual void tick(unsigned long)
  {
  }

  bool owned() const
  {
    return owned_;
  }

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

SocketRegistry &socket_registry();
} // namespace xv::net
