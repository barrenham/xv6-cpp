module;
#include "kernel/net/errno.h"
module xv.net;
import :ethernet_interface;
namespace xv::net
{
void EthernetInterface::initialize(xv::drivers::EthernetDevice &device)
{
  link_.initialize(device); arp_.initialize();
  link_.attach(0x806, arp_); link_.attach(0x800, *this);
}
int EthernetInterface::transmit(Packet &packet, unsigned next_hop)
{
  if (master() || !up() || !carrier() || !address_count()) return -ENETUNREACH;
  if (packet.size() > mtu()) return -EMSGSIZE;
  if (next_hop == 0xffffffff)
  { constexpr unsigned char broadcast[6] = {255,255,255,255,255,255}; return link_.send_ip(packet, broadcast); }
  return arp_.send(packet, next_hop);
}
void EthernetInterface::receive(Packet &packet)
{ if (packet.remove_prefix(14)) deliver(packet); }
}
