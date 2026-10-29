module;
#include "kernel/net/errno.h"
module xv.net;
import :ethernet_encapsulation_flow;
import :ethernet_flows;
import :ethernet;
import :wire;

namespace xv::net
{
void EthernetLayer::initialize(xv::drivers::EthernetDevice &device)
{
  device_ = &device;
  device.mac_address(mac_.bytes);
}

int EthernetLayer::send_ip(Packet &packet, const unsigned char *destination)
{
  EncapsulationFlow::Context ctx{*this, packet, destination};
  (void)EncapsulationFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void EthernetLayer::attach(unsigned type, EthernetProtocol &protocol)
{
  for (auto &b : bindings_)
  {
    if (!b.protocol)
    {
      b = {type, &protocol};
      return;
    }
  }
}

int EthernetLayer::send(Packet &packet, const unsigned char *destination, unsigned type)
{
  if (!packet || packet.size() < 14 || packet.size() > mtu_ + 14)
  {
    return -EMSGSIZE;
  }
  auto *p = packet.data();
  wire::copy(p, destination, 6);
  wire::copy(p + 6, mac_.bytes, 6);
  wire::write16(p + 12, type);
  return transmit_frame(packet);
}

int EthernetLayer::transmit_frame(Packet &packet)
{
  SendFlow::Context ctx{*this, packet};
  (void)SendFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void EthernetLayer::receive_frame(void *page, unsigned length)
{
  Packet packet(page, length);
  ReceiveFlow::Context ctx{*this, packet};
  (void)ReceiveFlow::Graph::run<0>(ctx);
}
} // namespace xv::net
