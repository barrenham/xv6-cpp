module;
#include "kernel/net/errno.h"
module xv.net;
import :tcp_option_flow;
import :tcp_incoming_flow;
import :tcp_socket;

namespace xv::net
{
unsigned tcp_peer_mss(const unsigned char *p, unsigned length)
{
  TcpMssFlow::Context ctx{p, length};
  (void)TcpMssFlow::Graph::run<0>(ctx);
  return ctx.result;
}

unsigned TcpSocket::window() const
{
  return receive_.window();
}

void TcpSocket::incoming(Endpoint, Endpoint, const unsigned char *p, unsigned length)
{
  IncomingFlow::Context ctx{*this, p, length};
  (void)IncomingFlow::Graph::run<0>(ctx);
}
} // namespace xv::net
