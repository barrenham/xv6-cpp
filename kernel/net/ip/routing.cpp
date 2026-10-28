module;
#include "kernel/net/errno.h"
module xv.net;
import :routing;

namespace xv::net
{
bool RoutingTable::valid(const Route &route) const
{
  auto &interface = *route.interface;
  if (interface.master() || route.prefix > 32 ||
      interfaces_.find(interface.id()) != &interface ||
      (route.gateway && !interface.source_for(route.gateway)) ||
      (route.source && (!interfaces_.owns(route.source) ||
                        (interface.mac() && !interface.owns(route.source)))))
  {
    return false;
  }
  if (route.gateway)
  {
    bool connected = false;
    for (unsigned i = 0; i < interface.address_count(); ++i)
    {
      if (interface.address(i).contains(route.gateway))
      {
        connected = true;
      }
    }
    if (!connected || interface.owns(route.gateway) ||
        interface.broadcast(route.gateway))
    {
      return false;
    }
  }
  return true;
}

bool RoutingTable::add(unsigned network, unsigned prefix, NetworkInterface &interface,
                       unsigned gateway, unsigned metric, unsigned source,
                       unsigned owner, unsigned local_address)
{
  ConfigurationGuard guard;
  Route route{network, prefix,     gateway, metric,
              source,  &interface, owner,   local_address};
  if (sealed_ || count_ == Capacity || !valid(route))
  {
    return false;
  }
  route.network &= Ipv4Address{0, prefix}.mask();
  routes_[count_++] = route;
  return true;
}

bool RoutingTable::at(unsigned index, Route &result) const
{
  ConfigurationGuard guard;
  if (index >= count_)
  {
    return false;
  }
  result = routes_[index];
  return true;
}

bool RoutingTable::references(const NetworkInterface &interface) const
{
  ConfigurationGuard guard;
  for (unsigned i = 0; i < count_; ++i)
  {
    if (routes_[i].interface == &interface ||
        routes_[i].address_owner == interface.id())
    {
      return true;
    }
  }
  return false;
}

bool RoutingTable::same_path(const Route &left, const Route &right)
{
  return left.network == right.network && left.prefix == right.prefix &&
         left.interface == right.interface && left.gateway == right.gateway &&
         left.metric == right.metric && left.source == right.source;
}

int RoutingTable::find(const Route &route) const
{
  for (unsigned i = 0; i < count_; ++i)
  {
    if (same_path(routes_[i], route))
    {
      return i;
    }
  }
  return -1;
}

void RoutingTable::erase(unsigned index)
{
  for (unsigned i = index + 1; i < count_; ++i)
  {
    routes_[i - 1] = routes_[i];
  }
  routes_[--count_] = {};
}

int RoutingTable::add_static(unsigned network, unsigned prefix,
                             NetworkInterface &interface, unsigned gateway,
                             unsigned metric, unsigned source)
{
  ConfigurationGuard guard;
  if (prefix > 32)
  {
    return -EINVAL;
  }
  Route route{network & Ipv4Address{0, prefix}.mask(),
              prefix,
              gateway,
              metric,
              source,
              &interface};
  if (find(route) >= 0)
  {
    return -EEXIST;
  }
  if (count_ == Capacity)
  {
    return -ENOBUFS;
  }
  if (!valid(route))
  {
    return -EINVAL;
  }
  routes_[count_++] = route;
  return 0;
}

int RoutingTable::remove_static(unsigned network, unsigned prefix,
                                NetworkInterface &interface, unsigned gateway,
                                unsigned metric, unsigned source)
{
  ConfigurationGuard guard;
  if (prefix > 32)
  {
    return -EINVAL;
  }
  Route route{network & Ipv4Address{0, prefix}.mask(),
              prefix,
              gateway,
              metric,
              source,
              &interface};
  int index = find(route);
  if (index < 0)
  {
    return -ENOENT;
  }
  if (routes_[index].address_owner || routes_[index].lease_owner)
  {
    return -EBUSY;
  }
  erase(index);
  return 0;
}

RoutingTable::RouteSet RoutingTable::address_routes(NetworkInterface &interface,
                                                    NetworkInterface &loopback,
                                                    Ipv4Address address,
                                                    unsigned lease_owner,
                                                    unsigned gateway, unsigned metric)
{
  RouteSet result{
      {{address.address, 32, 0, 0, address.address, &loopback, interface.id(),
        address.address, lease_owner},
       {address.address & address.mask(), address.prefix, 0, 0, 0, &interface,
        interface.id(), address.address, lease_owner},
       {0, 0, gateway, metric, address.address, &interface, 0, 0, lease_owner}},
      gateway ? 3U : 2U};
  return result;
}

bool RoutingTable::add_address_routes(NetworkInterface &interface,
                                      NetworkInterface &loopback, Ipv4Address address)
{
  ConfigurationGuard guard;
  if (address.prefix > 32 || available() < 2)
  {
    return false;
  }
  auto desired = address_routes(interface, loopback, address);
  for (unsigned i = 0; i < desired.count; ++i)
  {
    routes_[count_++] = desired.entries[i];
  }
  return true;
}

void RoutingTable::remove_address_routes(const NetworkInterface &interface,
                                         unsigned address)
{
  ConfigurationGuard guard;
  for (unsigned i = 0; i < count_;)
  {
    const auto &route = routes_[i];
    if ((route.address_owner == interface.id() && route.local_address == address) ||
        route.source == address)
    {
      erase(i);
    }
    else
    {
      ++i;
    }
  }
}

unsigned RoutingTable::lease_route_count(unsigned owner) const
{
  unsigned count = 0;
  for (unsigned i = 0; i < count_; ++i)
  {
    if (routes_[i].lease_owner == owner)
    {
      ++count;
    }
  }
  return count;
}

bool RoutingTable::lease_routes_match(NetworkInterface &interface,
                                      NetworkInterface &loopback, Ipv4Address address,
                                      unsigned gateway, unsigned metric) const
{
  ConfigurationGuard guard;
  if (address.prefix > 32)
  {
    return false;
  }
  auto desired =
      address_routes(interface, loopback, address, interface.id(), gateway, metric);
  if (lease_route_count(interface.id()) != desired.count)
  {
    return false;
  }
  for (unsigned n = 0; n < desired.count; ++n)
  {
    bool found = false;
    for (unsigned i = 0; i < count_; ++i)
    {
      if (routes_[i].lease_owner == interface.id() &&
          same_path(routes_[i], desired.entries[n]))
      {
        found = true;
      }
    }
    if (!found)
    {
      return false;
    }
  }
  return true;
}

bool RoutingTable::replace_lease_routes(NetworkInterface &interface,
                                        NetworkInterface &loopback, Ipv4Address address,
                                        unsigned gateway, unsigned metric)
{
  ConfigurationGuard guard;
  if (address.prefix > 32)
  {
    return false;
  }
  auto desired =
      address_routes(interface, loopback, address, interface.id(), gateway, metric);
  if (count_ - lease_route_count(interface.id()) + desired.count > Capacity)
  {
    return false;
  }
  remove_lease_routes(interface);
  for (unsigned i = 0; i < desired.count; ++i)
  {
    routes_[count_++] = desired.entries[i];
  }
  return true;
}

bool RoutingTable::remove_lease_routes(const NetworkInterface &interface)
{
  ConfigurationGuard guard;
  unsigned retained = 0, previous_count = count_;
  for (unsigned i = 0; i < previous_count; ++i)
  {
    if (routes_[i].lease_owner != interface.id())
    {
      routes_[retained++] = routes_[i];
    }
  }
  for (unsigned i = retained; i < previous_count; ++i)
  {
    routes_[i] = {};
  }
  count_ = retained;
  return count_ != previous_count;
}

bool RoutingTable::lookup(unsigned destination, RouteSelection &result, unsigned source,
                          const NetworkInterface *via) const
{
  ConfigurationGuard guard;
  result = {};
  if (!destination || ((destination >> 28) >= 14 && destination != 0xffffffff))
  {
    return false;
  }
  const Route *best = nullptr;
  for (unsigned i = 0; i < count_; ++i)
  {
    const auto &route = routes_[i];
    auto &interface = *route.interface;
    if (route.source && !interfaces_.owns(route.source))
    {
      continue;
    }
    if (route.gateway)
    {
      bool connected = false;
      for (unsigned address = 0; address < interface.address_count(); ++address)
      {
        if (interface.address(address).contains(route.gateway))
        {
          connected = true;
        }
      }
      if (!connected || interface.owns(route.gateway) ||
          interface.broadcast(route.gateway))
      {
        continue;
      }
    }
    unsigned mask = Ipv4Address{0, route.prefix}.mask();
    if (interface.master() || !interface.up() || !interface.carrier() ||
        (via && via != &interface) || (destination & mask) != route.network ||
        (source && !interface.owns(source) && source != route.source))
    {
      continue;
    }
    unsigned selected_source =
        source ? source
               : (route.source ? route.source : interface.source_for(destination));
    if (!selected_source)
    {
      continue;
    }
    if (!best || route.prefix > best->prefix ||
        (route.prefix == best->prefix && route.metric < best->metric))
    {
      best = &route;
    }
  }
  if (!best)
  {
    return false;
  }
  auto &interface = *best->interface;
  unsigned selected_source =
      source ? source
             : (best->source ? best->source : interface.source_for(destination));
  if (!selected_source)
  {
    return false;
  }
  unsigned next_hop = interface.broadcast(destination)
                          ? 0xffffffffU
                          : (best->gateway ? best->gateway : destination);
  result = {&interface, next_hop, selected_source};
  return true;
}
} // namespace xv::net
