module;
#include "kernel/net/errno.h"
module xv.net;
import :address_remove_flow;
import :address_flows;
import :address_configuration;

namespace xv::net
{
int AddressConfiguration::add_static(NetworkInterface &interface, Ipv4Address address)
{
  ConfigurationGuard guard;
  StaticFlow::Context ctx{*this, interface, address};
  (void)StaticFlow::AddGraph::run<0>(ctx);
  return ctx.result;
}

int AddressConfiguration::remove_static(NetworkInterface &interface,
                                        Ipv4Address address)
{
  ConfigurationGuard guard;
  StaticFlow::Context ctx{*this, interface, address};
  (void)StaticFlow::RemoveGraph::run<0>(ctx);
  return ctx.result;
}

int AddressConfiguration::install_lease(NetworkInterface &interface,
                                        Ipv4Address address, unsigned gateway,
                                        unsigned metric, bool &changed)
{
  ConfigurationGuard guard;
  LeaseFlow::Context ctx{*this, interface, address, gateway, metric, changed};
  (void)LeaseFlow::Graph::run<0>(ctx);
  return ctx.result;
}

bool AddressConfiguration::remove_lease(NetworkInterface &interface)
{
  ConfigurationGuard guard;
  RemoveLeaseFlow::Context ctx{*this, interface};
  (void)RemoveLeaseFlow::Graph::run<0>(ctx);
  return ctx.changed;
}
} // namespace xv::net
