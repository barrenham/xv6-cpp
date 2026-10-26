module;
#include "kernel/net/control.h"
#include "kernel/net/errno.h"
module xv.net;
import :control;
import :platform;
namespace xv::net
{
void NetworkManager::remove_route(unsigned index)
{
  for (unsigned i = index+1; i < routes_.count_; ++i) routes_.routes_[i-1] = routes_.routes_[i];
  routes_.routes_[--routes_.count_] = {};
}
int NetworkManager::inspect(unsigned operation, netctl_request &request)
{
  unsigned id = request.ifindex, index = request.index;
  netctl_request out{}; out.version = NETCTL_VERSION; out.size = sizeof(out); out.index = index;
  if (operation == NETCTL_ROUTE)
  {
    if (index >= routes_.count_) return 0;
    const auto &r = routes_.routes_[index];
    out.ifindex = r.interface->id(); out.address = r.network; out.prefix = r.prefix;
    out.gateway = r.gateway; out.metric = r.metric; out.source = r.source;
    out.flags = r.owner ? NET_ROUTE_AUTOMATIC : 0;
  }
  else
  {
    auto *interface = operation == NETCTL_LINK ? (id ? interfaces_.find(id) : interfaces_.at(index)) : interfaces_.find(id);
    if (!interface) return id ? -ENODEV : 0;
    out.ifindex = interface->id();
    if (operation == NETCTL_ADDRESS)
    {
      if (index >= interface->address_count()) return 0;
      auto a = interface->address(index); out.address = a.address; out.prefix = a.prefix;
    }
    else if (operation == NETCTL_LINK)
    {
      out.mtu = interface->mtu(); out.address_count = interface->address_count();
      out.flags = (interface->up() ? NET_LINK_UP : 0) | (interface->carrier() ? NET_LINK_CARRIER : 0) |
                  (interface->is_tunnel() ? NET_LINK_TUNNEL : 0) |
                  (interface->is_bridge() ? NET_LINK_BRIDGE : 0) | (interface->master() ? NET_LINK_SLAVE : 0);
      out.source = interface->master();
      for (unsigned i = 0; i < sizeof(out.name)-1 && interface->name()[i]; ++i) out.name[i] = interface->name()[i];
      if (auto *mac = interface->mac())
      { out.flags |= NET_LINK_MAC; for (unsigned i = 0; i < 6; ++i) out.mac[i] = mac->bytes[i]; }
      auto s = interface->statistics();
      out.rx_packets = s.rx_packets; out.rx_bytes = s.rx_bytes; out.rx_dropped = s.rx_dropped;
      out.tx_packets = s.tx_packets; out.tx_bytes = s.tx_bytes; out.tx_errors = s.tx_errors;
    }
    else return -EINVAL;
  }
  request = out; return 1;
}
int NetworkManager::mutate(unsigned operation, const netctl_request &r, NetworkInterface &interface)
{
  if (operation == NETCTL_SET_LINK)
  {
    if (&interface == &loopback_) return -EOPNOTSUPP;
    if (r.flags & ~NET_LINK_UP) return -EINVAL;
    interface.up_ = bool(r.flags & NET_LINK_UP); ++interface.generation_; return 0;
  }
  if (operation == NETCTL_ADD_ADDRESS || operation == NETCTL_DEL_ADDRESS)
  {
    if (&interface == &loopback_) return -EOPNOTSUPP;
    if (interface.master()) return -EBUSY;
    if (r.prefix > 32 || !r.address || (r.address >> 28) >= 14) return -EINVAL;
    unsigned position = interface.address_count_;
    for (unsigned i = 0; i < interface.address_count_; ++i)
      if (interface.addresses_[i].address == r.address && interface.addresses_[i].prefix == r.prefix) position = i;
    if (operation == NETCTL_ADD_ADDRESS)
    {
      if (interface.address_count_ == 4 || routes_.count_ > 30) return -ENOBUFS;
      for (unsigned i = 0; i < interfaces_.count(); ++i)
        if (interfaces_.at(i)->owns(r.address)) return -EEXIST;
      Ipv4Address value{r.address, r.prefix};
      if (r.prefix < 31 && ((r.address & ~value.mask()) == 0 || (r.address & ~value.mask()) == ~value.mask())) return -EINVAL;
      interface.addresses_[interface.address_count_++] = value;
      unsigned network = r.address & value.mask();
      routes_.routes_[routes_.count_++] = {r.address, 32, 0, 0, r.address, &loopback_, interface.id(), r.address};
      routes_.routes_[routes_.count_++] = {network, r.prefix, 0, 0, 0, &interface, interface.id(), r.address};
    }
    else
    {
      if (position == interface.address_count_) return -ENOENT;
      for (unsigned i = position+1; i < interface.address_count_; ++i) interface.addresses_[i-1] = interface.addresses_[i];
      interface.addresses_[--interface.address_count_] = {};
      for (unsigned i = 0; i < routes_.count_;)
      {
        const auto &route = routes_.routes_[i];
        if ((route.owner == interface.id() && route.local_address == r.address) || route.source == r.address) remove_route(i);
        else ++i;
      }
    }
    ++interface.generation_; return 0;
  }
  if (operation != NETCTL_ADD_ROUTE && operation != NETCTL_DEL_ROUTE) return -EINVAL;
  if (r.prefix > 32 || r.flags) return -EINVAL;
  unsigned mask = Ipv4Address{0, r.prefix}.mask(), network = r.address & mask;
  for (unsigned i = 0; i < routes_.count_; ++i)
  {
    const auto &route = routes_.routes_[i];
    if (route.network == network && route.prefix == r.prefix && route.interface == &interface &&
        route.gateway == r.gateway && route.metric == r.metric && route.source == r.source)
    {
      if (operation == NETCTL_ADD_ROUTE) return -EEXIST;
      if (route.owner) return -EBUSY;
      remove_route(i); return 0;
    }
  }
  if (operation == NETCTL_DEL_ROUTE) return -ENOENT;
  if (routes_.count_ == 32) return -ENOBUFS;
  bool sealed = routes_.sealed_; routes_.sealed_ = false;
  bool added = routes_.add(network, r.prefix, interface, r.gateway, r.metric, r.source);
  routes_.sealed_ = sealed; return added ? 0 : -EINVAL;
}
int NetworkManager::execute(unsigned operation, netctl_request &request)
{
  if (request.version != NETCTL_VERSION || request.size != sizeof(request)) return -EINVAL;
  if (operation >= NETCTL_BRIDGE_CREATE && operation <= NETCTL_BRIDGE_FLUSH_FDB)
    return bridges_ ? bridges_->execute(operation, request) : -EOPNOTSUPP;
  NetworkInterface *interface;
  int result;
  {
    ConfigurationGuard guard;
    if (operation == NETCTL_LINK || operation == NETCTL_ADDRESS || operation == NETCTL_ROUTE)
      return inspect(operation, request);
    interface = interfaces_.find(request.ifindex);
    if (!interface) return -ENODEV;
    if (operation != NETCTL_NEIGHBOR && operation != NETCTL_FLUSH_NEIGHBORS)
      result = mutate(operation, request, *interface);
    else result = 0;
  }
  if (operation == NETCTL_NEIGHBOR)
  {
    NeighborInfo neighbor;
    if (!interface->neighbor(request.index, neighbor)) return 0;
    netctl_request out{}; out.version = NETCTL_VERSION; out.size = sizeof(out);
    out.ifindex = interface->id(); out.index = request.index; out.address = neighbor.address; out.expires_ms = neighbor.expires_ms;
    for (unsigned i = 0; i < 6; ++i) out.mac[i] = neighbor.mac[i];
    request = out; return 1;
  }
  if (result == 0 && (operation == NETCTL_SET_LINK || operation == NETCTL_ADD_ADDRESS ||
      operation == NETCTL_DEL_ADDRESS || operation == NETCTL_FLUSH_NEIGHBORS)) interface->invalidate();
  return result;
}
}
