module xv.net;
import :routing;
namespace xv::net
{
bool RoutingTable::add(unsigned network, unsigned prefix, NetworkInterface &interface,
                       unsigned gateway, unsigned metric, unsigned source, unsigned owner, unsigned local_address)
{
  ConfigurationGuard guard;
  if (interface.master() || sealed_ || count_ == 32 || prefix > 32 || interfaces_.find(interface.id()) != &interface ||
      (gateway && !interface.source_for(gateway)) || (source && (!interfaces_.owns(source) || (interface.mac() && !interface.owns(source))))) return false;
  if (gateway)
  {
    bool connected = false;
    for (unsigned i = 0; i < interface.address_count(); ++i)
      if (interface.address(i).contains(gateway)) connected = true;
    if (!connected || interface.owns(gateway) || interface.broadcast(gateway)) return false;
  }
  unsigned mask = prefix ? ~0U << (32-prefix) : 0;
  routes_[count_++] = {network & mask, prefix, gateway, metric, source, &interface, owner, local_address}; return true;
}
bool RoutingTable::lookup(unsigned destination, RouteSelection &result, unsigned source,
                          const NetworkInterface *via) const
{
  ConfigurationGuard guard;
  result = {};
  if (!destination || ((destination >> 28) >= 14 && destination != 0xffffffff)) return false;
  const Route *best = nullptr;
  for (unsigned i = 0; i < count_; ++i)
  {
    const auto &r = routes_[i]; auto &interface = *r.interface;
    if (r.source && !interfaces_.owns(r.source)) continue;
    if (r.gateway)
    {
      bool connected = false;
      for (unsigned a = 0; a < interface.address_count(); ++a)
        if (interface.address(a).contains(r.gateway)) connected = true;
      if (!connected || interface.owns(r.gateway) || interface.broadcast(r.gateway)) continue;
    }
    unsigned mask = r.prefix ? ~0U << (32-r.prefix) : 0;
    if (interface.master() || !interface.up() || !interface.carrier() || (via && via != &interface) ||
        (destination & mask) != r.network ||
        (source && !interface.owns(source) && source != r.source)) continue;
    unsigned selected = source ? source : (r.source ? r.source : interface.source_for(destination));
    if (!selected) continue;
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
