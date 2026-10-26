module;
#include "kernel/net/errno.h"
module xv.net;
import :loopback;
namespace xv::net
{
void LoopbackInterface::initialize() { lock_.init("loopback packets"); }
bool LoopbackInterface::owns(unsigned value) const
{ ConfigurationGuard guard; for (unsigned i = 0; i < address_count(); ++i) if (address(i).contains(value)) return true; return false; }
int LoopbackInterface::transmit(Packet &packet, unsigned)
{
  if (!up()) return -ENETUNREACH;
  if (packet.size() > mtu()) return -EMSGSIZE;
  xv::sync::ScopedLock guard(lock_);
  if (count_ == 16) return -ENOBUFS;
  packets_[(head_+count_)%16] = {packet.release(), packet.size()}; ++count_; return 0;
}
void LoopbackInterface::tick(unsigned long)
{
  for (unsigned budget = 0; budget < 64; ++budget)
  {
    lock_.acquire(); if (!count_) { lock_.release(); return; }
    auto pending = packets_[head_]; packets_[head_] = {}; head_ = (head_+1)%16; --count_;
    lock_.release(); Packet packet(pending.page, pending.length); deliver(packet);
  }
}
}
