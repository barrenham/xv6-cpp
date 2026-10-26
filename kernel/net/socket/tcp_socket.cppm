module xv.net:tcp_socket;
import :tcp;
import :retransmission_timer;
namespace xv::net
{
enum class TcpState { Closed, Listen, SynSent, SynReceived, Established, FinWait1, FinWait2,
                      Closing, CloseWait, LastAck, TimeWait };
class TcpSocket final : public InetSocket
{
public:
  explicit TcpSocket(SocketRegistry &registry);
  bool initialize();
  int connect(Endpoint peer, bool nonblocking) override;
  int listen(int backlog) override;
  Socket *accept(bool nonblocking, int &error) override;
  int read(xv::io::Buffer destination, int count) override;
  int write(xv::io::Buffer source, int count) override;
  int send_to(xv::io::Buffer data, unsigned length, const Endpoint *peer) override;
  int receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer) override;
  unsigned poll(xv::sync::PollTable *table) override;
  int shutdown(int how) override;
  void close() override;
  void destroy() override;
  void tick(unsigned long now) override;
  bool matches(Endpoint local, Endpoint peer) const;
  bool listening(Endpoint local) const;
  void incoming(Endpoint source, Endpoint destination, const unsigned char *segment, unsigned length);
  TcpState state() const { return state_; }
  void passive(TcpSocket &listener, Endpoint local, Endpoint peer, const unsigned char *syn, unsigned length);
private:
  void emit(unsigned flags, const void *data = nullptr, unsigned length = 0);
  void queue(unsigned flags, const void *data, unsigned length);
  void acknowledge(unsigned acknowledgement);
  void finish();
  void fail(int error);
  void enter_time_wait();
  unsigned window() const;
  friend class TcpProtocol;
  TcpState state_ = TcpState::Closed;
  TcpSocket *listener_ = nullptr;
  unsigned backlog_ = 0;
  unsigned send_unacknowledged_ = 0, send_next_ = 0, receive_next_ = 0;
  unsigned peer_window_ = 65535, peer_mss_ = 536;
  unsigned receive_head_ = 0, receive_count_ = 0;
  unsigned char *receive_buffer_ = nullptr;
  unsigned char transmit_buffer_[1460]{};
  unsigned flight_sequence_ = 0, flight_flags_ = 0, flight_length_ = 0, retries_ = 0;
  RetransmissionTimer retransmission_;
  unsigned long idle_deadline_ = 0;
  bool flight_ = false, read_shutdown_ = false, write_shutdown_ = false, peer_finished_ = false;
};
}
