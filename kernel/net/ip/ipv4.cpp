module;
#include "kernel/net/errno.h"
module xv.net;
import :ipv4;
import :platform;
import :ipv4_receive_flow;
import :ipv4_send_flow;

namespace xv::net
{
void Ipv4Layer::initialize()
{
  reassembly_.initialize();
}

void Ipv4Layer::attach(unsigned protocol, IpProtocol &handler)
{
  for (auto &binding : handlers_)
  {
    if (binding.handler)
    {
      continue;
    }
    binding = {protocol, &handler};
    return;
  }
}

Packet Ipv4Layer::allocate(unsigned length)
{
  if (length > Ipv4Reassembly::MaxPayload)
  {
    return {};
  }
  auto packet = Packet::allocate(20 + length);
  if (packet)
  {
    wire::zero(packet.data(), 20 + length);
  }
  return packet;
}

unsigned Ipv4Layer::source_for(unsigned destination, unsigned requested,
                               const NetworkInterface *via) const
{
  RouteSelection selection;
  return routes_.lookup(destination, selection, requested, via) ? selection.source : 0;
}

unsigned Ipv4Layer::mtu_for(unsigned destination, unsigned requested) const
{
  RouteSelection selection;
  return routes_.lookup(destination, selection, requested) ? selection.interface->mtu()
                                                           : 0;
}

void Ipv4Layer::write_header(Packet &packet, unsigned source, unsigned destination,
                             unsigned protocol)
{
  auto *header = packet.data();
  wire::zero(header, 20);
  header[0] = 0x45;
  header[8] = 64;
  header[9] = protocol;
  wire::write16(header + 2, packet.size());
  wire::write16(header + 4, __atomic_fetch_add(&identification_, 1U, __ATOMIC_RELAXED));
  wire::write16(header + 6, 0x4000);
  wire::write32(header + 12, source);
  wire::write32(header + 16, destination);
  wire::write16(header + 10, ~wire::sum(header, 20));
}

bool Ipv4Layer::output_allowed(const Packet &packet, const NetworkInterface &interface)
{
  return !filter_ || filter_->accept(FilterHook::Output, packet.data(), packet.size(),
                                     interface.id());
}

int Ipv4Layer::send(Packet &packet, unsigned destination, unsigned protocol,
                    unsigned source, const NetworkInterface *via)
{
  SendFlow::Context ctx{*this, packet, destination, protocol, source, via};
  (void)SendFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void Ipv4Layer::receive(NetworkInterface &interface, Packet &packet)
{
  ReceiveFlow::Context ctx{*this, interface, packet};
  (void)ReceiveFlow::Graph::run<0>(ctx);
}

void Ipv4Layer::tick(unsigned long now)
{
  reassembly_.tick(now);
}

int Ipv4Layer::send_bootstrap(NetworkInterface &interface, Packet &packet,
                              unsigned source)
{
  SendFlow::Context ctx{*this, packet, 0xffffffff, 17, source, nullptr, &interface};
  (void)SendFlow::BootstrapGraph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::net
