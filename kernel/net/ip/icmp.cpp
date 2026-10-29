module xv.net;
import :icmp;
import :icmp_reply_flow;

namespace xv::net
{
void IcmpProtocol::receive(Packet &packet, const IpDatagram &d)
{
  ReplyFlow::Context ctx{*this, packet, d};
  (void)ReplyFlow::EchoGraph::run<0>(ctx);
}

void IcmpProtocol::unreachable(Packet &packet, const IpDatagram &d)
{
  ReplyFlow::Context ctx{*this, packet, d};
  (void)ReplyFlow::UnreachableGraph::run<0>(ctx);
}
} // namespace xv::net
