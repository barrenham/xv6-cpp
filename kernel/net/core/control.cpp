module;
#include "kernel/net/control.h"
#include "kernel/net/errno.h"
module xv.net;
import :control;

namespace xv::net
{
int NetworkManager::inspect(unsigned operation, netctl_request &request)
{
  unsigned id = request.ifindex, index = request.index;
  netctl_request out{};
  out.version = NETCTL_VERSION;
  out.size = sizeof(out);
  out.index = index;
  if (operation == NETCTL_ROUTE)
  {
    RoutingTable::Route r;
    if (!routes_.at(index, r))
    {
      return 0;
    }
    out.ifindex = r.interface->id();
    out.address = r.network;
    out.prefix = r.prefix;
    out.gateway = r.gateway;
    out.metric = r.metric;
    out.source = r.source;
    out.flags = (r.address_owner ? NET_ROUTE_AUTOMATIC : 0) |
                (r.lease_owner ? NET_CONFIG_DHCP : 0);
  }
  else
  {
    auto *interface = operation == NETCTL_LINK
                          ? (id ? interfaces_.find(id) : interfaces_.at(index))
                          : interfaces_.find(id);
    if (!interface)
    {
      return id ? -ENODEV : 0;
    }
    out.ifindex = interface->id();
    if (operation == NETCTL_ADDRESS)
    {
      if (index >= interface->address_count())
      {
        return 0;
      }
      auto a = interface->address(index);
      out.address = a.address;
      out.prefix = a.prefix;
      out.flags = a.owner ? NET_CONFIG_DHCP : 0;
    }
    else if (operation == NETCTL_LINK)
    {
      out.mtu = interface->mtu();
      out.address_count = interface->address_count();
      out.flags = (interface->up() ? NET_LINK_UP : 0) |
                  (interface->carrier() ? NET_LINK_CARRIER : 0) |
                  (interface->is_tunnel() ? NET_LINK_TUNNEL : 0) |
                  (interface->is_bridge() ? NET_LINK_BRIDGE : 0) |
                  (interface->master() ? NET_LINK_SLAVE : 0);
      out.source = interface->master();
      for (unsigned i = 0; i < sizeof(out.name) - 1 && interface->name()[i]; ++i)
      {
        out.name[i] = interface->name()[i];
      }
      if (auto *mac = interface->mac())
      {
        out.flags |= NET_LINK_MAC;
        for (unsigned i = 0; i < 6; ++i)
        {
          out.mac[i] = mac->bytes[i];
        }
      }
      auto s = interface->statistics();
      out.rx_packets = s.rx_packets;
      out.rx_bytes = s.rx_bytes;
      out.rx_dropped = s.rx_dropped;
      out.tx_packets = s.tx_packets;
      out.tx_bytes = s.tx_bytes;
      out.tx_errors = s.tx_errors;
    }
    else
    {
      return -EINVAL;
    }
  }
  request = out;
  return 1;
}

int NetworkManager::mutate(unsigned operation, const netctl_request &r,
                           NetworkInterface &interface)
{
  if (operation == NETCTL_SET_LINK)
  {
    if (&interface == &loopback_)
    {
      return -EOPNOTSUPP;
    }
    if (r.flags & ~NET_LINK_UP)
    {
      return -EINVAL;
    }
    interface.configure_link(bool(r.flags & NET_LINK_UP));
    return 0;
  }
  if (operation == NETCTL_ADD_ADDRESS)
  {
    return addresses_.add_static(interface, {r.address, r.prefix});
  }
  if (operation == NETCTL_DEL_ADDRESS)
  {
    return addresses_.remove_static(interface, {r.address, r.prefix});
  }
  if (operation != NETCTL_ADD_ROUTE && operation != NETCTL_DEL_ROUTE)
  {
    return -EINVAL;
  }
  if (r.prefix > 32 || r.flags)
  {
    return -EINVAL;
  }
  if (operation == NETCTL_ADD_ROUTE)
  {
    return routes_.add_static(r.address, r.prefix, interface, r.gateway, r.metric,
                              r.source);
  }
  return routes_.remove_static(r.address, r.prefix, interface, r.gateway, r.metric,
                               r.source);
}

int NetworkManager::execute(unsigned operation, netctl_request &request)
{
  if (request.version != NETCTL_VERSION || request.size != sizeof(request))
  {
    return -EINVAL;
  }
  if (operation >= NETCTL_BRIDGE_CREATE && operation <= NETCTL_BRIDGE_FLUSH_FDB)
  {
    return bridges_ ? bridges_->execute(operation, request) : -EOPNOTSUPP;
  }
  if (operation >= NETCTL_DHCP_START && operation <= NETCTL_DHCP_RENEW)
  {
    return dhcp_ ? dhcp_->execute(operation, request) : -EOPNOTSUPP;
  }
  NetworkInterface *interface;
  int result;
  {
    ConfigurationGuard guard;
    if (operation == NETCTL_LINK || operation == NETCTL_ADDRESS ||
        operation == NETCTL_ROUTE)
    {
      return inspect(operation, request);
    }
    interface = interfaces_.find(request.ifindex);
    if (!interface)
    {
      return -ENODEV;
    }
    if (operation != NETCTL_NEIGHBOR && operation != NETCTL_FLUSH_NEIGHBORS)
    {
      result = mutate(operation, request, *interface);
    }
    else
    {
      result = 0;
    }
  }
  if (operation == NETCTL_NEIGHBOR)
  {
    NeighborInfo neighbor;
    if (!interface->neighbor(request.index, neighbor))
    {
      return 0;
    }
    netctl_request out{};
    out.version = NETCTL_VERSION;
    out.size = sizeof(out);
    out.ifindex = interface->id();
    out.index = request.index;
    out.address = neighbor.address;
    out.expires_ms = neighbor.expires_ms;
    for (unsigned i = 0; i < 6; ++i)
    {
      out.mac[i] = neighbor.mac[i];
    }
    request = out;
    return 1;
  }
  if (result == 0 &&
      (operation == NETCTL_SET_LINK || operation == NETCTL_ADD_ADDRESS ||
       operation == NETCTL_DEL_ADDRESS || operation == NETCTL_FLUSH_NEIGHBORS))
  {
    interface->invalidate();
  }
  return result;
}
} // namespace xv::net
