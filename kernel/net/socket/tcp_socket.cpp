module;
#include "kernel/net/errno.h"
#include "kernel/fs/poll.h"
module xv.net;
import :tcp_socket;
namespace xv::net
{
namespace
{
unsigned initial_sequence()
{
  static unsigned counter = 0x584e0000;
  return __atomic_add_fetch(&counter, 64001U, __ATOMIC_RELAXED) + detail::milliseconds()*250;
}
unsigned peer_mss(const unsigned char *p, unsigned length)
{
  unsigned header = (p[12]>>4)*4, mss = 536;
  if (header > length) return 0;
  for (unsigned i = 20; i < header;)
  {
    if (!p[i]) break;
    if (p[i] == 1) { ++i; continue; }
    if (i+1 >= header || p[i+1] < 2 || i+p[i+1] > header) return 0;
    if (p[i] == 2 && p[i+1] == 4) mss = wire::read16(p+i+2);
    i += p[i+1];
  }
  return mss && mss < 1460 ? mss : 1460;
}
}
TcpSocket::TcpSocket(SocketRegistry &registry) : InetSocket(registry, 6) {}
bool TcpSocket::initialize()
{
  receive_buffer_ = static_cast<unsigned char *>(detail::allocate_page());
  return receive_buffer_ != nullptr;
}
unsigned TcpSocket::window() const { return read_shutdown_ ? 4096 : 4096-receive_count_; }
void TcpSocket::emit(unsigned flags, const void *data, unsigned length)
{
  tcp_protocol().transmit(local_, peer_, send_next_, receive_next_, flags, window(), data, length);
}
void TcpSocket::queue(unsigned flags, const void *data, unsigned length)
{
  if (length) wire::copy(transmit_buffer_, data, length);
  flight_ = true; flight_sequence_ = send_next_; flight_flags_ = flags; flight_length_ = length;
  send_next_ += length + ((flags&2) ? 1 : 0) + ((flags&1) ? 1 : 0);
  retries_ = 0; retransmission_.start(detail::milliseconds());
  tcp_protocol().transmit(local_, peer_, flight_sequence_, receive_next_, flags, window(), transmit_buffer_, length);
}
void TcpSocket::fail(int error)
{
  error_ = error; state_ = TcpState::Closed; flight_ = false; notify();
  if (listener_) listener_->notify();
}
void TcpSocket::enter_time_wait()
{ state_ = TcpState::TimeWait; idle_deadline_ = detail::milliseconds()+30000; notify(); }
void TcpSocket::finish()
{
  if (!write_shutdown_ || flight_) return;
  if (state_ == TcpState::Established) { state_ = TcpState::FinWait1; queue(17, nullptr, 0); }
  else if (state_ == TcpState::CloseWait) { state_ = TcpState::LastAck; queue(17, nullptr, 0); }
}
void TcpSocket::acknowledge(unsigned acknowledgement)
{
  if (wire::before(acknowledgement, send_unacknowledged_) || wire::before(send_next_, acknowledgement)) return;
  send_unacknowledged_ = acknowledgement;
  if (!flight_ || !wire::before(flight_sequence_, acknowledgement)) return;
  unsigned end = flight_sequence_+flight_length_+((flight_flags_&3) ? 1 : 0);
  if (!wire::before(acknowledgement, end))
  {
    retransmission_.acknowledge(detail::milliseconds());
    bool fin = flight_flags_&1; flight_ = false; flight_length_ = 0;
    if (fin)
    {
      if (state_ == TcpState::FinWait1)
      { state_ = TcpState::FinWait2; idle_deadline_ = detail::milliseconds()+30000; }
      else if (state_ == TcpState::Closing) enter_time_wait();
      else if (state_ == TcpState::LastAck) state_ = TcpState::Closed;
    }
    finish(); notify(); return;
  }
  unsigned consumed = acknowledgement-flight_sequence_;
  if (consumed <= flight_length_)
  {
    for (unsigned i = consumed; i < flight_length_; ++i) transmit_buffer_[i-consumed] = transmit_buffer_[i];
    flight_length_ -= consumed; flight_sequence_ = acknowledgement;
    retransmission_.restart(detail::milliseconds()); retries_ = 0;
  }
}
bool TcpSocket::matches(Endpoint local, Endpoint peer) const
{
  return state_ != TcpState::Closed && state_ != TcpState::Listen && local_.port == local.port &&
         local_.address == local.address && peer_.port == peer.port && peer_.address == peer.address;
}
bool TcpSocket::listening(Endpoint local) const
{ return state_ == TcpState::Listen && local_.port == local.port && (!local_.address || local_.address == local.address); }
int TcpSocket::connect(Endpoint peer, bool nonblocking)
{
  if (!peer.address || !peer.port) return -1;
  registry_.lock().acquire();
  if (state_ != TcpState::Closed)
  { int result = state_ == TcpState::SynSent ? -EALREADY : -EISCONN; registry_.lock().release(); return result; }
  int bound = auto_bind(peer.address);
  if (bound < 0) { registry_.lock().release(); return bound; }
  peer_ = peer; error_ = 0; state_ = TcpState::SynSent;
  send_unacknowledged_ = send_next_ = initial_sequence();
  queue(2, nullptr, 0);
  if (nonblocking) { registry_.lock().release(); return -EINPROGRESS; }
  while (state_ == TcpState::SynSent || state_ == TcpState::SynReceived)
  {
    if (detail::interrupted()) { registry_.lock().release(); return -1; }
    wait();
  }
  int result = state_ == TcpState::Established ? 0 : -(error_ ? error_ : ECONNREFUSED);
  registry_.lock().release(); return result;
}
int TcpSocket::listen(int backlog)
{
  xv::sync::ScopedLock guard(registry_.lock());
  if (state_ != TcpState::Closed && state_ != TcpState::Listen) return -1;
  if (!local_.port) local_.port = registry_.ephemeral(6);
  if (!local_.port) return -EADDRINUSE;
  backlog_ = backlog < 1 ? 1 : (backlog > 8 ? 8 : backlog);
  state_ = TcpState::Listen; return 0;
}
void TcpSocket::passive(TcpSocket &listener, Endpoint local, Endpoint peer,
                         const unsigned char *syn, unsigned length)
{
  owned_ = false; listener_ = &listener; local_ = local; peer_ = peer;
  peer_window_ = wire::read16(syn+14); peer_mss_ = peer_mss(syn, length);
  if (!peer_mss_) { fail(ECONNRESET); return; }
  receive_next_ = wire::read32(syn+4)+1;
  send_unacknowledged_ = send_next_ = initial_sequence();
  state_ = TcpState::SynReceived; queue(18, nullptr, 0);
}
Socket *TcpSocket::accept(bool nonblocking, int &error)
{
  registry_.lock().acquire();
  for (;;)
  {
    if (state_ != TcpState::Listen) { error = -1; registry_.lock().release(); return nullptr; }
    for (unsigned i = 0; i < 64; ++i)
    {
      auto *s = registry_.at(i);
      if (!s || s->protocol() != 6) continue;
      auto *child = static_cast<TcpSocket *>(s);
      if (child->listener_ == this &&
          (child->state_ == TcpState::Established || child->state_ == TcpState::CloseWait))
      { child->owned_ = true; child->listener_ = nullptr; registry_.lock().release(); error = 0; return child; }
    }
    if (detail::interrupted()) { error = -1; registry_.lock().release(); return nullptr; }
    if (nonblocking) { error = -EAGAIN; registry_.lock().release(); return nullptr; }
    wait();
  }
}
void TcpSocket::incoming(Endpoint, Endpoint, const unsigned char *p, unsigned length)
{
  unsigned flags = p[13], sequence = wire::read32(p+4), acknowledgement = wire::read32(p+8);
  unsigned header = (p[12]>>4)*4, payload = length-header;
  if (state_ == TcpState::SynSent)
  {
    if ((flags&16) && acknowledgement != send_next_) return;
    if (flags&4) { if (flags&16) fail(ECONNREFUSED); return; }
    if (!(flags&2)) return;
    peer_mss_ = peer_mss(p, length); if (!peer_mss_) return;
    receive_next_ = sequence+1; peer_window_ = wire::read16(p+14);
    if (flags&16)
    { acknowledge(acknowledgement); state_ = TcpState::Established; emit(16); notify(); }
    else
    {
      state_ = TcpState::SynReceived; flight_flags_ = 18;
      tcp_protocol().transmit(local_, peer_, flight_sequence_, receive_next_, 18, window(), nullptr, 0);
    }
    return;
  }
  if (state_ == TcpState::TimeWait)
  { if (flags&1) { emit(16); idle_deadline_ = detail::milliseconds()+30000; } return; }
  if (state_ == TcpState::SynReceived && (flags&2) && sequence+1 == receive_next_)
  {
    tcp_protocol().transmit(local_, peer_, flight_sequence_, receive_next_, 18, window(), nullptr, 0); return;
  }
  unsigned effective = payload+((flags&1) ? 1 : 0);
  bool acceptable = sequence == receive_next_ ||
      (effective && wire::before(sequence, receive_next_) && !wire::before(sequence+effective, receive_next_)) ||
      (window() && !wire::before(sequence, receive_next_) && wire::before(sequence, receive_next_+window()));
  if (!acceptable) { if (!(flags&4)) emit(16); return; }
  if (flags&4)
  { if (sequence == receive_next_) fail(ECONNRESET); else emit(16); return; }
  if (flags&2) { emit(16); return; }
  if (!(flags&16)) return;
  if (wire::before(send_next_, acknowledgement)) { emit(16); return; }
  if (state_ == TcpState::SynReceived)
  {
    if (acknowledgement != send_next_ || sequence != receive_next_) return;
    acknowledge(acknowledgement); state_ = TcpState::Established;
    if (listener_) listener_->notify(); notify();
  }
  else acknowledge(acknowledgement);
  peer_window_ = wire::read16(p+14); notify();
  if (state_ == TcpState::Closed) return;
  if (sequence != receive_next_ && wire::before(receive_next_, sequence))
  { if (effective) emit(16); return; }
  unsigned skipped = receive_next_-sequence;
  if (skipped > payload) skipped = payload;
  unsigned available = payload-skipped;
  unsigned accepted = available < window() ? available : window();
  if (peer_finished_) accepted = 0;
  for (unsigned i = 0; i < accepted; ++i)
    if (!read_shutdown_) receive_buffer_[(receive_head_+receive_count_+i)%4096] = p[header+skipped+i];
  if (!read_shutdown_) receive_count_ += accepted;
  receive_next_ += accepted;
  bool fin = (flags&1) && sequence+payload == receive_next_ && !peer_finished_;
  if (fin)
  {
    ++receive_next_; peer_finished_ = true;
    if (state_ == TcpState::Established) state_ = TcpState::CloseWait;
    else if (state_ == TcpState::FinWait2) enter_time_wait();
    else if (state_ == TcpState::FinWait1) state_ = TcpState::Closing;
  }
  if (payload || (flags&1)) emit(16);
  if (accepted || fin) notify();
}
int TcpSocket::read(xv::io::Buffer destination, int count)
{
  if (count < 0) return -1;
  if (!count) return 0;
  unsigned char staged[512];
  registry_.lock().acquire();
  for (;;)
  {
    if (receive_count_)
    {
      unsigned n = receive_count_ < static_cast<unsigned>(count) ? receive_count_ : static_cast<unsigned>(count);
      if (n > sizeof(staged)) n = sizeof(staged);
      for (unsigned i = 0; i < n; ++i) staged[i] = receive_buffer_[(receive_head_+i)%4096];
      receive_head_ = (receive_head_+n)%4096; receive_count_ -= n;
      emit(16); registry_.lock().release();
      return detail::copy_out(destination.address, staged, n) < 0 ? -1 : static_cast<int>(n);
    }
    if (read_shutdown_ || peer_finished_) { registry_.lock().release(); return 0; }
    if (error_) { int error = -error_; registry_.lock().release(); return error; }
    if (state_ != TcpState::Established && state_ != TcpState::FinWait1 && state_ != TcpState::FinWait2)
    { registry_.lock().release(); return -ENOTCONN; }
    if (detail::interrupted()) { registry_.lock().release(); return -1; }
    if (destination.nonblocking) { registry_.lock().release(); return -EAGAIN; }
    wait();
  }
}
int TcpSocket::write(xv::io::Buffer source, int count)
{
  if (count < 0) return -1;
  if (!count) return 0;
  unsigned char staged[1460];
  unsigned n = static_cast<unsigned>(count) < sizeof(staged) ? count : sizeof(staged);
  if (detail::copy_in(staged, source.address, n) < 0) return -1;
  registry_.lock().acquire();
  for (;;)
  {
    if (write_shutdown_) { registry_.lock().release(); return -EPIPE; }
    if (error_) { int error = -error_; registry_.lock().release(); return error; }
    if (state_ != TcpState::Established && state_ != TcpState::CloseWait)
    { registry_.lock().release(); return -ENOTCONN; }
    if (!flight_ && peer_window_)
    {
      unsigned mtu = registry_.ip().mtu_for(peer_.address, local_.address);
      if (mtu < 68) { registry_.lock().release(); return -ENETUNREACH; }
      if (n > mtu-40) n = mtu-40;
      if (n > peer_mss_) n = peer_mss_;
      if (n > peer_window_) n = peer_window_;
      queue(24, staged, n); registry_.lock().release(); return n;
    }
    if (detail::interrupted()) { registry_.lock().release(); return -1; }
    if (source.nonblocking) { registry_.lock().release(); return -EAGAIN; }
    wait();
  }
}
int TcpSocket::send_to(xv::io::Buffer data, unsigned length, const Endpoint *peer)
{ return peer ? -EISCONN : write(data, length); }
int TcpSocket::receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer)
{ int result = read(data, length); if (result >= 0 && peer) *peer = this->peer(); return result; }
unsigned TcpSocket::poll(xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(registry_.lock());
  if (table) table->watch(this);
  unsigned events = error_ ? POLLERR : 0;
  if (state_ == TcpState::Listen)
  {
    for (unsigned i = 0; i < 64; ++i)
      if (auto *s = registry_.at(i); s && s->protocol() == 6)
      {
        auto *child = static_cast<TcpSocket *>(s);
        if (child->listener_ == this && (child->state_ == TcpState::Established || child->state_ == TcpState::CloseWait))
          events |= POLLIN;
      }
    return events;
  }
  if (receive_count_ || peer_finished_ || read_shutdown_) events |= POLLIN;
  if ((state_ == TcpState::Established || state_ == TcpState::CloseWait) && !write_shutdown_ && !flight_ && peer_window_)
    events |= POLLOUT;
  if (state_ == TcpState::Closed && (peer_.port || error_)) events |= POLLHUP | POLLOUT;
  return events;
}
int TcpSocket::shutdown(int how)
{
  if (how < 0 || how > 2) return -1;
  xv::sync::ScopedLock guard(registry_.lock());
  if (state_ != TcpState::Established && state_ != TcpState::CloseWait && state_ != TcpState::FinWait1 && state_ != TcpState::FinWait2)
    return -ENOTCONN;
  if (how == 0 || how == 2) { read_shutdown_ = true; receive_count_ = 0; emit(16); }
  if (how == 1 || how == 2) { write_shutdown_ = true; finish(); }
  notify(); return 0;
}
void TcpSocket::close()
{
  xv::sync::ScopedLock guard(registry_.lock());
  owned_ = false;
  if (state_ == TcpState::Listen)
  {
    for (unsigned i = 0; i < 64; ++i)
      if (auto *s = registry_.at(i); s && s->protocol() == 6 && static_cast<TcpSocket *>(s)->listener_ == this)
      { auto *child = static_cast<TcpSocket *>(s); child->emit(20); registry_.destroy(*child); }
    state_ = TcpState::Closed;
  }
  else if (state_ == TcpState::SynSent || state_ == TcpState::SynReceived)
  { emit(20); state_ = TcpState::Closed; }
  else { write_shutdown_ = true; finish(); }
  if (state_ == TcpState::Closed) registry_.destroy(*this);
}
void TcpSocket::destroy()
{
  if (receive_buffer_) detail::free_page(receive_buffer_);
  this->~TcpSocket(); detail::free_page(this);
}
void TcpSocket::tick(unsigned long now)
{
  if (flight_ && retransmission_.expired(now))
  {
    if (retries_++ >= 6) { emit(20); fail(ETIMEDOUT); }
    else
    {
      tcp_protocol().transmit(local_, peer_, flight_sequence_, receive_next_, flight_flags_, window(), transmit_buffer_, flight_length_);
      retransmission_.retransmit(now);
    }
  }
  if ((state_ == TcpState::TimeWait || state_ == TcpState::FinWait2) && now >= idle_deadline_)
  { state_ = TcpState::Closed; notify(); }
  if ((state_ == TcpState::Established || state_ == TcpState::CloseWait) && !peer_window_ && now >= idle_deadline_)
  {
    tcp_protocol().transmit(local_, peer_, send_next_-1, receive_next_, 16, window(), nullptr, 0);
    idle_deadline_ = now+1000;
  }
  if (!owned_ && state_ == TcpState::Closed) registry_.destroy(*this);
}
}
