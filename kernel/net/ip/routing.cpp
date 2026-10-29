module;
#include "kernel/net/errno.h"
module xv.net;
import :routing_transaction_flows;
import :routing_lookup_flow;
import :routing;

namespace xv::net
{
bool RoutingTable::valid(const Route &route) const
{
  ValidationFlow::Context ctx{*this, route};
  (void)ValidationFlow::Graph::run<0>(ctx);
  return ctx.result;
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
  MutationFlow::Context ctx{*this, network, prefix, interface, gateway, metric, source};
  (void)MutationFlow::AddGraph::run<0>(ctx);
  return ctx.result;
}

int RoutingTable::remove_static(unsigned network, unsigned prefix,
                                NetworkInterface &interface, unsigned gateway,
                                unsigned metric, unsigned source)
{
  ConfigurationGuard guard;
  MutationFlow::Context ctx{*this, network, prefix, interface, gateway, metric, source};
  (void)MutationFlow::RemoveGraph::run<0>(ctx);
  return ctx.result;
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
  LeaseFlow::MatchContext ctx{*this, interface, loopback, address, gateway, metric};
  (void)LeaseFlow::MatchGraph::run<0>(ctx);
  return ctx.result;
}

bool RoutingTable::replace_lease_routes(NetworkInterface &interface,
                                        NetworkInterface &loopback, Ipv4Address address,
                                        unsigned gateway, unsigned metric)
{
  ConfigurationGuard guard;
  LeaseFlow::Context ctx{*this, interface, loopback, address, gateway, metric};
  (void)LeaseFlow::ReplaceGraph::run<0>(ctx);
  return ctx.result;
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
  LookupFlow::Context ctx{*this, destination, result, source, via};
  (void)LookupFlow::Graph::run<0>(ctx);
  return ctx.found;
}
} // namespace xv::net
