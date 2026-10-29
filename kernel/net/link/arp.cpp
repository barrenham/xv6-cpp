module;
#include "kernel/net/errno.h"
module xv.net;
import :arp_request_flow;
import :arp_flows;
import :arp;
import :wire;
import :platform;

namespace xv::net
{
void ArpCache::initialize()
{
  lock_.init("ARP neighbors");
}

void ArpCache::request(unsigned address)
{
  RequestFlow::Context ctx{*this, address};
  (void)RequestFlow::Graph::run<0>(ctx);
}

int ArpCache::send(Packet &packet, unsigned next_hop)
{
  SendFlow::Context ctx{*this, packet, next_hop};
  (void)SendFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void ArpCache::receive(Packet &packet)
{
  ReceiveFlow::Context ctx{*this, packet};
  (void)ReceiveFlow::Graph::run<0>(ctx);
}

void ArpCache::tick(unsigned long now)
{
  TickFlow::Context ctx{*this, now};
  (void)TickFlow::Graph::run<0>(ctx);
}
} // namespace xv::net

namespace xv::net
{
bool ArpCache::neighbor(unsigned index, NeighborInfo &result)
{
  xv::sync::ScopedLock guard(lock_);
  auto now = detail::milliseconds();
  for (const auto &entry : entries_)
  {
    if (entry.address && entry.expires > now)
    {
      if (index--)
      {
        continue;
      }
      result.address = entry.address;
      result.expires_ms = entry.expires - now;
      for (unsigned i = 0; i < 6; ++i)
      {
        result.mac[i] = entry.mac[i];
      }
      return true;
    }
  }
  return false;
}

void ArpCache::clear()
{
  xv::sync::ScopedLock guard(lock_);
  for (auto &entry : entries_)
  {
    entry = {};
  }
  for (auto &pending : pending_)
  {
    Packet discarded(pending.page, pending.length);
    pending = {};
  }
}
} // namespace xv::net
