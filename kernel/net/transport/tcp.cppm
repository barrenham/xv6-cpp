module xv.net:tcp;
import :ipv4;

namespace xv::net
{
struct TcpSegment
{
  Endpoint source, destination;
  const unsigned char *data;
  unsigned length;
};

class TcpSegmentSink
{
public:

  virtual bool deliver_segment(const TcpSegment &segment) = 0;

protected:

  ~TcpSegmentSink() = default;
};

class TcpProtocol final : public IpProtocol
{
public:

  constexpr explicit TcpProtocol(Ipv4Layer &ip) : ip_(ip)
  {
  }

  void attach(TcpSegmentSink &sink)
  {
    sink_ = &sink;
  }

  void receive(Packet &packet, const IpDatagram &datagram) override;
  int transmit(Endpoint local, Endpoint peer, unsigned sequence,
               unsigned acknowledgement, unsigned flags, unsigned window,
               const void *data, unsigned length);

private:

  struct ReceiveFlow;
  struct TransmitFlow;

  Ipv4Layer &ip_;
  TcpSegmentSink *sink_ = nullptr;
};
} // namespace xv::net
