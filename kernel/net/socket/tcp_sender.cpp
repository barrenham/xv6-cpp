module;
#include "kernel/net/errno.h"
module xv.net;
import :tcp_lifecycle_flow;
import :tcp_reliability_flow;
import :tcp_sender_flow;
import :tcp_socket;

namespace xv::net
{
unsigned TcpSocket::mss() const
{
  unsigned mtu = registry_.ip().mtu_for(peer_.address, local_.address);
  if (mtu < 68)
  {
    return 0;
  }
  unsigned result = mtu - 40;
  if (result > peer_mss_)
  {
    result = peer_mss_;
  }
  return result > 1460 ? 1460 : result;
}

int TcpSocket::segment(unsigned sequence, unsigned flags, const void *data,
                       unsigned length)
{
  int result = registry_.tcp().transmit(local_, peer_, sequence, receive_.next(), flags,
                                        window(), data, length);
  if (result >= 0 && (flags & 16))
  {
    advertised_right_ = receive_.next() + window();
  }
  return result;
}

void TcpSocket::emit(unsigned flags)
{
  segment(send_next_, flags);
}

void TcpSocket::synchronize(unsigned sequence)
{
  syn_sequence_ = send_unacknowledged_ = sequence;
  send_next_ = sequence + 1;
  syn_pending_ = true;
  fin_sent_ = fin_acked_ = false;
  read_shutdown_ = write_shutdown_ = false;
  retries_ = 0;
  transmit_.reset(send_next_);
  retransmission_.reset();
  auto now = detail::milliseconds();
  last_activity_ = now;
  retransmission_.start(now);
  retransmission_.sent(send_next_, now);
  segment(sequence, state_ == TcpState::SynSent ? 2 : 18);
}

void TcpSocket::pump()
{
  PumpFlow::Context ctx{*this};
  (void)PumpFlow::Graph::run<0>(ctx);
}

void TcpSocket::finish()
{
  FinishFlow::Context ctx{*this};
  (void)FinishFlow::Graph::run<0>(ctx);
}

void TcpSocket::retransmit()
{
  RetransmitFlow::Context ctx{*this};
  (void)RetransmitFlow::Graph::run<0>(ctx);
}

bool TcpSocket::update_window(unsigned sequence, unsigned acknowledgement,
                              unsigned value)
{
  WindowFlow::Context ctx{*this, sequence, acknowledgement, value};
  (void)WindowFlow::Graph::run<0>(ctx);
  return ctx.changed;
}

void TcpSocket::acknowledge(unsigned acknowledgement, bool duplicate)
{
  AcknowledgeFlow::Context ctx{*this, acknowledgement, duplicate};
  (void)AcknowledgeFlow::Graph::run<0>(ctx);
}

void TcpSocket::persist(unsigned long now)
{
  PersistFlow::Context ctx{*this, now};
  (void)PersistFlow::Graph::run<0>(ctx);
}
} // namespace xv::net
