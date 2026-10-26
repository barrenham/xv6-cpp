module;
#include "kernel/net/control.h"
module xv.net;
import :udp;
import :registry;
import :tcp;
import :ethernet_interface;
import :loopback;
import :tunnel;
import :control;
namespace xv::net
{
namespace
{
constinit InterfaceRegistry interface_registry;
constinit RoutingTable routes(interface_registry);
constinit Ipv4Layer ip(interface_registry, routes);
constinit LoopbackInterface loopback;
constinit EthernetInterface ethernet[] = {{2, "eth0"}, {3, "eth1"}, {4, "eth2"}, {5, "eth3"}};
constinit IcmpProtocol icmp(ip);
constinit UdpProtocol udp(ip, icmp);
constinit SocketRegistry sockets(ip, udp);
constinit TcpProtocol tcp(sockets);
constinit IpInIpProtocol tunnels;
constinit BridgeManager bridges(interface_registry, routes, ip);
constinit LeaseConfiguration lease_configuration(interface_registry, routes, loopback);
constinit DhcpManager dhcp(interface_registry, ip, lease_configuration);
constinit NetworkManager manager(interface_registry, routes, loopback, &bridges, &dhcp);
bool qemu_defaults()
{
#if !XV_NET_BOOT_DHCP
  for (unsigned i = 0; i < xv::drivers::network_device_count(); ++i)
  {
    auto &interface = ethernet[i]; unsigned address = 0x0a00020f + (i<<8);
    if (!interface.add_address({address, 24}) ||
        !routes.add(address, 32, loopback, 0, 0, address, interface.id(), address) ||
        !routes.add(address & 0xffffff00, 24, interface, 0, 0, 0, interface.id(), address) ||
        !routes.add(0, 0, interface, (address & 0xffffff00)|2, i*10)) return false;
  }
  return true;
#else
  return true;
#endif
}
}
int control(unsigned operation, netctl_request &request) { return manager.execute(operation, request); }
const InterfaceRegistry &interfaces() { return interface_registry; }
const RoutingTable &routing_table() { return routes; }
SocketRegistry &socket_registry() { return sockets; }
TcpProtocol &tcp_protocol() { return tcp; }
void initialize(NetworkSetup *setup)
{
  sockets.initialize(); ip.initialize(); loopback.initialize(); dhcp.initialize();
  loopback.add_address({0x7f000001, 8}); loopback.set_up(true); loopback.attach(ip);
  if (!interface_registry.add(loopback) || !routes.add(0x7f000000, 8, loopback)) detail::fail("loopback setup");
  for (unsigned i = 0; i < xv::drivers::network_device_count(); ++i)
  {
    auto &interface = ethernet[i];
    xv::drivers::initialize_network(interface.receiver(), i);
    interface.initialize(xv::drivers::network_device(i));
    interface.set_up(true); interface.attach(ip);
    if (!interface_registry.add(interface)) detail::fail("interface registry full");
  }
  bridges.initialize();
  ip.attach(1, icmp); ip.attach(17, udp); ip.attach(6, tcp); ip.attach(4, tunnels);
  udp.attach(sockets);
  if (!(setup ? setup->configure(interface_registry, routes, ip, tunnels) : qemu_defaults()) ||
      !interface_registry.seal()) detail::fail("invalid network setup");
  routes.seal();
#if XV_NET_BOOT_DHCP
  if (!setup)
    for (unsigned i = 0; i < xv::drivers::network_device_count(); ++i)
    {
      netctl_request request{}; request.version=NETCTL_VERSION; request.size=sizeof(request); request.ifindex=ethernet[i].id();
      if (dhcp.execute(NETCTL_DHCP_START,request)<0) detail::fail("DHCP startup");
    }
#endif
}
void timer_tick()
{
  auto now = detail::milliseconds();
  interface_registry.tick(now); ip.tick(now); dhcp.tick(now); sockets.tick(now); interface_registry.tick(now);
}
}
