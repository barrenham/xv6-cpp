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
  return __atomic_add_fetch(&counter,64001U,__ATOMIC_RELAXED)+detail::milliseconds()*250;
}
}
TcpSocket::TcpSocket(SocketRegistry &registry) : InetSocket(registry,6) {}
bool TcpSocket::initialize() { return transmit_.initialize() && receive_.initialize(); }
void TcpSocket::fail(int error)
{
  error_ = error; state_ = TcpState::Closed;
  send_unacknowledged_ = send_next_; syn_pending_ = false;
  transmit_.reset(send_next_); notify(); if (listener_) listener_->notify();
}
void TcpSocket::enter_time_wait()
{ state_ = TcpState::TimeWait; idle_deadline_ = detail::milliseconds()+30000; notify(); }
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
  int bound = auto_bind(peer.address); if (bound < 0) { registry_.lock().release(); return bound; }
  peer_ = peer; error_ = 0; state_ = TcpState::SynSent; peer_mss_ = 536; peer_window_ = 65535;
  receive_.reset(0); congestion_.initialize(peer_mss_); persist_deadline_ = 0; persist_interval_ = 1000;
  synchronize(initial_sequence());
  if (nonblocking) { registry_.lock().release(); return -EINPROGRESS; }
  while (state_ == TcpState::SynSent || state_ == TcpState::SynReceived)
  { if (detail::interrupted()) { registry_.lock().release(); return -1; } wait(); }
  int result = state_ == TcpState::Established ? 0 : -(error_ ? error_ : ECONNREFUSED);
  registry_.lock().release(); return result;
}
int TcpSocket::listen(int backlog)
{
  xv::sync::ScopedLock guard(registry_.lock());
  if (state_ != TcpState::Closed && state_ != TcpState::Listen) return -1;
  if (!local_.port) local_.port = registry_.ephemeral(6);
  if (!local_.port) return -EADDRINUSE;
  backlog_ = backlog < 1 ? 1 : (backlog > 8 ? 8 : backlog); state_ = TcpState::Listen; return 0;
}
void TcpSocket::passive(TcpSocket &listener, Endpoint local, Endpoint peer, const unsigned char *syn, unsigned length)
{
  owned_ = false; listener_ = &listener; local_ = local; peer_ = peer;
  peer_mss_ = tcp_peer_mss(syn,length); if (!peer_mss_) { fail(ECONNRESET); return; }
  unsigned sequence = wire::read32(syn+4); receive_.reset(sequence+1);
  peer_window_ = wire::read16(syn+14); window_sequence_ = sequence; window_ack_ = 0;
  unsigned segment = mss(); congestion_.initialize(segment ? segment : peer_mss_);
  state_ = TcpState::SynReceived; synchronize(initial_sequence());
}
Socket *TcpSocket::accept(bool nonblocking, int &error)
{
  registry_.lock().acquire();
  for (;;)
  {
    if (state_ != TcpState::Listen) { error = -1; registry_.lock().release(); return nullptr; }
    for (unsigned i = 0; i < 64; ++i)
    {
      auto *s = registry_.at(i); if (!s || s->protocol() != 6) continue;
      auto *child = static_cast<TcpSocket *>(s);
      if (child->listener_ == this && (child->state_ == TcpState::Established || child->state_ == TcpState::CloseWait))
      { child->owned_ = true; child->listener_ = nullptr; registry_.lock().release(); error = 0; return child; }
    }
    if (detail::interrupted()) { error = -1; registry_.lock().release(); return nullptr; }
    if (nonblocking) { error = -EAGAIN; registry_.lock().release(); return nullptr; }
    wait();
  }
}
int TcpSocket::read(xv::io::Buffer destination, int count)
{
  if (count < 0) return -1; if (!count) return 0;
  unsigned char staged[1024]; registry_.lock().acquire();
  for (;;)
  {
    if (receive_.readable())
    {
      unsigned amount = static_cast<unsigned>(count); if (amount > sizeof(staged)) amount = sizeof(staged);
      unsigned before = window(), n = receive_.read(staged,amount), after = window();
      if (!before || after == TcpPageBuffer::Capacity || receive_.next()+after-advertised_right_ >= peer_mss_) emit(16);
      registry_.lock().release();
      return detail::copy_out(destination.address,staged,n) < 0 ? -1 : static_cast<int>(n);
    }
    if (read_shutdown_ || receive_.finished()) { registry_.lock().release(); return 0; }
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
  if (count < 0) return -1; if (!count) return 0;
  unsigned char staged[1024]; unsigned n = static_cast<unsigned>(count);
  if (n > sizeof(staged)) n = sizeof(staged);
  if (detail::copy_in(staged,source.address,n) < 0) return -1;
  registry_.lock().acquire();
  for (;;)
  {
    if (write_shutdown_) { registry_.lock().release(); return -EPIPE; }
    if (error_) { int error = -error_; registry_.lock().release(); return error; }
    if (state_ != TcpState::Established && state_ != TcpState::CloseWait)
    { registry_.lock().release(); return -ENOTCONN; }
    if (transmit_.space())
    {
      if (!mss()) { registry_.lock().release(); return -ENETUNREACH; }
      if (!transmit_.size() && send_next_ == send_unacknowledged_) last_activity_ = detail::milliseconds();
      unsigned accepted = transmit_.append(staged,n); pump(); registry_.lock().release(); return accepted;
    }
    if (detail::interrupted()) { registry_.lock().release(); return -1; }
    if (source.nonblocking) { registry_.lock().release(); return -EAGAIN; }
    wait();
  }
}
int TcpSocket::send_to(xv::io::Buffer data, unsigned length, const Endpoint *peer)
{ return peer ? -EISCONN : write(data,length); }
int TcpSocket::receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer)
{ int result = read(data,length); if (result >= 0 && peer) *peer = this->peer(); return result; }
unsigned TcpSocket::poll(xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(registry_.lock()); if (table) table->watch(this);
  unsigned events = error_ ? POLLERR : 0;
  if (state_ == TcpState::Listen)
  {
    for (unsigned i = 0; i < 64; ++i)
      if (auto *s = registry_.at(i); s && s->protocol() == 6)
      {
        auto *child = static_cast<TcpSocket *>(s);
        if (child->listener_ == this && (child->state_ == TcpState::Established || child->state_ == TcpState::CloseWait)) events |= POLLIN;
      }
    return events;
  }
  if (receive_.readable() || receive_.finished() || read_shutdown_) events |= POLLIN;
  if ((state_ == TcpState::Established || state_ == TcpState::CloseWait) && !write_shutdown_ && transmit_.space()) events |= POLLOUT;
  if (state_ == TcpState::Closed && (peer_.port || error_)) events |= POLLHUP | POLLOUT;
  return events;
}
int TcpSocket::shutdown(int how)
{
  if (how < 0 || how > 2) return -1;
  xv::sync::ScopedLock guard(registry_.lock());
  if (state_ != TcpState::Established && state_ != TcpState::CloseWait && state_ != TcpState::FinWait1 && state_ != TcpState::FinWait2) return -ENOTCONN;
  if (how == 0 || how == 2) { read_shutdown_ = true; receive_.discard(); emit(16); }
  if (how == 1 || how == 2) { if (!write_shutdown_) last_activity_ = detail::milliseconds(); write_shutdown_ = true; pump(); }
  notify(); return 0;
}
void TcpSocket::close()
{
  xv::sync::ScopedLock guard(registry_.lock()); owned_ = false;
  if (state_ == TcpState::Listen)
  {
    for (unsigned i = 0; i < 64; ++i)
      if (auto *s = registry_.at(i); s && s->protocol() == 6 && static_cast<TcpSocket *>(s)->listener_ == this)
      { auto *child = static_cast<TcpSocket *>(s); child->emit(20); registry_.destroy(*child); }
    state_ = TcpState::Closed;
  }
  else if (state_ == TcpState::SynSent || state_ == TcpState::SynReceived) { emit(20); state_ = TcpState::Closed; }
  else { if (!write_shutdown_) last_activity_ = detail::milliseconds(); read_shutdown_ = write_shutdown_ = true; receive_.discard(); pump(); }
  if (state_ == TcpState::Closed) registry_.destroy(*this);
}
void TcpSocket::destroy()
{ transmit_.release(); receive_.release(); this->~TcpSocket(); detail::free_page(this); }
void TcpSocket::tick(unsigned long now)
{
  bool pending = send_next_ != send_unacknowledged_;
  if (pending && (peer_window_ || syn_pending_) && retransmission_.expired(now))
  {
    if (retries_++ >= 6) { emit(20); fail(ETIMEDOUT); }
    else
    {
      if (!syn_pending_) congestion_.timeout(send_next_-send_unacknowledged_,send_next_);
      retransmit(); retransmission_.retransmit(now);
    }
  }
  if ((state_ == TcpState::TimeWait || state_ == TcpState::FinWait2) && now >= idle_deadline_) { state_ = TcpState::Closed; notify(); }
  if (state_ == TcpState::Established || state_ == TcpState::CloseWait || state_ == TcpState::FinWait1 || state_ == TcpState::LastAck)
  {
    if ((transmit_.size() || (write_shutdown_ && !fin_acked_)) && now >= last_activity_ && now-last_activity_ >= 120000) { emit(20); fail(ETIMEDOUT); }
    else { persist(now); pump(); }
  }
  if (!owned_ && state_ == TcpState::Closed) registry_.destroy(*this);
}
}
