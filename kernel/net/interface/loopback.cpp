module;
#include "kernel/net/errno.h"
module xv.net;
import :platform;
import :loopback_flows;
import :loopback;

namespace xv::net
{
void LoopbackInterface::initialize()
{
  lock_.init("loopback packets");
}

bool LoopbackInterface::owns(unsigned value) const
{
  ConfigurationGuard guard;
  for (unsigned i = 0; i < address_count(); ++i)
  {
    if (address(i).contains(value))
    {
      return true;
    }
  }
  return false;
}

int LoopbackInterface::transmit(Packet &packet, unsigned)
{
  detail::TimerChange changed;
  TransmitFlow::Context ctx{*this, packet};
  (void)TransmitFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void LoopbackInterface::tick(unsigned long)
{
  DrainFlow::Context ctx{*this};
  (void)DrainFlow::Graph::run<0>(ctx);
}
} // namespace xv::net

namespace xv::net
{
unsigned long LoopbackInterface::next_deadline()
{
  xv::sync::ScopedLock guard(lock_);
  return count_ ? 0 : ~0UL;
}
} // namespace xv::net
