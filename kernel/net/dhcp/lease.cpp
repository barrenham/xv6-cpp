module;
#include "kernel/net/errno.h"
module xv.net;
import :lease;

namespace xv::net
{
int LeaseConfiguration::install(NetworkInterface &interface, const DhcpLease &lease,
                                bool &changed)
{
  changed = false;
  if (!lease.address || lease.prefix > 32 || !lease.seconds)
  {
    return -EINVAL;
  }
  return addresses_.install_lease(interface, {lease.address, lease.prefix},
                                  lease.gateway, 100 + interface.id(), changed);
}

bool LeaseConfiguration::remove(NetworkInterface &interface)
{
  return addresses_.remove_lease(interface);
}
} // namespace xv::net
