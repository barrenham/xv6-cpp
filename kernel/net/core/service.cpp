module;
#include "kernel/net/control.h"
#include "kernel/net/filter/control.h"
module xv.net;
import :filter;
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
constinit Firewall firewall;
constinit Ipv4Layer ip(interface_registry, routes);
constinit LoopbackInterface loopback;
constinit EthernetInterface ethernet[] = {
    {2, "eth0"}, {3, "eth1"}, {4, "eth2"}, {5, "eth3"}};
constinit IcmpProtocol icmp(ip);
constinit UdpProtocol udp(ip, icmp);
constinit TcpProtocol tcp(ip);
constinit SocketRegistry sockets(ip, udp, tcp);
constinit IpInIpProtocol tunnels;
constinit BridgeManager bridges(interface_registry, routes, ip);
constinit LeaseConfiguration lease_configuration(interface_registry, routes, loopback);
constinit DhcpManager dhcp(interface_registry, ip, lease_configuration);
constinit NetworkManager manager(interface_registry, routes, loopback, &bridges, &dhcp);

bool configure_qemu_defaults()
{
#if !XV_NET_BOOT_DHCP
  for (unsigned i = 0; i < xv::drivers::network_device_count(); ++i)
  {
    auto &interface = ethernet[i];
    unsigned address = 0x0a00020f + (i << 8);
    unsigned network = address & 0xffffff00;
    if (!interface.add_address({address, 24}) ||
        !routes.add(address, 32, loopback, 0, 0, address, interface.id(), address) ||
        !routes.add(network, 24, interface, 0, 0, 0, interface.id(), address) ||
        !routes.add(0, 0, interface, network | 2, i * 10))
    {
      return false;
    }
  }
#endif
  return true;
}

void initialize_loopback()
{
  loopback.add_address({0x7f000001, 8});
  loopback.set_up(true);
  loopback.attach(ip);
  if (!interface_registry.add(loopback) || !routes.add(0x7f000000, 8, loopback))
  {
    detail::fail("loopback setup");
  }
}

void initialize_ethernet_interfaces()
{
  for (unsigned i = 0; i < xv::drivers::network_device_count(); ++i)
  {
    auto &interface = ethernet[i];
    xv::drivers::initialize_network(interface.receiver(), i);
    interface.initialize(xv::drivers::network_device(i));
    interface.set_up(true);
    interface.attach(ip);
    if (!interface_registry.add(interface))
    {
      detail::fail("interface registry full");
    }
  }
}

void attach_protocols()
{
  ip.attach(1, icmp);
  ip.attach(17, udp);
  ip.attach(6, tcp);
  ip.attach(4, tunnels);
  udp.attach(sockets);
  tcp.attach(sockets);
}

void start_default_dhcp()
{
#if XV_NET_BOOT_DHCP
  for (unsigned i = 0; i < xv::drivers::network_device_count(); ++i)
  {
    netctl_request request{};
    request.version = NETCTL_VERSION;
    request.size = sizeof(request);
    request.ifindex = ethernet[i].id();
    if (dhcp.execute(NETCTL_DHCP_START, request) < 0)
    {
      detail::fail("DHCP startup");
    }
  }
#endif
}
} // namespace

int firewall_control(unsigned operation, firewall_request &request)
{
  return firewall.execute(operation, request);
}

int control(unsigned operation, netctl_request &request)
{
  return manager.execute(operation, request);
}

const InterfaceRegistry &interfaces()
{
  return interface_registry;
}

const RoutingTable &routing_table()
{
  return routes;
}

SocketRegistry &socket_registry()
{
  return sockets;
}

void initialize(NetworkSetup *setup)
{
  firewall.initialize();
  ip.attach_filter(firewall);
  sockets.initialize();
  ip.initialize();
  loopback.initialize();
  dhcp.initialize();
  initialize_loopback();
  initialize_ethernet_interfaces();
  bridges.initialize();
  attach_protocols();

  bool configured = setup ? setup->configure(interface_registry, routes, ip, tunnels)
                          : configure_qemu_defaults();
  if (!configured || !interface_registry.seal())
  {
    detail::fail("invalid network setup");
  }
  routes.seal();
  if (!setup)
  {
    start_default_dhcp();
  }
}

void timer_tick()
{
  auto now = detail::milliseconds();
  interface_registry.tick(now);
  ip.tick(now);
  dhcp.tick(now);
  sockets.tick(now);
  interface_registry.tick(now);
}
} // namespace xv::net
