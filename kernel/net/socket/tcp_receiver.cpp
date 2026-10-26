module;
#include "kernel/net/errno.h"
module xv.net;
import :tcp_socket;
namespace xv::net
{
unsigned tcp_peer_mss(const unsigned char *p, unsigned length)
{
  unsigned header = (p[12]>>4)*4, mss = 536; bool found = false;
  if (header < 20 || header > length) return 0;
  for (unsigned i = 20; i < header;)
  {
    if (!p[i]) break;
    if (p[i] == 1) { ++i; continue; }
    if (i+1 >= header || p[i+1] < 2 || i+p[i+1] > header) return 0;
    if (p[i] == 2)
    { if (found || p[i+1] != 4) return 0; mss = wire::read16(p+i+2); if (!mss) return 0; found = true; }
    i += p[i+1];
  }
  return mss < 1460 ? mss : 1460;
}
unsigned TcpSocket::window() const { return receive_.window(); }
void TcpSocket::incoming(Endpoint, Endpoint, const unsigned char *p, unsigned length)
{
  unsigned flags = p[13], sequence = wire::read32(p+4), acknowledgement = wire::read32(p+8);
  unsigned header = (p[12]>>4)*4, payload = length-header;
  auto now = detail::milliseconds(); bool simultaneous = false;
  if (state_ == TcpState::SynSent)
  {
    if ((flags&16) && acknowledgement != send_next_)
    { if (!(flags&4)) tcp_protocol().transmit(local_,peer_,acknowledgement,0,4,0,nullptr,0); return; }
    if (flags&4) { if (flags&16) fail(ECONNREFUSED); return; }
    if (!(flags&2)) return;
    peer_mss_ = tcp_peer_mss(p,length); if (!peer_mss_) return;
    receive_.reset(sequence+1); peer_window_ = wire::read16(p+14);
    window_sequence_ = sequence; window_ack_ = acknowledgement; last_activity_ = now;
    unsigned segment = mss(); congestion_.initialize(segment ? segment : peer_mss_);
    if (!(flags&16))
    { state_ = TcpState::SynReceived; retransmit(); return; }
    acknowledge(acknowledgement,false); state_ = TcpState::Established; notify();
    ++sequence; flags &= ~2U;
    if (!payload && !(flags&1)) { emit(16); pump(); return; }
  }
  if (state_ == TcpState::TimeWait)
  {
    if (!(flags&4) && (flags&1) && sequence+payload+1 == receive_.next())
    { emit(16); idle_deadline_ = now+30000; }
    return;
  }
  if (state_ == TcpState::SynReceived && (flags&2) && sequence+1 == receive_.next())
  {
    if (!(flags&16)) { retransmit(); return; }
    if (acknowledgement != send_next_) return;
    ++sequence; flags &= ~2U; simultaneous = true;
  }
  unsigned effective = payload+((flags&1) ? 1 : 0);
  if (!receive_.acceptable(sequence,effective)) { if (!(flags&4)) emit(16); return; }
  if (flags&4)
  { if (sequence == receive_.next()) fail(ECONNRESET); else emit(16); return; }
  if (flags&2) { emit(16); return; }
  if (!(flags&16)) return;
  if (wire::before(send_next_,acknowledgement)) { emit(16); return; }
  if (state_ == TcpState::SynReceived && (acknowledgement != send_next_ || sequence != receive_.next())) return;
  bool reopened = !peer_window_;
  bool changed = update_window(sequence,acknowledgement,wire::read16(p+14));
  bool duplicate = !payload && !(flags&1) && !changed && acknowledgement == send_unacknowledged_;
  acknowledge(acknowledgement,duplicate);
  if (state_ == TcpState::Closed) return;
  if (reopened && peer_window_ && send_unacknowledged_ != send_next_)
  { retransmit(); retransmission_.start(now); }
  last_activity_ = now;
  if (state_ == TcpState::SynReceived)
  {
    state_ = TcpState::Established;
    if (listener_) listener_->notify();
    notify();
  }
  bool finished = receive_.finished();
  unsigned progress = receive_.insert(sequence,p+header,payload,flags&1);
  if (read_shutdown_) receive_.discard();
  if (!finished && receive_.finished())
  {
    if (state_ == TcpState::Established) state_ = TcpState::CloseWait;
    else if (state_ == TcpState::FinWait2) enter_time_wait();
    else if (state_ == TcpState::FinWait1) state_ = TcpState::Closing;
  }
  if (payload || (flags&1) || simultaneous) emit(16);
  if (progress || changed) notify();
  pump();
}
}
