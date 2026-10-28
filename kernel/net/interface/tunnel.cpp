module;
#include "kernel/net/errno.h"
module xv.net;
import :tunnel;

namespace xv::net
{
bool IpInIpInterface::initialize(Ipv4Layer &ip, NetworkInterface &underlay,
                                 unsigned local, unsigned remote)
{
  if (ip_ || underlay.is_tunnel() || &underlay == this || !local || !remote ||
      local == remote || !underlay.owns(local) || underlay.mtu() < 88 ||
      mtu() > underlay.mtu() - 20)
  {
    return false;
  }
  ip_ = &ip;
  underlay_ = &underlay;
  outer_local_ = local;
  outer_remote_ = remote;
  return true;
}

int IpInIpInterface::transmit(Packet &packet, unsigned)
{
  if (!up() || !carrier() || !ip_)
  {
    return -ENETUNREACH;
  }
  if (packet.size() > mtu() || packet.size() < 20)
  {
    return -EMSGSIZE;
  }
  if (!packet.prepend(20))
  {
    return -EMSGSIZE;
  }
  int result = ip_->send(packet, outer_remote_, 4, outer_local_, underlay_);
  if (result < 0)
  {
    packet.remove_prefix(20);
  }
  return result;
}

bool IpInIpInterface::receive_outer(Packet &packet, const IpDatagram &d)
{
  if (!up() || !ip_ || d.interface != underlay_ || d.source != outer_remote_ ||
      d.destination != outer_local_)
  {
    return false;
  }
  if (d.length < 20 || !packet.enter_tunnel())
  {
    return true;
  }
  packet.resize(d.offset + d.length);
  if (packet.remove_prefix(d.offset))
  {
    deliver(packet);
  }
  return true;
}

bool IpInIpProtocol::add(IpInIpInterface &interface)
{
  for (auto *tunnel : tunnels_)
  {
    if (tunnel == &interface)
    {
      return false;
    }
  }
  for (auto &tunnel : tunnels_)
  {
    if (!tunnel)
    {
      tunnel = &interface;
      return true;
    }
  }
  return false;
}

void IpInIpProtocol::receive(Packet &packet, const IpDatagram &datagram)
{
  for (auto *tunnel : tunnels_)
  {
    if (tunnel && tunnel->receive_outer(packet, datagram))
    {
      return;
    }
  }
}
} // namespace xv::net
