module;
#include "kernel/net/errno.h"
module xv.net;
import :tunnel_protocol_flow;
import :tunnel_flows;
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
  TransmitFlow::Context ctx{*this, packet};
  (void)TransmitFlow::Graph::run<0>(ctx);
  return ctx.result;
}

bool IpInIpInterface::receive_outer(Packet &packet, const IpDatagram &d)
{
  ReceiveFlow::Context ctx{*this, packet, d};
  (void)ReceiveFlow::Graph::run<0>(ctx);
  return ctx.result;
}

bool IpInIpProtocol::add(IpInIpInterface &interface)
{
  DispatchFlow::AddContext ctx{*this, interface};
  (void)DispatchFlow::AddGraph::run<0>(ctx);
  return ctx.result;
}

void IpInIpProtocol::receive(Packet &packet, const IpDatagram &datagram)
{
  DispatchFlow::ReceiveContext ctx{*this, packet, datagram};
  (void)DispatchFlow::ReceiveGraph::run<0>(ctx);
}
} // namespace xv::net
