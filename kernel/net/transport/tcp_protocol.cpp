module;
#include "kernel/net/errno.h"
module xv.net;
import :tcp;
import :wire;

namespace xv::net
{
int TcpProtocol::transmit(Endpoint local, Endpoint peer, unsigned sequence,
                          unsigned acknowledgement, unsigned flags, unsigned window,
                          const void *data, unsigned length)
{
  unsigned mtu = ip_.mtu_for(peer.address, local.address);
  if (mtu < 68)
  {
    return -ENETUNREACH;
  }
  unsigned header_length = (flags & 2) ? 24 : 20;
  auto packet = ip_.allocate(header_length + length);
  if (!packet)
  {
    return -ENOBUFS;
  }
  auto *segment = packet.data() + 20;
  wire::write16(segment, local.port);
  wire::write16(segment + 2, peer.port);
  wire::write32(segment + 4, sequence);
  wire::write32(segment + 8, acknowledgement);
  segment[12] = (header_length / 4) << 4;
  segment[13] = flags;
  wire::write16(segment + 14, window);
  if (flags & 2)
  {
    segment[20] = 2;
    segment[21] = 4;
    wire::write16(segment + 22, mtu - 40);
  }
  if (length)
  {
    wire::copy(segment + header_length, data, length);
  }
  unsigned char pseudoheader[8];
  wire::write32(pseudoheader, local.address);
  wire::write32(pseudoheader + 4, peer.address);
  unsigned segment_length = header_length + length;
  wire::write16(segment + 16,
                ~wire::sum(segment, segment_length,
                           wire::sum(pseudoheader, 8, 6 + segment_length)));
  return ip_.send(packet, peer.address, 6, local.address);
}

void TcpProtocol::receive(Packet &packet, const IpDatagram &datagram)
{
  if (datagram.length < 20)
  {
    return;
  }
  auto *segment = packet.data() + datagram.offset;
  unsigned header_length = (segment[12] >> 4) * 4;
  if (header_length < 20 || header_length > datagram.length || (segment[12] & 15) ||
      wire::sum(segment, datagram.length,
                wire::sum(packet.data() + 12, 8, 6 + datagram.length)) != 0xffff ||
      !wire::read16(segment) || !wire::read16(segment + 2))
  {
    return;
  }
  Endpoint peer{datagram.source, static_cast<unsigned short>(wire::read16(segment))};
  Endpoint local{datagram.destination,
                 static_cast<unsigned short>(wire::read16(segment + 2))};
  if (sink_ && sink_->deliver_segment({peer, local, segment, datagram.length}))
  {
    return;
  }

  unsigned flags = segment[13];
  if (flags & 4)
  {
    return;
  }
  unsigned sequence_length =
      datagram.length - header_length + ((flags & 2) ? 1 : 0) + ((flags & 1) ? 1 : 0);
  unsigned reset_sequence = (flags & 16) ? wire::read32(segment + 8) : 0;
  unsigned reset_acknowledgement =
      (flags & 16) ? 0 : wire::read32(segment + 4) + sequence_length;
  transmit(local, peer, reset_sequence, reset_acknowledgement, (flags & 16) ? 4 : 20, 0,
           nullptr, 0);
}
} // namespace xv::net
