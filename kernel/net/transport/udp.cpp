module;
#include "kernel/net/errno.h"
module xv.net;
import :udp;

namespace xv::net
{
void UdpProtocol::attach(DatagramSink &sink)
{
  sink_ = &sink;
}

void UdpProtocol::receive(Packet &packet, const IpDatagram &d)
{
  auto *p = packet.data() + d.offset;
  if (d.length < 8 || wire::read16(p + 4) != d.length || !wire::read16(p + 2))
  {
    return;
  }
  if (wire::read16(p + 6) &&
      wire::sum(p, d.length, wire::sum(packet.data() + 12, 8, 17 + d.length)) != 0xffff)
  {
    return;
  }
  Endpoint source{d.source, static_cast<unsigned short>(wire::read16(p))};
  Endpoint destination{d.destination, static_cast<unsigned short>(wire::read16(p + 2))};
  if (sink_ && sink_->deliver(packet, source, destination, d.offset + 8, d.length - 8))
  {
    return;
  }
  icmp_.unreachable(packet, d);
}

int UdpProtocol::send(Endpoint source, Endpoint destination, const void *data,
                      unsigned length)
{
  source.address = ip_.source_for(destination.address, source.address);
  if (!source.address)
  {
    return -ENETUNREACH;
  }
  if (length > 4042)
  {
    return -EMSGSIZE;
  }
  auto packet = ip_.allocate(8 + length);
  if (!packet)
  {
    return -ENOBUFS;
  }
  auto *p = packet.data() + 20;
  wire::write16(p, source.port);
  wire::write16(p + 2, destination.port);
  wire::write16(p + 4, 8 + length);
  wire::copy(p + 8, data, length);
  unsigned char pseudo[8];
  wire::write32(pseudo, source.address);
  wire::write32(pseudo + 4, destination.address);
  auto checksum = static_cast<unsigned short>(
      ~wire::sum(p, 8 + length, wire::sum(pseudo, 8, 17 + 8 + length)));
  wire::write16(p + 6, checksum ? checksum : 0xffff);
  return ip_.send(packet, destination.address, 17, source.address);
}
} // namespace xv::net
