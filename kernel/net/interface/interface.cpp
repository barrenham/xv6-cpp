module;
#include "kernel/net/errno.h"
module xv.net;
import :interface;

namespace xv::net
{
bool NetworkInterface::owns(unsigned value) const
{
  ConfigurationGuard guard;
  for (unsigned i = 0; i < address_count_; ++i)
  {
    if (addresses_[i].address == value)
    {
      return true;
    }
  }
  return false;
}

bool NetworkInterface::accepts_destination(unsigned address,
                                           const InterfaceRegistry &) const
{
  return owns(address) || broadcast(address);
}

bool NetworkInterface::broadcast(unsigned value) const
{
  ConfigurationGuard guard;
  if (value == 0xffffffff)
  {
    return address_count_ != 0;
  }
  for (unsigned i = 0; i < address_count_; ++i)
  {
    if (addresses_[i].prefix < 31 &&
        value == (addresses_[i].address | ~addresses_[i].mask()))
    {
      return true;
    }
  }
  return false;
}

unsigned NetworkInterface::source_for(unsigned destination) const
{
  ConfigurationGuard guard;
  const Ipv4Address *best = nullptr;
  for (unsigned i = 0; i < address_count_; ++i)
  {
    if (!best ||
        (addresses_[i].contains(destination) &&
         (!best->contains(destination) || addresses_[i].prefix > best->prefix)))
    {
      best = &addresses_[i];
    }
  }
  return best ? best->address : 0;
}

bool NetworkInterface::add_address(Ipv4Address value)
{
  ConfigurationGuard guard;
  if (sealed_ || address_count_ == AddressCapacity || !value.address ||
      (value.address >> 28) >= 14 || value.prefix > 32 || owns(value.address))
  {
    return false;
  }
  addresses_[address_count_++] = value;
  return true;
}

bool NetworkInterface::set_up(bool value)
{
  ConfigurationGuard guard;
  if (sealed_)
  {
    return false;
  }
  up_ = value;
  return true;
}

void NetworkInterface::configure_link(bool value)
{
  ConfigurationGuard guard;
  up_ = value;
  ++generation_;
}

void NetworkInterface::set_dhcp_active(bool active)
{
  ConfigurationGuard guard;
  dhcp_active_ = active;
}

void NetworkInterface::topology_changed()
{
  ConfigurationGuard guard;
  ++generation_;
}

void NetworkInterface::deliver(Packet &packet)
{
  if (!up() || !receiver_)
  {
    receive_drop();
    return;
  }
  __atomic_fetch_add(&statistics_.rx_packets, 1ULL, __ATOMIC_RELAXED);
  __atomic_fetch_add(&statistics_.rx_bytes,
                     static_cast<unsigned long long>(packet.size()), __ATOMIC_RELAXED);
  receiver_->receive(*this, packet);
}

int InterfaceRegistry::registration_error(const NetworkInterface &interface) const
{
  if (!interface.id() || !interface.name() || interface.mtu() < 68 ||
      interface.mtu() > 4082)
  {
    return -EINVAL;
  }
  if (find(interface.id()))
  {
    return -EEXIST;
  }
  for (unsigned i = 0; i < count_; ++i)
  {
    auto *existing_name = interfaces_[i]->name();
    auto *new_name = interface.name();
    while (*existing_name && *existing_name == *new_name)
    {
      ++existing_name;
      ++new_name;
    }
    if (*existing_name == *new_name)
    {
      return -EEXIST;
    }
    for (unsigned address = 0; address < interface.address_count(); ++address)
    {
      if (interfaces_[i]->owns(interface.address(address).address))
      {
        return -EEXIST;
      }
    }
  }
  return count_ == Capacity ? -ENOBUFS : 0;
}

bool InterfaceRegistry::add(NetworkInterface &interface)
{
  ConfigurationGuard guard;
  if (sealed_ || registration_error(interface))
  {
    return false;
  }
  interfaces_[count_++] = &interface;
  return true;
}

int InterfaceRegistry::publish(NetworkInterface &interface)
{
  ConfigurationGuard guard;
  int result = registration_error(interface);
  if (result < 0)
  {
    return result;
  }
  interface.sealed_ = true;
  ++interface.generation_;
  interfaces_[count_++] = &interface;
  return 0;
}

bool InterfaceRegistry::withdraw(NetworkInterface &interface)
{
  ConfigurationGuard guard;
  for (unsigned i = 0; i < count_; ++i)
  {
    if (interfaces_[i] != &interface)
    {
      continue;
    }
    for (unsigned next = i + 1; next < count_; ++next)
    {
      interfaces_[next - 1] = interfaces_[next];
    }
    interfaces_[--count_] = nullptr;
    ++interface.generation_;
    return true;
  }
  return false;
}

bool InterfaceRegistry::seal()
{
  ConfigurationGuard guard;
  for (unsigned i = 0; i < count_; ++i)
  {
    for (unsigned k = 0; k < count_; ++k)
    {
      if (i != k)
      {
        for (unsigned n = 0; n < interfaces_[i]->address_count(); ++n)
        {
          if (interfaces_[k]->owns(interfaces_[i]->address(n).address))
          {
            return false;
          }
        }
      }
    }
  }
  sealed_ = true;
  for (unsigned i = 0; i < count_; ++i)
  {
    interfaces_[i]->sealed_ = true;
  }
  return true;
}

NetworkInterface *InterfaceRegistry::find(unsigned id) const
{
  ConfigurationGuard guard;
  for (unsigned i = 0; i < count_; ++i)
  {
    if (interfaces_[i]->id() == id)
    {
      return interfaces_[i];
    }
  }
  return nullptr;
}

bool InterfaceRegistry::owns(unsigned address) const
{
  ConfigurationGuard guard;
  for (unsigned i = 0; i < count_; ++i)
  {
    if (interfaces_[i]->up() && interfaces_[i]->owns(address))
    {
      return true;
    }
  }
  return false;
}

void InterfaceRegistry::tick(unsigned long now)
{
  NetworkInterface *snapshot[Capacity];
  unsigned count;
  {
    ConfigurationGuard guard;
    count = count_;
    for (unsigned i = 0; i < count; ++i)
    {
      snapshot[i] = interfaces_[i];
    }
  }
  for (unsigned i = 0; i < count; ++i)
  {
    snapshot[i]->tick(now);
  }
}

InterfaceStatistics NetworkInterface::statistics() const
{
  return {__atomic_load_n(&statistics_.rx_packets, __ATOMIC_RELAXED),
          __atomic_load_n(&statistics_.rx_bytes, __ATOMIC_RELAXED),
          __atomic_load_n(&statistics_.rx_dropped, __ATOMIC_RELAXED),
          __atomic_load_n(&statistics_.tx_packets, __ATOMIC_RELAXED),
          __atomic_load_n(&statistics_.tx_bytes, __ATOMIC_RELAXED),
          __atomic_load_n(&statistics_.tx_errors, __ATOMIC_RELAXED)};
}

void NetworkInterface::receive_drop()
{
  __atomic_fetch_add(&statistics_.rx_dropped, 1ULL, __ATOMIC_RELAXED);
}

int NetworkInterface::send(Packet &packet, unsigned next_hop)
{
  unsigned length = packet.size();
  int result = transmit(packet, next_hop);
  return account_send(length, result);
}

int NetworkInterface::send_bootstrap(Packet &packet)
{
  unsigned length = packet.size();
  return account_send(length, transmit_bootstrap(packet));
}

int NetworkInterface::account_send(unsigned length, int result)
{
  if (result < 0)
  {
    __atomic_fetch_add(&statistics_.tx_errors, 1ULL, __ATOMIC_RELAXED);
  }
  else
  {
    __atomic_fetch_add(&statistics_.tx_packets, 1ULL, __ATOMIC_RELAXED);
    __atomic_fetch_add(&statistics_.tx_bytes, static_cast<unsigned long long>(length),
                       __ATOMIC_RELAXED);
  }
  return result;
}
} // namespace xv::net
