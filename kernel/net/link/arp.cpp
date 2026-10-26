module;
#include "kernel/net/errno.h"
module xv.net;
import :arp;
import :wire;
import :platform;
namespace xv::net
{
void ArpCache::initialize() { lock_.init("ARP neighbors"); }
void ArpCache::request(unsigned address)
{
  if (!interface_.source_for(address)) return;
  auto packet = Packet::allocate(42);
  if (!packet) return;
  auto *p = packet.data(); wire::zero(p, 42);
  wire::write16(p+14, 1); wire::write16(p+16, 0x800); p[18] = 6; p[19] = 4;
  wire::write16(p+20, 1); wire::copy(p+22, link_.mac().bytes, 6);
  wire::write32(p+28, interface_.source_for(address)); wire::write32(p+38, address);
  constexpr unsigned char broadcast[6] = {255,255,255,255,255,255};
  link_.send(packet, broadcast, 0x806);
}
int ArpCache::send(Packet &packet, unsigned next_hop)
{
  unsigned char mac[6];
  lock_.acquire();
  auto now = detail::milliseconds();
  for (auto &e : entries_)
    if (e.address == next_hop && e.expires > now)
    {
      wire::copy(mac, e.mac, 6); lock_.release(); return link_.send_ip(packet, mac);
    }
  for (auto &p : pending_) if (!p.page)
  {
    p = {packet.release(), packet.size(), next_hop, 1, now+1000};
    lock_.release(); request(next_hop); return 0;
  }
  lock_.release(); return -ENOBUFS;
}
void ArpCache::receive(Packet &packet)
{
  if (packet.size() < 42) return;
  auto *p = packet.data(); unsigned operation = wire::read16(p+20);
  if (wire::read16(p+14) != 1 || wire::read16(p+16) != 0x800 || p[18] != 6 || p[19] != 4 ||
      (operation != 1 && operation != 2) || !wire::equal(p+6, p+22, 6) || (p[22]&1) ||
      !interface_.owns(wire::read32(p+38))) return;
  unsigned source = wire::read32(p+28);
  if (!source || interface_.owns(source) || source == 0xffffffff) return;
  lock_.acquire();
  Neighbor *entry = &entries_[0];
  for (auto &e : entries_)
    if (e.address == source) { entry = &e; break; }
    else if (e.expires < entry->expires) entry = &e;
  *entry = {source, {}, detail::milliseconds()+60000}; wire::copy(entry->mac, p+22, 6);
  lock_.release();
  for (auto &queued : pending_)
  {
    lock_.acquire();
    if (!queued.page || queued.address != source) { lock_.release(); continue; }
    Packet ready(queued.page, queued.length); queued = {};
    lock_.release(); link_.send_ip(ready, p+22);
  }
  if (operation == 1)
  {
    auto reply = Packet::allocate(42); if (!reply) return;
    wire::copy(reply.data()+14, p+14, 28); wire::write16(reply.data()+20, 2);
    wire::copy(reply.data()+32, p+22, 6); wire::write32(reply.data()+38, source);
    wire::copy(reply.data()+22, link_.mac().bytes, 6); wire::write32(reply.data()+28, wire::read32(p+38));
    link_.send(reply, p+22, 0x806);
  }
}
void ArpCache::tick(unsigned long now)
{
  for (auto &p : pending_)
  {
    lock_.acquire();
    if (!p.page || p.deadline > now) { lock_.release(); continue; }
    if (p.retries++ >= 3)
    { Packet expired(p.page, p.length); p = {}; lock_.release(); continue; }
    auto address = p.address; p.deadline = now+1000;
    lock_.release(); request(address);
  }
}
}
