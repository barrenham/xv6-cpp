module xv.net;
import :icmp;
namespace xv::net
{
void IcmpProtocol::receive(Packet &packet, const IpDatagram &d)
{
  auto *p = packet.data()+d.offset;
  if (d.length < 8 || wire::sum(p, d.length) != 0xffff || p[0] != 8 || p[1] ||
      !ip_.local(d.destination)) return;
  auto reply = ip_.allocate(d.length); if (!reply) return;
  auto *out = reply.data()+20; wire::copy(out, p, d.length);
  out[0] = 0; wire::write16(out+2, 0); wire::write16(out+2, ~wire::sum(out, d.length));
  ip_.send(reply, d.source, 1, d.destination);
}
void IcmpProtocol::unreachable(Packet &packet, const IpDatagram &d)
{
  if (!ip_.local(d.destination)) return;
  unsigned quoted = d.offset + (d.length < 8 ? d.length : 8);
  auto reply = ip_.allocate(8+quoted); if (!reply) return;
  auto *p = reply.data()+20; p[0] = 3; p[1] = 3;
  wire::copy(p+8, packet.data(), quoted); wire::write16(p+2, ~wire::sum(p, 8+quoted));
  ip_.send(reply, d.source, 1, d.destination);
}
}
