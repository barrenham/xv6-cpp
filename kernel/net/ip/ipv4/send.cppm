module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:ipv4_send_flow;
import :ipv4;

namespace xv::net
{
struct Ipv4Layer::SendFlow
{
  struct Context
  {
    Ipv4Layer &owner;
    Packet &packet;
    unsigned destination, protocol, source;
    const NetworkInterface *via;
    NetworkInterface *bootstrap = nullptr;
    RouteSelection selection{};
    int result = 0;
  };

  static inline bool select_route(Context &c) noexcept
  {
    return c.owner.routes_.lookup(c.destination, c.selection, c.source, c.via);
  }

  static inline bool valid_packet(Context &c) noexcept
  {
    return c.packet && c.packet.size() >= 20 &&
           c.packet.size() <= c.selection.interface->mtu();
  }

  static inline bool valid_bootstrap_packet(Context &c) noexcept
  {
    return c.packet && c.packet.size() >= 28 && c.packet.size() <= c.bootstrap->mtu();
  }

  static inline bool valid_bootstrap_interface(Context &c) noexcept
  {
    return c.owner.interfaces_.find(c.bootstrap->id()) == c.bootstrap &&
           c.bootstrap->dhcp_active() && c.bootstrap->up() && !c.bootstrap->master() &&
           (!c.source || c.bootstrap->owns(c.source));
  }

  static inline bool valid_bootstrap_ports(Context &c) noexcept
  {
    return wire::read16(c.packet.data() + 20) == 68 &&
           wire::read16(c.packet.data() + 22) == 67;
  }

  static inline void write_header(Context &c) noexcept
  {
    c.owner.write_header(c.packet, c.selection.source, c.destination, c.protocol);
  }

  static inline void write_bootstrap_header(Context &c) noexcept
  {
    c.owner.write_header(c.packet, c.source, 0xffffffff, 17);
  }

  static inline bool allowed(Context &c) noexcept
  {
    return c.owner.output_allowed(c.packet, *c.selection.interface);
  }

  static inline bool bootstrap_allowed(Context &c) noexcept
  {
    return c.owner.output_allowed(c.packet, *c.bootstrap);
  }

  static inline void send(Context &c) noexcept
  {
    c.result = c.selection.interface->send(c.packet, c.selection.next_hop);
  }

  static inline void send_bootstrap(Context &c) noexcept
  {
    c.result = c.bootstrap->send_bootstrap(c.packet);
  }

  static inline void unreachable(Context &c) noexcept
  {
    c.result = -ENETUNREACH;
  }

  static inline void too_large(Context &c) noexcept
  {
    c.result = -EMSGSIZE;
  }

  static inline void forbidden(Context &c) noexcept
  {
    c.result = -EACCES;
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -EINVAL;
  }
  struct Route;
  struct PacketSize;
  struct Header;
  struct Filter;
  struct Send;
  struct Unreachable;
  struct TooLarge;
  struct Forbidden;
  struct Invalid;
  struct BootstrapSize;
  struct BootstrapInterface;
  struct BootstrapPorts;
  struct BootstrapHeader;
  struct BootstrapFilter;
  struct BootstrapSend;
  using Errors = xv::flow::types<
      xv::flow::node<Unreachable, xv::flow::action<&unreachable>, xv::flow::stop>,
      xv::flow::node<TooLarge, xv::flow::action<&too_large>, xv::flow::stop>,
      xv::flow::node<Forbidden, xv::flow::action<&forbidden>, xv::flow::stop>,
      xv::flow::node<Invalid, xv::flow::action<&invalid>, xv::flow::stop>>;
  using Graph = xv::flow::graph<
      Route, xv::flow::concat<
                 Errors,
                 xv::flow::types<
                     xv::flow::node<Route, xv::flow::noop,
                                    xv::flow::branch<xv::flow::condition<&select_route>,
                                                     xv::flow::go<PacketSize>,
                                                     xv::flow::go<Unreachable>>>,
                     xv::flow::node<PacketSize, xv::flow::noop,
                                    xv::flow::branch<xv::flow::condition<&valid_packet>,
                                                     xv::flow::go<Header>,
                                                     xv::flow::go<TooLarge>>>,
                     xv::flow::node<Header, xv::flow::action<&write_header>,
                                    xv::flow::go<Filter>>,
                     xv::flow::node<
                         Filter, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&allowed>,
                                          xv::flow::go<Send>, xv::flow::go<Forbidden>>>,
                     xv::flow::node<Send, xv::flow::action<&send>, xv::flow::stop>>>>;
  using BootstrapGraph = xv::flow::graph<
      BootstrapSize,
      xv::flow::concat<
          Errors,
          xv::flow::types<
              xv::flow::
                  node<BootstrapSize, xv::flow::noop,
                       xv::flow::branch<xv::flow::condition<&valid_bootstrap_packet>,
                                        xv::flow::go<BootstrapInterface>,
                                        xv::flow::go<TooLarge>>>,
              xv::flow::node<
                  BootstrapInterface, xv::flow::noop,
                  xv::flow::branch<xv::flow::condition<&valid_bootstrap_interface>,
                                   xv::flow::go<BootstrapPorts>,
                                   xv::flow::go<Unreachable>>>,
              xv::flow::node<BootstrapPorts, xv::flow::noop,
                             xv::flow::branch<
                                 xv::flow::condition<&valid_bootstrap_ports>,
                                 xv::flow::go<BootstrapHeader>, xv::flow::go<Invalid>>>,
              xv::flow::node<BootstrapHeader, xv::flow::action<&write_bootstrap_header>,
                             xv::flow::go<BootstrapFilter>>,
              xv::flow::node<BootstrapFilter, xv::flow::noop,
                             xv::flow::branch<xv::flow::condition<&bootstrap_allowed>,
                                              xv::flow::go<BootstrapSend>,
                                              xv::flow::go<Forbidden>>>,
              xv::flow::node<BootstrapSend, xv::flow::action<&send_bootstrap>,
                             xv::flow::stop>>>>;
};
} // namespace xv::net
