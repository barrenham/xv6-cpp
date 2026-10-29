module;
#include "kernel/net/errno.h"
module xv.net;
import :ethernet_interface_flows;
import :ethernet_interface;

namespace xv::net
{
void EthernetInterface::initialize(xv::drivers::EthernetDevice &device)
{
  link_.initialize(device);
  arp_.initialize();
  link_.attach(0x806, arp_);
  link_.attach(0x800, *this);
}

int EthernetInterface::transmit(Packet &packet, unsigned next_hop)
{
  TransmitFlow::Context ctx{*this, packet, next_hop};
  (void)TransmitFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void EthernetInterface::receive(Packet &packet)
{
  if (packet.remove_prefix(14))
  {
    deliver(packet);
  }
}
} // namespace xv::net

namespace xv::net
{
int EthernetInterface::transmit_bootstrap(Packet &packet)
{
  TransmitFlow::Context ctx{*this, packet, 0xffffffff};
  (void)TransmitFlow::BootstrapGraph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::net
