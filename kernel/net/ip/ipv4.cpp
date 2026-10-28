module;
#include "kernel/net/errno.h"
module xv.net;
import :ipv4;
import :platform;

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
  RouteSelection selection;
  if (!routes_.lookup(destination, selection, source, via))
  {
    return -ENETUNREACH;
  }
  if (!packet || packet.size() < 20 || packet.size() > selection.interface->mtu())
  {
    return -EMSGSIZE;
  }
  write_header(packet, selection.source, destination, protocol);
  if (!output_allowed(packet, *selection.interface))
  {
    return -EACCES;
  }
  return selection.interface->send(packet, selection.next_hop);
}

void Ipv4Layer::receive(NetworkInterface &interface, Packet &packet)
{
  if (!interface.up() || interfaces_.find(interface.id()) != &interface || !packet ||
      packet.size() < 20)
  {
    interface.receive_drop();
    return;
  }
  auto *header = packet.data();
  unsigned header_length = (header[0] & 15) * 4,
           total_length = wire::read16(header + 2);
  unsigned source = wire::read32(header + 12), destination = wire::read32(header + 16);
  if ((header[0] >> 4) != 4 || header_length < 20 || total_length < header_length ||
      total_length > packet.size() || !header[8] ||
      (wire::read16(header + 6) & 0x8000) ||
      wire::sum(header, header_length) != 0xffff || !source || (source >> 28) >= 14)
  {
    interface.receive_drop();
    return;
  }
  if (wire::read16(header + 6) & 0x3fff)
  {
    if (!interface.accepts_destination(destination, interfaces_))
    {
      interface.receive_drop();
      return;
    }
    auto complete = reassembly_.accept(&interface, packet, header_length, total_length,
                                       detail::milliseconds());
    if (complete)
    {
      receive(interface, complete);
    }
    return;
  }
  bool local_destination = interface.accepts_destination(destination, interfaces_);
  bool bootstrap_candidate = bootstrap_ && interface.dhcp_active() && header[9] == 17 &&
                             total_length - header_length >= 8 &&
                             wire::read16(header + header_length) == 67 &&
                             wire::read16(header + header_length + 2) == 68;
  if (!local_destination && !bootstrap_candidate)
  {
    interface.receive_drop();
    return;
  }
  if (filter_ &&
      !filter_->accept(FilterHook::Input, header, total_length, interface.id()))
  {
    interface.receive_drop();
    return;
  }
  IpDatagram datagram{
      source,    destination, header[9], header_length, total_length - header_length,
      &interface};
  if (datagram.protocol == 17 && bootstrap_ &&
      bootstrap_->receive_bootstrap(interface, packet, datagram))
  {
    return;
  }
  if (!local_destination)
  {
    interface.receive_drop();
    return;
  }
  for (auto &binding : handlers_)
  {
    if (!binding.handler || binding.protocol != datagram.protocol)
    {
      continue;
    }
    binding.handler->receive(packet, datagram);
    return;
  }
}

void Ipv4Layer::tick(unsigned long now)
{
  reassembly_.tick(now);
}

int Ipv4Layer::send_bootstrap(NetworkInterface &interface, Packet &packet,
                              unsigned source)
{
  if (!packet || packet.size() < 28 || packet.size() > interface.mtu())
  {
    return -EMSGSIZE;
  }
  if (interfaces_.find(interface.id()) != &interface || !interface.dhcp_active() ||
      !interface.up() || interface.master() || (source && !interface.owns(source)))
  {
    return -ENETUNREACH;
  }
  if (wire::read16(packet.data() + 20) != 68 || wire::read16(packet.data() + 22) != 67)
  {
    return -EINVAL;
  }
  write_header(packet, source, 0xffffffff, 17);
  if (!output_allowed(packet, interface))
  {
    return -EACCES;
  }
  return interface.send_bootstrap(packet);
}
} // namespace xv::net
