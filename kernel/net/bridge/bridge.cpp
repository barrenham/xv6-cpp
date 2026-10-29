module;
#include "kernel/net/errno.h"
module xv.net;
import :bridge_forward_flow;
import :bridge;
import :wire;
import :platform;

namespace xv::net
{
void BridgeInterface::initialize(IpReceiver &ip)
{
  EthernetInterface::initialize(*this);
  attach(ip);
}

void BridgeInterface::mac_address(unsigned char *out) const
{
  for (unsigned i = 0; i < 6; ++i)
  {
    out[i] = bridge_mac_.bytes[i];
  }
}

bool BridgeInterface::carrier() const
{
  ConfigurationGuard guard;
  if (!active_)
  {
    return false;
  }
  for (const auto &member : members_)
  {
    if (member.interface && member.interface->up() && member.interface->carrier())
    {
      return true;
    }
  }
  return false;
}

void BridgeInterface::tick(unsigned long now)
{
  {
    ConfigurationGuard guard;
    for (auto &entry : fdb_)
    {
      if (entry.member.port && entry.expires <= now)
      {
        entry = {};
      }
    }
  }
  EthernetInterface::tick(now);
}

void BridgeInterface::ingress(Layer2Port &port, unsigned epoch, Packet &frame)
{
  forward(&port, epoch, frame);
}

bool BridgeInterface::transmit(void *page, unsigned length)
{
  Packet frame(page, length);
  if (forward(nullptr, 0, frame))
  {
    return true;
  }
  frame.release();
  return false;
}

bool BridgeInterface::forward(Layer2Port *ingress, unsigned epoch, Packet &frame)
{
  ForwardFlow::Context ctx{*this, ingress, epoch, frame};
  (void)ForwardFlow::Graph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::net
