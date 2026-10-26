module;
#include "kernel/net/control.h"
#include "kernel/net/errno.h"
module xv.net;
import :bridge;
import :wire;
import :platform;
namespace xv::net
{
void BridgeManager::initialize()
{
  admin_lock_.init("bridge administration");
  MacAddress seed{{2, 0x58, 0x56, 0, 0, 0}};
  for (unsigned i = 0; i < interfaces_.count(); ++i)
    if (auto *mac = interfaces_.at(i)->mac())
    {
      bool valid = false; for (auto byte : mac->bytes) valid |= byte != 0;
      if (valid && !(mac->bytes[0]&1)) { seed = *mac; break; }
    }
  for (unsigned i = 0; i < 2; ++i)
  {
    auto &bridge = bridges_[i]; bridge.bridge_mac_ = seed;
    bridge.bridge_mac_.bytes[0] = (seed.bytes[0]|2)&0xfe;
    bridge.bridge_mac_.bytes[3] ^= 0x80+i;
    bridge.initialize(ip_);
  }
}
BridgeInterface *BridgeManager::find(unsigned id)
{
  for (auto &bridge : bridges_) if (bridge.active_ && bridge.id() == id) return &bridge;
  return nullptr;
}
bool BridgeManager::unused(NetworkInterface &interface) const
{
  if (interface.address_count()) return false;
  for (unsigned i = 0; i < routes_.count_; ++i)
    if (routes_.routes_[i].interface == &interface || routes_.routes_[i].owner == interface.id()) return false;
  return true;
}
int BridgeManager::membership(BridgeInterface &bridge, NetworkInterface &interface, bool add)
{
  auto *port = interface.layer2();
  if (!port || interface.is_bridge() || port->frame_mtu() < bridge.mtu()) return -EOPNOTSUPP;
  unsigned slot = 4;
  {
    ConfigurationGuard guard;
    if (add)
    {
      if (port->master_ || !unused(interface)) return -EBUSY;
      for (unsigned i = 0; i < 4; ++i) if (!bridge.members_[i].port) { slot = i; break; }
      if (slot == 4) return -ENOBUFS;
    }
    else
    {
      for (unsigned i = 0; i < 4; ++i) if (bridge.members_[i].port == port) slot = i;
      if (slot == 4 || port->master_ != &bridge) return -ENOENT;
      port->master_ = nullptr; ++port->epoch_; bridge.members_[slot] = {};
      for (auto &entry : bridge.fdb_) if (entry.member.port == port) entry = {};
      ++interface.generation_;
    }
  }
  if (!add)
  {
    port->promiscuous(false); interface.invalidate(); return 0;
  }
  if (!port->promiscuous(true)) return -EOPNOTSUPP;
  bool valid;
  {
    ConfigurationGuard guard;
    valid = bridge.active_ && !port->master_ && unused(interface) && !bridge.members_[slot].port;
    if (valid)
    {
      port->master_ = &bridge; ++port->epoch_; ++interface.generation_;
      bridge.members_[slot] = {&interface, port, port->epoch_};
    }
  }
  if (!valid) { port->promiscuous(false); return -EBUSY; }
  interface.invalidate(); return 0;
}
int BridgeManager::execute(unsigned operation, netctl_request &request)
{
  if (request.version != NETCTL_VERSION || request.size != sizeof(request)) return -EINVAL;
  xv::sync::ScopedLock administrative(admin_lock_);
  if (operation == NETCTL_BRIDGE_CREATE)
  {
    ConfigurationGuard guard;
    BridgeInterface *target = nullptr;
    for (auto &bridge : bridges_)
    {
      unsigned i = 0;
      while (i < sizeof(request.name) && request.name[i] && request.name[i] == bridge.name()[i]) ++i;
      if (i < sizeof(request.name) && !request.name[i] && !bridge.name()[i]) target = &bridge;
    }
    if (!target) return -EINVAL;
    if (target->active_ || interfaces_.find(target->id())) return -EEXIST;
    for (unsigned i = 0; i < interfaces_.count_; ++i)
    {
      const char *name = interfaces_.interfaces_[i]->name(); const char *other = target->name();
      while (*name && *name == *other) { ++name; ++other; }
      if (*name == *other) return -EEXIST;
    }
    if (interfaces_.count_ == 8) return -ENOBUFS;
    target->sealed_ = true; target->active_ = true; ++target->generation_;
    interfaces_.interfaces_[interfaces_.count_++] = target; return 0;
  }
  BridgeInterface *bridge;
  {
    ConfigurationGuard guard;
    bridge = find(request.ifindex);
    if (!bridge) return -ENODEV;
    if (operation == NETCTL_BRIDGE_DELETE)
    {
      if (bridge->up() || !unused(*bridge)) return -EBUSY;
      for (const auto &member : bridge->members_) if (member.port) return -EBUSY;
      bridge->active_ = false; ++bridge->generation_;
      for (unsigned i = 0; i < interfaces_.count_; ++i)
        if (interfaces_.interfaces_[i] == bridge)
        {
          for (unsigned j = i+1; j < interfaces_.count_; ++j) interfaces_.interfaces_[j-1] = interfaces_.interfaces_[j];
          interfaces_.interfaces_[--interfaces_.count_] = nullptr; break;
        }
      for (auto &entry : bridge->fdb_) entry = {};
    }
    else if (operation == NETCTL_BRIDGE_PORT || operation == NETCTL_BRIDGE_FDB)
    {
      netctl_request out{}; out.version = NETCTL_VERSION; out.size = sizeof(out);
      out.ifindex = bridge->id(); out.index = request.index;
      unsigned index = request.index; bool found = false;
      if (operation == NETCTL_BRIDGE_PORT)
      {
        for (const auto &member : bridge->members_)
          if (member.port && !index--)
          {
            out.source = member.interface->id(); out.flags = member.interface->up() ? NET_LINK_UP : 0;
            found = true; break;
          }
      }
      else
      {
        auto now = detail::milliseconds();
        for (const auto &entry : bridge->fdb_)
          if (entry.member.port && entry.expires > now && !index--)
          {
            out.source = entry.member.interface->id(); wire::copy(out.mac, entry.mac.bytes, 6);
            out.expires_ms = entry.expires-now; found = true; break;
          }
      }
      if (!found) return 0;
      request = out; return 1;
    }
    else if (operation == NETCTL_BRIDGE_FLUSH_FDB)
    { for (auto &entry : bridge->fdb_) entry = {}; return 0; }
    else if (operation != NETCTL_BRIDGE_ADD_PORT && operation != NETCTL_BRIDGE_DEL_PORT) return -EINVAL;
  }
  if (operation == NETCTL_BRIDGE_DELETE) { bridge->invalidate(); return 0; }
  NetworkInterface *interface;
  { ConfigurationGuard guard; interface = interfaces_.find(request.source); }
  if (!interface) return -ENODEV;
  return membership(*bridge, *interface, operation == NETCTL_BRIDGE_ADD_PORT);
}
}
