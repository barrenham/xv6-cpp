module xv.net;
import :interface;
namespace xv::net
{
bool NetworkInterface::owns(unsigned value) const
{ ConfigurationGuard guard; for (unsigned i = 0; i < address_count_; ++i) if (addresses_[i].address == value) return true; return false; }
bool NetworkInterface::accepts_destination(unsigned address, const InterfaceRegistry &) const
{ return owns(address) || broadcast(address); }
bool NetworkInterface::broadcast(unsigned value) const
{
  ConfigurationGuard guard;
  if (value == 0xffffffff) return address_count_ != 0;
  for (unsigned i = 0; i < address_count_; ++i)
    if (addresses_[i].prefix < 31 && value == (addresses_[i].address | ~addresses_[i].mask())) return true;
  return false;
}
unsigned NetworkInterface::source_for(unsigned destination) const
{
  ConfigurationGuard guard;
  const Ipv4Address *best = nullptr;
  for (unsigned i = 0; i < address_count_; ++i)
    if (!best || (addresses_[i].contains(destination) &&
        (!best->contains(destination) || addresses_[i].prefix > best->prefix))) best = &addresses_[i];
  return best ? best->address : 0;
}
bool NetworkInterface::add_address(Ipv4Address value)
{
  ConfigurationGuard guard;
  if (sealed_ || address_count_ == 4 || !value.address || (value.address >> 28) >= 14 ||
      value.prefix > 32 || owns(value.address)) return false;
  addresses_[address_count_++] = value; return true;
}
bool NetworkInterface::set_up(bool value)
{ ConfigurationGuard guard; if (sealed_) return false; up_ = value; return true; }
void NetworkInterface::deliver(Packet &packet)
{
  if (!up() || !receiver_) { receive_drop(); return; }
  __atomic_fetch_add(&statistics_.rx_packets, 1ULL, __ATOMIC_RELAXED);
  __atomic_fetch_add(&statistics_.rx_bytes, static_cast<unsigned long long>(packet.size()), __ATOMIC_RELAXED);
  receiver_->receive(*this, packet);
}
bool InterfaceRegistry::add(NetworkInterface &interface)
{
  ConfigurationGuard guard;
  if (sealed_ || count_ == 8 || !interface.id() || !interface.name() ||
      interface.mtu() < 68 || interface.mtu() > 4082 || find(interface.id())) return false;
  for (unsigned i = 0; i < count_; ++i)
  {
    auto *a = interfaces_[i]->name(); auto *b = interface.name();
    while (*a && *a == *b) { ++a; ++b; }
    if (*a == *b) return false;
    for (unsigned n = 0; n < interface.address_count(); ++n)
      if (interfaces_[i]->owns(interface.address(n).address)) return false;
  }
  interfaces_[count_++] = &interface; return true;
}
bool InterfaceRegistry::seal()
{
  ConfigurationGuard guard;
  for (unsigned i = 0; i < count_; ++i)
    for (unsigned k = 0; k < count_; ++k)
      if (i != k)
        for (unsigned n = 0; n < interfaces_[i]->address_count(); ++n)
          if (interfaces_[k]->owns(interfaces_[i]->address(n).address)) return false;
  sealed_ = true;
  for (unsigned i = 0; i < count_; ++i) interfaces_[i]->sealed_ = true;
  return true;
}
NetworkInterface *InterfaceRegistry::find(unsigned id) const
{ ConfigurationGuard guard; for (unsigned i = 0; i < count_; ++i) if (interfaces_[i]->id() == id) return interfaces_[i]; return nullptr; }
bool InterfaceRegistry::owns(unsigned address) const
{ ConfigurationGuard guard; for (unsigned i = 0; i < count_; ++i) if (interfaces_[i]->up() && interfaces_[i]->owns(address)) return true; return false; }
void InterfaceRegistry::tick(unsigned long now)
{
  NetworkInterface *snapshot[8]; unsigned count;
  { ConfigurationGuard guard; count = count_; for (unsigned i = 0; i < count; ++i) snapshot[i] = interfaces_[i]; }
  for (unsigned i = 0; i < count; ++i) snapshot[i]->tick(now);
}
}

namespace xv::net
{
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
{ __atomic_fetch_add(&statistics_.rx_dropped, 1ULL, __ATOMIC_RELAXED); }
int NetworkInterface::send(Packet &packet, unsigned next_hop)
{
  unsigned length = packet.size();
  int result = transmit(packet, next_hop);
  return account_send(length,result);
}
int NetworkInterface::send_bootstrap(Packet &packet)
{
  unsigned length=packet.size();
  return account_send(length,transmit_bootstrap(packet));
}
int NetworkInterface::account_send(unsigned length,int result)
{
  if (result < 0) __atomic_fetch_add(&statistics_.tx_errors, 1ULL, __ATOMIC_RELAXED);
  else
  {
    __atomic_fetch_add(&statistics_.tx_packets, 1ULL, __ATOMIC_RELAXED);
    __atomic_fetch_add(&statistics_.tx_bytes, static_cast<unsigned long long>(length), __ATOMIC_RELAXED);
  }
  return result;
}
}
