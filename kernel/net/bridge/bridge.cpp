module;
#include "kernel/net/errno.h"
module xv.net;
import :bridge;
import :wire;
import :platform;
namespace xv::net
{
void BridgeInterface::initialize(IpReceiver &ip)
{ EthernetInterface::initialize(*this); attach(ip); }
void BridgeInterface::mac_address(unsigned char *out) const
{ for (unsigned i = 0; i < 6; ++i) out[i] = bridge_mac_.bytes[i]; }
bool BridgeInterface::carrier() const
{
  ConfigurationGuard guard;
  if (!active_) return false;
  for (const auto &member : members_)
    if (member.interface && member.interface->up() && member.interface->carrier()) return true;
  return false;
}
void BridgeInterface::tick(unsigned long now)
{
  {
    ConfigurationGuard guard;
    for (auto &entry : fdb_) if (entry.member.port && entry.expires <= now) entry = {};
  }
  EthernetInterface::tick(now);
}
void BridgeInterface::ingress(Layer2Port &port, unsigned epoch, Packet &frame)
{ forward(&port, epoch, frame); }
bool BridgeInterface::transmit(void *page, unsigned length)
{
  Packet frame(page, length);
  if (forward(nullptr, 0, frame)) return true;
  frame.release(); return false;
}
bool BridgeInterface::forward(Layer2Port *ingress, unsigned epoch, Packet &frame)
{
  if (!frame || frame.size() < 14 || frame.size() > mtu()+14) return false;
  auto *bytes = frame.data();
  unsigned type = wire::read16(bytes+12);
  if ((bytes[6]&1) || type == 0x8100 || type == 0x88a8 ||
      (bytes[0]==1 && bytes[1]==0x80 && bytes[2]==0xc2 && !bytes[3] && !bytes[4] && bytes[5]<=15)) return false;
  bool nonzero = false; for (unsigned i = 6; i < 12; ++i) nonzero |= bytes[i] != 0;
  if (!nonzero) return false;
  Member targets[4]{}; unsigned count = 0;
  bool local = false;
  {
    ConfigurationGuard guard;
    if (!active_ || !up()) return false;
    Member incoming{};
    for (const auto &member : members_) if (member.port == ingress) incoming = member;
    if (ingress && (!incoming.interface || ingress->master_ != this || ingress->epoch_ != incoming.epoch || incoming.epoch != epoch ||
                    !incoming.interface->up() || !incoming.interface->carrier())) return false;
    auto now = detail::milliseconds();
    if (ingress)
    {
      if (wire::equal(bytes+6, bridge_mac_.bytes, 6)) return false;
      Entry *slot = &fdb_[0];
      for (auto &entry : fdb_)
      {
        if (entry.member.port && wire::equal(entry.mac.bytes, bytes+6, 6)) { slot = &entry; break; }
        if (!entry.member.port || entry.expires < slot->expires) slot = &entry;
      }
      wire::copy(slot->mac.bytes, bytes+6, 6); slot->member = incoming; slot->expires = now+300000;
    }
    bool own = wire::equal(bytes, bridge_mac_.bytes, 6), group = bytes[0]&1;
    local = ingress && (own || group);
    Entry *destination = nullptr;
    if (!own && !group)
      for (auto &entry : fdb_)
        if (entry.member.port && entry.expires > now && wire::equal(entry.mac.bytes, bytes, 6))
        { destination = &entry; break; }
    if (!own)
      for (const auto &member : members_)
        if (member.port && member.port != ingress && member.interface->up() && member.interface->carrier() &&
            (!destination || (destination->member.port == member.port && destination->member.epoch == member.epoch)))
          targets[count++] = member;
  }
  bool sent = false;
  for (unsigned i = 0; i < count; ++i)
  {
    const auto &member = targets[i];
    auto copy = frame.clone(); if (!copy) continue;
    bool valid;
    {
      ConfigurationGuard guard;
      valid = active_ && up() && member.port->master_ == this && member.port->epoch_ == member.epoch &&
              member.interface->up() && member.interface->carrier();
    }
    if (valid && member.port->transmit_frame(copy) == 0) sent = true;
  }
  if (local)
  {
    receiver().receive_frame(frame.release(), frame.size());
    return true;
  }
  return sent;
}
}
