module xv.net:tcp_socket;
import :tcp;
import :retransmission_timer;
import :stream_buffer;
import :congestion;
namespace xv::net
{
unsigned tcp_peer_mss(const unsigned char *segment, unsigned length);
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
  int segment(unsigned sequence, unsigned flags, const void *data = nullptr, unsigned length = 0);
  void emit(unsigned flags);
  void synchronize(unsigned sequence);
  void pump();
  void retransmit();
  void persist(unsigned long now);
  void acknowledge(unsigned acknowledgement, bool duplicate);
  bool update_window(unsigned sequence, unsigned acknowledgement, unsigned window);
  void finish();
  void fail(int error);
  void enter_time_wait();
  unsigned window() const;
  unsigned mss() const;
  friend class TcpProtocol;
  TcpState state_ = TcpState::Closed;
  TcpSocket *listener_ = nullptr;
  unsigned backlog_ = 0;
  unsigned send_unacknowledged_ = 0, send_next_ = 0, syn_sequence_ = 0, fin_sequence_ = 0;
  unsigned peer_window_ = 65535, peer_mss_ = 536, window_sequence_ = 0, window_ack_ = 0;
  unsigned retries_ = 0, persist_interval_ = 1000, advertised_right_ = 0;
  TcpSendBuffer transmit_;
  TcpReceiveBuffer receive_;
  TcpCongestion congestion_;
  RetransmissionTimer retransmission_;
  unsigned long idle_deadline_ = 0, persist_deadline_ = 0, last_activity_ = 0;
  bool syn_pending_ = false, fin_sent_ = false, fin_acked_ = false;
  bool read_shutdown_ = false, write_shutdown_ = false;
};
}
