module;
#include "kernel/net/errno.h"
module xv.net;
import :tcp;
import :tcp_socket;
namespace xv::net
{
int TcpProtocol::transmit(Endpoint local, Endpoint peer, unsigned sequence, unsigned acknowledgement,
                          unsigned flags, unsigned window, const void *data, unsigned length)
{
  unsigned mtu = registry_.ip().mtu_for(peer.address, local.address);
  if (mtu < 68) return -ENETUNREACH;
  unsigned header = (flags&2) ? 24 : 20;
  auto packet = registry_.ip().allocate(header+length); if (!packet) return -ENOBUFS;
  auto *p = packet.data()+20;
  wire::write16(p, local.port); wire::write16(p+2, peer.port);
  wire::write32(p+4, sequence); wire::write32(p+8, acknowledgement);
  p[12] = (header/4)<<4; p[13] = flags; wire::write16(p+14, window);
  if (flags&2) { p[20] = 2; p[21] = 4; wire::write16(p+22, mtu-40); }
  if (length) wire::copy(p+header, data, length);
  unsigned char pseudo[8]; wire::write32(pseudo, local.address); wire::write32(pseudo+4, peer.address);
  wire::write16(p+16, ~wire::sum(p, header+length, wire::sum(pseudo, 8, 6+header+length)));
  return registry_.ip().send(packet, peer.address, 6, local.address);
}
void TcpProtocol::receive(Packet &packet, const IpDatagram &d)
{
  if (d.length < 20) return;
  auto *p = packet.data()+d.offset;
  unsigned header = (p[12]>>4)*4;
  if (header < 20 || header > d.length || (p[12]&15) ||
      wire::sum(p, d.length, wire::sum(packet.data()+12, 8, 6+d.length)) != 0xffff ||
      !wire::read16(p) || !wire::read16(p+2)) return;
  Endpoint peer{d.source, static_cast<unsigned short>(wire::read16(p))};
  Endpoint local{d.destination, static_cast<unsigned short>(wire::read16(p+2))};
  xv::sync::ScopedLock guard(registry_.lock());
  TcpSocket *listener = nullptr;
  for (unsigned i = 0; i < 64; ++i)
  {
    auto *socket = registry_.at(i);
    if (!socket || socket->protocol() != 6) continue;
    auto *tcp = static_cast<TcpSocket *>(socket);
    if (tcp->matches(local, peer)) { tcp->incoming(peer, local, p, d.length); return; }
    if (tcp->listening(local)) listener = tcp;
  }
  if (listener && (p[13]&0x17) == 2)
  {
    unsigned queued = 0;
    for (unsigned i = 0; i < 64; ++i)
      if (auto *s = registry_.at(i); s && s->protocol() == 6 &&
          static_cast<TcpSocket *>(s)->listener_ == listener) ++queued;
    if (queued >= listener->backlog_) return;
    if (auto *child = registry_.create_child()) child->passive(*listener, local, peer, p, d.length);
    return;
  }
  if (p[13]&4) return;
  unsigned length = d.length-header + ((p[13]&2) ? 1 : 0) + ((p[13]&1) ? 1 : 0);
  transmit(local, peer, (p[13]&16) ? wire::read32(p+8) : 0,
           (p[13]&16) ? 0 : wire::read32(p+4)+length,
           (p[13]&16) ? 4 : 20, 0, nullptr, 0);
}
}
