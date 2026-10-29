module;
#include "kernel/net/errno.h"
module xv.net;
import :udp;
import :udp_flows;

namespace xv::net
{
void UdpProtocol::attach(DatagramSink &sink)
{
  sink_ = &sink;
}

void UdpProtocol::receive(Packet &packet, const IpDatagram &d)
{
  ReceiveFlow::Context ctx{*this, packet, d};
  (void)ReceiveFlow::Graph::run<0>(ctx);
}

int UdpProtocol::send(Endpoint source, Endpoint destination, const void *data,
                      unsigned length)
{
  SendFlow::Context ctx{*this, source, destination, data, length};
  (void)SendFlow::Graph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::net
