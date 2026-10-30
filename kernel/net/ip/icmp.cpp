module;
#include "kernel/net/errno.h"
module xv.net;
import :icmp;
import :icmp_reply_flow;

namespace xv::net
{
void IcmpProtocol::receive(Packet &packet, const IpDatagram &d)
{
  auto *data = packet.data() + d.offset;
  if (sink_ && d.length >= 8 && data[0] == 0 && data[1] == 0 &&
      ip_.local(d.destination) && wire::sum(data, d.length) == 0xffff)
  {
    (void)sink_->deliver_echo(packet, d);
    return;
  }
  ReplyFlow::Context ctx{*this, packet, d};
  (void)ReplyFlow::EchoGraph::run<0>(ctx);
}

int IcmpProtocol::send_echo(const unsigned char *data, unsigned length,
                            Endpoint source, Endpoint destination)
{
  if (length < 8 || length > 1472) return -EMSGSIZE;
  if (data[0] != 8 || data[1] != 0) return -EINVAL;
  auto packet = ip_.allocate(length);
  if (!packet) return -ENOBUFS;
  auto *out = packet.data() + 20;
  wire::copy(out, data, length);
  wire::write16(out + 4, source.port);
  wire::write16(out + 2, 0);
  wire::write16(out + 2, ~wire::sum(out, length));
  return ip_.send(packet, destination.address, 1, source.address);
}

void IcmpProtocol::unreachable(Packet &packet, const IpDatagram &d)
{
  ReplyFlow::Context ctx{*this, packet, d};
  (void)ReplyFlow::UnreachableGraph::run<0>(ctx);
}
} // namespace xv::net
