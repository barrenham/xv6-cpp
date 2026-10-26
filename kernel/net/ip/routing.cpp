module xv.net;
import :routing;
namespace xv::net
{
bool RoutingTable::add(unsigned network, unsigned prefix, NetworkInterface &interface,
                       unsigned gateway, unsigned metric, unsigned source)
{
  if (sealed_ || count_ == 32 || prefix > 32 || interfaces_.find(interface.id()) != &interface ||
      (gateway && !interface.source_for(gateway)) || (source && !interfaces_.owns(source))) return false;
  if (gateway)
  {
    bool connected = false;
    for (unsigned i = 0; i < interface.address_count(); ++i)
      if (interface.address(i).contains(gateway)) connected = true;
    if (!connected || interface.owns(gateway) || interface.broadcast(gateway)) return false;
  }
  unsigned mask = prefix ? ~0U << (32-prefix) : 0;
  routes_[count_++] = {network & mask, prefix, gateway, metric, source, &interface}; return true;
}
bool RoutingTable::lookup(unsigned destination, RouteSelection &result, unsigned source,
                          const NetworkInterface *via) const
{
  result = {};
  if (!destination || ((destination >> 28) >= 14 && destination != 0xffffffff)) return false;
  const Route *best = nullptr;
  for (unsigned i = 0; i < count_; ++i)
  {
    const auto &r = routes_[i]; auto &interface = *r.interface;
    unsigned mask = r.prefix ? ~0U << (32-r.prefix) : 0;
    if (!interface.up() || !interface.carrier() || (via && via != &interface) ||
        (destination & mask) != r.network ||
        (source && !interface.owns(source) && source != r.source)) continue;
    if (!best || r.prefix > best->prefix || (r.prefix == best->prefix && r.metric < best->metric)) best = &r;
  }
  if (!best) return false;
  auto &interface = *best->interface;
  unsigned selected = source ? source : (best->source ? best->source : interface.source_for(destination));
  if (!selected) return false;
  result = {&interface, interface.broadcast(destination) ? 0xffffffffU : (best->gateway ? best->gateway : destination), selected};
  return true;
}
}
