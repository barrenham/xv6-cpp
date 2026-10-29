module;
#include "kernel/net/control.h"
#include "kernel/net/errno.h"
module xv.net;
import :bridge_manager_flows;
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
  {
    if (auto *mac = interfaces_.at(i)->mac())
    {
      bool valid = false;
      for (auto byte : mac->bytes)
      {
        valid |= byte != 0;
      }
      if (valid && !(mac->bytes[0] & 1))
      {
        seed = *mac;
        break;
      }
    }
  }
  for (unsigned i = 0; i < 2; ++i)
  {
    auto &bridge = bridges_[i];
    bridge.bridge_mac_ = seed;
    bridge.bridge_mac_.bytes[0] = (seed.bytes[0] | 2) & 0xfe;
    bridge.bridge_mac_.bytes[3] ^= 0x80 + i;
    bridge.initialize(ip_);
  }
}

BridgeInterface *BridgeManager::find(unsigned id)
{
  for (auto &bridge : bridges_)
  {
    if (bridge.active_ && bridge.id() == id)
    {
      return &bridge;
    }
  }
  return nullptr;
}

bool BridgeManager::unused(NetworkInterface &interface) const
{
  if (interface.address_count() || interface.dhcp_active())
  {
    return false;
  }
  return !routes_.references(interface);
}

int BridgeManager::membership(BridgeInterface &bridge, NetworkInterface &interface,
                              bool add)
{
  MembershipFlow::Context ctx{*this, bridge, interface, add};
  (void)MembershipFlow::Graph::run<0>(ctx);
  return ctx.result;
}

int BridgeManager::execute(unsigned operation, netctl_request &request)
{
  ExecuteFlow::Context ctx{*this, operation, request};
  (void)ExecuteFlow::Graph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::net
