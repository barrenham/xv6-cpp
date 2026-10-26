module;
#include "kernel/net/errno.h"
module xv.net;
import :tcp_socket;
namespace xv::net
{
unsigned TcpSocket::mss() const
{
  unsigned mtu = registry_.ip().mtu_for(peer_.address,local_.address);
  if (mtu < 68) return 0;
  unsigned result = mtu-40;
  if (result > peer_mss_) result = peer_mss_;
  return result > 1460 ? 1460 : result;
}
int TcpSocket::segment(unsigned sequence, unsigned flags, const void *data, unsigned length)
{
  int result = tcp_protocol().transmit(local_,peer_,sequence,receive_.next(),flags,window(),data,length);
  if (result >= 0 && (flags&16)) advertised_right_ = receive_.next()+window();
  return result;
}
void TcpSocket::emit(unsigned flags) { segment(send_next_,flags); }
void TcpSocket::synchronize(unsigned sequence)
{
  syn_sequence_ = send_unacknowledged_ = sequence; send_next_ = sequence+1;
  syn_pending_ = true; fin_sent_ = fin_acked_ = false;
  read_shutdown_ = write_shutdown_ = false; retries_ = 0;
  transmit_.reset(send_next_); retransmission_.reset();
  auto now = detail::milliseconds(); last_activity_ = now;
  retransmission_.start(now); retransmission_.sent(send_next_,now);
  segment(sequence,state_ == TcpState::SynSent ? 2 : 18);
}
void TcpSocket::pump()
{
  if (state_ != TcpState::Established && state_ != TcpState::CloseWait) return;
  unsigned maximum = mss(); if (!maximum) return;
  while (wire::before(send_next_,transmit_.end()))
  {
    unsigned flight = send_next_-send_unacknowledged_;
    unsigned limit = peer_window_ < congestion_.window() ? peer_window_ : congestion_.window();
    if (flight >= limit) break;
    unsigned amount = transmit_.end()-send_next_;
    if (amount > maximum) amount = maximum;
    if (amount > limit-flight) amount = limit-flight;
    const auto *data = transmit_.span(send_next_,amount);
    int result = segment(send_next_,24,data,amount);
    if (result < 0) break;
    auto now = detail::milliseconds();
    if (!flight) retransmission_.start(now);
    send_next_ += amount; retransmission_.sent(send_next_,now);
  }
  finish();
}
void TcpSocket::finish()
{
  if (!write_shutdown_ || fin_sent_ || send_next_ != transmit_.end() || syn_pending_ ||
      (state_ != TcpState::Established && state_ != TcpState::CloseWait)) return;
  unsigned flight = send_next_-send_unacknowledged_;
  if (flight >= peer_window_ || flight >= congestion_.window()) return;
  if (segment(send_next_,17) < 0) return;
  fin_sequence_ = send_next_++; fin_sent_ = true;
  state_ = state_ == TcpState::Established ? TcpState::FinWait1 : TcpState::LastAck;
  auto now = detail::milliseconds(); if (!flight) retransmission_.start(now);
  retransmission_.sent(send_next_,now);
}
void TcpSocket::retransmit()
{
  retransmission_.uncertain();
  if (syn_pending_)
  { segment(syn_sequence_,state_ == TcpState::SynSent ? 2 : 18); return; }
  if (wire::before(send_unacknowledged_,transmit_.end()))
  {
    unsigned amount = transmit_.end()-send_unacknowledged_, outstanding = send_next_-send_unacknowledged_;
    if (amount > outstanding) amount = outstanding;
    unsigned maximum = mss(); if (!maximum || !amount) return;
    if (amount > maximum) amount = maximum;
    auto *data = transmit_.span(send_unacknowledged_,amount);
    segment(send_unacknowledged_,24,data,amount);
  }
  else if (fin_sent_ && !fin_acked_)
    segment(fin_sequence_,17);
}
bool TcpSocket::update_window(unsigned sequence, unsigned acknowledgement, unsigned value)
{
  if (wire::before(acknowledgement,send_unacknowledged_) || wire::before(send_next_,acknowledgement) ||
      wire::before(sequence,window_sequence_) || (sequence == window_sequence_ && wire::before(acknowledgement,window_ack_))) return false;
  bool changed = value != peer_window_;
  if (!peer_window_ && value) { persist_deadline_ = 0; persist_interval_ = 1000; retransmission_.start(detail::milliseconds()); }
  peer_window_ = value; window_sequence_ = sequence; window_ack_ = acknowledgement;
  return changed;
}
void TcpSocket::acknowledge(unsigned acknowledgement, bool duplicate)
{
  if (wire::before(acknowledgement,send_unacknowledged_) || wire::before(send_next_,acknowledgement)) return;
  auto now = detail::milliseconds();
  if (acknowledgement == send_unacknowledged_)
  {
    if (duplicate && !syn_pending_ && congestion_.duplicate(acknowledgement,send_next_-send_unacknowledged_,send_next_))
    { retransmit(); retransmission_.start(now); }
    else if (!duplicate) congestion_.reset_duplicates();
    return;
  }
  send_unacknowledged_ = acknowledgement;
  bool handshake = syn_pending_;
  if (syn_pending_ && wire::before(syn_sequence_,acknowledgement)) syn_pending_ = false;
  unsigned bytes = transmit_.discard(acknowledgement);
  retries_ = 0;
  retransmission_.acknowledge(acknowledgement,now,send_unacknowledged_ != send_next_);
  if (!handshake && congestion_.acknowledged(bytes,acknowledgement)) retransmit();
  if (fin_sent_ && !fin_acked_ && wire::before(fin_sequence_,acknowledgement))
  {
    fin_acked_ = true;
    if (state_ == TcpState::FinWait1) { state_ = TcpState::FinWait2; idle_deadline_ = now+30000; }
    else if (state_ == TcpState::Closing) enter_time_wait();
    else if (state_ == TcpState::LastAck) state_ = TcpState::Closed;
  }
  notify();
}
void TcpSocket::persist(unsigned long now)
{
  if (peer_window_ || syn_pending_ || (!transmit_.size() && (!write_shutdown_ || fin_acked_)))
  { persist_deadline_ = 0; persist_interval_ = 1000; return; }
  if (persist_deadline_ && now < persist_deadline_) return;
  unsigned sequence = send_unacknowledged_, amount = 1;
  if (wire::before(sequence,transmit_.end()))
  {
    auto *data = transmit_.span(sequence,amount);
    if (segment(sequence,16,data,1) >= 0 && sequence == send_next_)
      ++send_next_;
    retransmission_.uncertain();
  }
  else segment(send_next_-1,16);
  persist_deadline_ = now+persist_interval_;
  if (persist_interval_ < 60000) { persist_interval_ *= 2; if (persist_interval_ > 60000) persist_interval_ = 60000; }
}
}
