module;
#include "kernel/net/control.h"
#include "kernel/support/flow.h"
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
constinit SocketRegistry sockets(ip, udp, tcp, &icmp);
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
  icmp.attach(sockets);
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
namespace flow = xv::flow;

struct InitializationFlow
{
  struct Context
  {
    NetworkSetup *setup;
    bool configured = false;
  };

  static void services(Context &) noexcept
  {
    firewall.initialize();
    ip.attach_filter(firewall);
    sockets.initialize();
    ip.initialize();
    loopback.initialize();
    dhcp.initialize();
  }

  static void local_interface(Context &) noexcept
  {
    initialize_loopback();
  }

  static void ethernet_interfaces(Context &) noexcept
  {
    initialize_ethernet_interfaces();
  }

  static void bridge_interfaces(Context &) noexcept
  {
    bridges.initialize();
  }

  static void protocols(Context &) noexcept
  {
    attach_protocols();
  }

  static void configure(Context &c) noexcept
  {
    c.configured = c.setup ? c.setup->configure(interface_registry, routes, ip, tunnels)
                           : configure_qemu_defaults();
  }

  static bool configured(Context &c) noexcept
  {
    return c.configured;
  }

  static bool seal_interfaces(Context &) noexcept
  {
    return interface_registry.seal();
  }

  static void seal_routes(Context &) noexcept
  {
    routes.seal();
  }

  static bool default_setup(Context &c) noexcept
  {
    return !c.setup;
  }

  static void start_dhcp(Context &) noexcept
  {
    start_default_dhcp();
  }

  static void invalid(Context &) noexcept
  {
    detail::fail("invalid network setup");
  }
  struct Services;
  struct Loopback;
  struct Ethernet;
  struct Bridges;
  struct Protocols;
  struct Configure;
  struct SealInterfaces;
  struct SealRoutes;
  struct Dhcp;
  struct Invalid;
  using Graph = flow::graph<
      Services,
      flow::types<
          flow::node<Services, flow::action<&services>, flow::go<Loopback>>,
          flow::node<Loopback, flow::action<&local_interface>, flow::go<Ethernet>>,
          flow::node<Ethernet, flow::action<&ethernet_interfaces>, flow::go<Bridges>>,
          flow::node<Bridges, flow::action<&bridge_interfaces>, flow::go<Protocols>>,
          flow::node<Protocols, flow::action<&protocols>, flow::go<Configure>>,
          flow::node<Configure, flow::action<&configure>,
                     flow::branch<flow::condition<&configured>,
                                  flow::go<SealInterfaces>, flow::go<Invalid>>>,
          flow::node<SealInterfaces, flow::noop,
                     flow::branch<flow::condition<&seal_interfaces>,
                                  flow::go<SealRoutes>, flow::go<Invalid>>>,
          flow::node<SealRoutes, flow::action<&seal_routes>,
                     flow::branch<flow::condition<&default_setup>, flow::go<Dhcp>,
                                  flow::stop>>,
          flow::node<Dhcp, flow::action<&start_dhcp>, flow::stop>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>>>;
};

struct TimerFlow
{
  struct Context
  {
    unsigned long now;
  };

  static void interfaces(Context &c) noexcept
  {
    interface_registry.tick(c.now);
  }

  static void fragments(Context &c) noexcept
  {
    ip.tick(c.now);
  }

  static void leases(Context &c) noexcept
  {
    dhcp.tick(c.now);
  }

  static void connections(Context &c) noexcept
  {
    sockets.tick(c.now);
  }
  struct FirstInterfaces;
  struct Fragments;
  struct Leases;
  struct Connections;
  struct LastInterfaces;
  using Graph = flow::graph<
      FirstInterfaces,
      flow::types<
          flow::node<FirstInterfaces, flow::action<&interfaces>, flow::go<Fragments>>,
          flow::node<Fragments, flow::action<&fragments>, flow::go<Leases>>,
          flow::node<Leases, flow::action<&leases>, flow::go<Connections>>,
          flow::node<Connections, flow::action<&connections>, flow::go<LastInterfaces>>,
          flow::node<LastInterfaces, flow::action<&interfaces>, flow::stop>>>;
};
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
  InitializationFlow::Context ctx{setup};
  (void)InitializationFlow::Graph::run<0>(ctx);
}

void timer_tick()
{
  TimerFlow::Context ctx{detail::milliseconds()};
  (void)TimerFlow::Graph::run<0>(ctx);
}
} // namespace xv::net
