module xv.net;
import :interface;
namespace xv::net
{
bool NetworkInterface::owns(unsigned value) const
{ for (unsigned i = 0; i < address_count_; ++i) if (addresses_[i].address == value) return true; return false; }
bool NetworkInterface::accepts_destination(unsigned address, const InterfaceRegistry &) const
{ return owns(address) || broadcast(address); }
bool NetworkInterface::broadcast(unsigned value) const
{
  if (value == 0xffffffff) return address_count_ != 0;
  for (unsigned i = 0; i < address_count_; ++i)
    if (addresses_[i].prefix < 31 && value == (addresses_[i].address | ~addresses_[i].mask())) return true;
  return false;
}
unsigned NetworkInterface::source_for(unsigned destination) const
{
  const Ipv4Address *best = nullptr;
  for (unsigned i = 0; i < address_count_; ++i)
    if (!best || (addresses_[i].contains(destination) &&
        (!best->contains(destination) || addresses_[i].prefix > best->prefix))) best = &addresses_[i];
  return best ? best->address : 0;
}
bool NetworkInterface::add_address(Ipv4Address value)
{
  if (sealed_ || address_count_ == 4 || !value.address || (value.address >> 28) >= 14 ||
      value.prefix > 32 || owns(value.address)) return false;
  addresses_[address_count_++] = value; return true;
}
bool NetworkInterface::set_up(bool value)
{ if (sealed_) return false; up_ = value; return true; }
void NetworkInterface::deliver(Packet &packet)
{ if (up_ && receiver_) receiver_->receive(*this, packet); }
bool InterfaceRegistry::add(NetworkInterface &interface)
{
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
{ for (unsigned i = 0; i < count_; ++i) if (interfaces_[i]->id() == id) return interfaces_[i]; return nullptr; }
bool InterfaceRegistry::owns(unsigned address) const
{ for (unsigned i = 0; i < count_; ++i) if (interfaces_[i]->up() && interfaces_[i]->owns(address)) return true; return false; }
void InterfaceRegistry::tick(unsigned long now)
{ for (unsigned i = 0; i < count_; ++i) interfaces_[i]->tick(now); }
}
