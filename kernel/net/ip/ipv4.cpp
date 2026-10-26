module;
#include "kernel/net/errno.h"
module xv.net;
import :ipv4;
import :platform;
namespace xv::net
{
void Ipv4Layer::initialize() { lock_.init("IPv4 queues"); }
void Ipv4Layer::attach(unsigned protocol, IpProtocol &handler)
{ for (auto &h : handlers_) if (!h.handler) { h = {protocol, &handler}; return; } }
Packet Ipv4Layer::allocate(unsigned length)
{
  if (length > 4062) return {};
  auto packet = Packet::allocate(20+length);
  if (packet) wire::zero(packet.data(), 20+length);
  return packet;
}
unsigned Ipv4Layer::source_for(unsigned destination, unsigned requested, const NetworkInterface *via) const
{
  RouteSelection selection;
  return routes_.lookup(destination, selection, requested, via) ? selection.source : 0;
}
unsigned Ipv4Layer::mtu_for(unsigned destination, unsigned requested) const
{
  RouteSelection selection;
  return routes_.lookup(destination, selection, requested) ? selection.interface->mtu() : 0;
}
int Ipv4Layer::send(Packet &packet, unsigned destination, unsigned protocol, unsigned source,
                    const NetworkInterface *via)
{
  RouteSelection selection;
  if (!routes_.lookup(destination, selection, source, via)) return -ENETUNREACH;
  if (!packet || packet.size() < 20 || packet.size() > selection.interface->mtu()) return -EMSGSIZE;
  auto *ip = packet.data();
  wire::zero(ip, 20); ip[0] = 0x45; ip[8] = 64; ip[9] = protocol;
  wire::write16(ip+2, packet.size());
  wire::write16(ip+4, __atomic_fetch_add(&identification_, 1U, __ATOMIC_RELAXED));
  wire::write16(ip+6, 0x4000);
  wire::write32(ip+12, selection.source); wire::write32(ip+16, destination);
  wire::write16(ip+10, ~wire::sum(ip, 20));
  return selection.interface->transmit(packet, selection.next_hop);
}
void Ipv4Layer::receive(NetworkInterface &interface, Packet &packet)
{
  if (!interface.up() || interfaces_.find(interface.id()) != &interface || packet.size() < 20) return;
  auto *ip = packet.data();
  unsigned header = (ip[0]&15)*4, total = wire::read16(ip+2);
  unsigned source = wire::read32(ip+12), destination = wire::read32(ip+16);
  if ((ip[0] >> 4) != 4 || header < 20 || total < header || total > packet.size() ||
      !ip[8] || (wire::read16(ip+6)&0x8000) || wire::sum(ip, header) != 0xffff ||
      !interface.accepts_destination(destination, interfaces_) || !source || (source >> 28) >= 14) return;
  if (wire::read16(ip+6)&0x3fff) { fragment(interface, packet, header, total); return; }
  IpDatagram datagram{source, destination, ip[9], header, total-header, &interface};
  for (auto &h : handlers_) if (h.handler && h.protocol == datagram.protocol)
  { h.handler->receive(packet, datagram); return; }
}
void Ipv4Layer::fragment(NetworkInterface &interface, Packet &packet, unsigned header, unsigned total)
{
  if (header != 20) return;
  auto *ip = packet.data();
  unsigned flags = wire::read16(ip+6), offset = (flags&0x1fff)*8, length = total-header;
  if (!length || offset+length > 4062 || ((flags&0x2000) && (length&7))) return;
  lock_.acquire();
  Assembly *slot = nullptr;
  for (auto &a : assemblies_)
    if (a.page && a.interface == &interface && a.source == wire::read32(ip+12) && a.destination == wire::read32(ip+16) &&
        a.id == wire::read16(ip+4) && a.protocol == ip[9]) { slot = &a; break; }
  if (!slot)
  {
    for (auto &a : assemblies_) if (!a.page) { slot = &a; break; }
    if (!slot) { lock_.release(); return; }
    slot->page = detail::allocate_page();
    if (!slot->page) { lock_.release(); return; }
    slot->interface = &interface;
    slot->source = wire::read32(ip+12); slot->destination = wire::read32(ip+16);
    slot->id = wire::read16(ip+4); slot->protocol = ip[9];
    slot->deadline = detail::milliseconds()+15000;
    wire::copy(slot->page, packet.data(), 20);
  }
  auto &a = *slot;
  bool invalid = (a.last && offset+length > a.total);
  for (unsigned i = offset; i < offset+length; ++i)
    if (a.present[i/8] & (1U<<(i%8))) invalid = true;
  if (!(flags&0x2000))
  {
    if ((a.last && a.total != offset+length) || a.received > offset+length) invalid = true;
    for (unsigned i = offset+length; i < 4062; ++i)
      if (a.present[i/8] & (1U<<(i%8))) invalid = true;
    a.total = offset+length; a.last = true;
  }
  if (invalid)
  { Packet discarded(a.page, 0); a = {}; lock_.release(); return; }
  wire::copy(static_cast<unsigned char *>(a.page)+20+offset, ip+header, length);
  for (unsigned i = offset; i < offset+length; ++i) a.present[i/8] |= 1U<<(i%8);
  a.received += length;
  if (a.last && a.received == a.total)
  {
    Packet complete(a.page, 20+a.total); a = {}; lock_.release();
    auto *h = complete.data(); wire::write16(h+2, complete.size());
    wire::write16(h+6, 0); wire::write16(h+10, 0); wire::write16(h+10, ~wire::sum(h, 20));
    receive(interface, complete); return;
  }
  lock_.release();
}
void Ipv4Layer::tick(unsigned long now)
{
  for (auto &a : assemblies_)
  {
    lock_.acquire();
    if (a.page && a.deadline <= now) { Packet expired(a.page, 0); a = {}; }
    lock_.release();
  }
}
}
