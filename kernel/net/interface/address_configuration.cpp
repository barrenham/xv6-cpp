module;
#include "kernel/net/errno.h"
module xv.net;
import :address_configuration;

namespace xv::net
{
int AddressConfiguration::add_static(NetworkInterface &interface, Ipv4Address address)
{
  ConfigurationGuard guard;
  if (interfaces_.find(interface.id()) != &interface)
  {
    return -ENODEV;
  }
  if (&interface == &loopback_)
  {
    return -EOPNOTSUPP;
  }
  if (interface.master())
  {
    return -EBUSY;
  }
  if (address.prefix > 32 || !address.address || (address.address >> 28) >= 14)
  {
    return -EINVAL;
  }
  if (interface.address_count_ == NetworkInterface::AddressCapacity ||
      routes_.available() < 2)
  {
    return -ENOBUFS;
  }
  for (unsigned i = 0; i < interfaces_.count(); ++i)
  {
    if (interfaces_.at(i)->owns(address.address))
    {
      return -EEXIST;
    }
  }
  if (address.prefix < 31 && ((address.address & ~address.mask()) == 0 ||
                              (address.address & ~address.mask()) == ~address.mask()))
  {
    return -EINVAL;
  }

  address.owner = 0;
  if (!routes_.add_address_routes(interface, loopback_, address))
  {
    return -ENOBUFS;
  }
  interface.addresses_[interface.address_count_++] = address;
  ++interface.generation_;
  return 0;
}

int AddressConfiguration::remove_static(NetworkInterface &interface,
                                        Ipv4Address address)
{
  ConfigurationGuard guard;
  if (interfaces_.find(interface.id()) != &interface)
  {
    return -ENODEV;
  }
  if (&interface == &loopback_)
  {
    return -EOPNOTSUPP;
  }
  if (interface.master())
  {
    return -EBUSY;
  }
  if (address.prefix > 32 || !address.address || (address.address >> 28) >= 14)
  {
    return -EINVAL;
  }
  unsigned position = interface.address_count_;
  for (unsigned i = 0; i < interface.address_count_; ++i)
  {
    if (interface.addresses_[i].address == address.address &&
        interface.addresses_[i].prefix == address.prefix)
    {
      position = i;
    }
  }
  if (position == interface.address_count_)
  {
    return -ENOENT;
  }
  if (interface.addresses_[position].owner)
  {
    return -EBUSY;
  }

  for (unsigned i = position + 1; i < interface.address_count_; ++i)
  {
    interface.addresses_[i - 1] = interface.addresses_[i];
  }
  interface.addresses_[--interface.address_count_] = {};
  routes_.remove_address_routes(interface, address.address);
  ++interface.generation_;
  return 0;
}

int AddressConfiguration::install_lease(NetworkInterface &interface,
                                        Ipv4Address address, unsigned gateway,
                                        unsigned metric, bool &changed)
{
  ConfigurationGuard guard;
  changed = false;
  if (!address.address || address.prefix > 32)
  {
    return -EINVAL;
  }
  if (interfaces_.find(interface.id()) != &interface || !interface.up() ||
      interface.master())
  {
    return -ENETUNREACH;
  }
  unsigned owner = interface.id(), position = interface.address_count_;
  for (unsigned i = 0; i < interface.address_count_; ++i)
  {
    if (interface.addresses_[i].owner == owner)
    {
      position = i;
    }
  }
  for (unsigned i = 0; i < interfaces_.count(); ++i)
  {
    auto *other = interfaces_.at(i);
    if (other != &interface && other->owns(address.address))
    {
      return -EADDRINUSE;
    }
  }
  for (unsigned i = 0; i < interface.address_count_; ++i)
  {
    if (i != position && interface.addresses_[i].address == address.address)
    {
      return -EADDRINUSE;
    }
  }
  if (position == interface.address_count_ &&
      position == NetworkInterface::AddressCapacity)
  {
    return -ENOBUFS;
  }

  address.owner = owner;
  bool same_address = position < interface.address_count_ &&
                      interface.addresses_[position].address == address.address &&
                      interface.addresses_[position].prefix == address.prefix;
  if (same_address &&
      routes_.lease_routes_match(interface, loopback_, address, gateway, metric))
  {
    return 0;
  }
  if (!routes_.replace_lease_routes(interface, loopback_, address, gateway, metric))
  {
    return -ENOBUFS;
  }

  if (position == interface.address_count_)
  {
    ++interface.address_count_;
  }
  interface.addresses_[position] = address;
  ++interface.generation_;
  changed = true;
  return 0;
}

bool AddressConfiguration::remove_lease(NetworkInterface &interface)
{
  ConfigurationGuard guard;
  if (interfaces_.find(interface.id()) != &interface)
  {
    return false;
  }
  unsigned owner = interface.id(), retained = 0;
  bool changed = false;
  for (unsigned i = 0; i < interface.address_count_; ++i)
  {
    if (interface.addresses_[i].owner == owner)
    {
      changed = true;
    }
    else
    {
      interface.addresses_[retained++] = interface.addresses_[i];
    }
  }
  for (unsigned i = retained; i < interface.address_count_; ++i)
  {
    interface.addresses_[i] = {};
  }
  interface.address_count_ = retained;
  if (routes_.remove_lease_routes(interface))
  {
    changed = true;
  }
  if (changed)
  {
    ++interface.generation_;
  }
  return changed;
}
} // namespace xv::net
