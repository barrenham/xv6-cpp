module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
#include "kernel/net/control.h"
module xv.net:control_detail_flows;
import :control;

namespace xv::net
{
namespace flow = xv::flow;

struct NetworkManager::InspectFlow
{
  struct Context
  {
    NetworkManager &owner;
    unsigned operation;
    netctl_request &request;
    unsigned id = 0, index = 0;
    netctl_request out{};
    RoutingTable::Route route{};
    NetworkInterface *interface = nullptr;
    int result = 0;
  };

  template <unsigned Operation> static bool operation(Context &c) noexcept
  {
    return c.operation == Operation;
  }

  static inline void initialize(Context &c) noexcept
  {
    c.id = c.request.ifindex;
    c.index = c.request.index;
    c.out.version = NETCTL_VERSION;
    c.out.size = sizeof(c.out);
    c.out.index = c.index;
  }

  static inline bool route(Context &c) noexcept
  {
    return c.owner.routes_.at(c.index, c.route);
  }

  static inline void describe_route(Context &c) noexcept
  {
    auto &out = c.out;
    const auto &r = c.route;
    out.ifindex = r.interface->id();
    out.address = r.network;
    out.prefix = r.prefix;
    out.gateway = r.gateway;
    out.metric = r.metric;
    out.source = r.source;
    out.flags = (r.address_owner ? NET_ROUTE_AUTOMATIC : 0) |
                (r.lease_owner ? NET_CONFIG_DHCP : 0);
  }

  static inline void find_interface(Context &c) noexcept
  {
    c.interface = c.operation == NETCTL_LINK ? (c.id ? c.owner.interfaces_.find(c.id)
                                                     : c.owner.interfaces_.at(c.index))
                                             : c.owner.interfaces_.find(c.id);
  }

  static inline bool found(Context &c) noexcept
  {
    return c.interface != nullptr;
  }

  static inline void identify(Context &c) noexcept
  {
    c.out.ifindex = c.interface->id();
  }

  static inline bool address(Context &c) noexcept
  {
    return c.index < c.interface->address_count();
  }

  static inline void describe_address(Context &c) noexcept
  {
    auto value = c.interface->address(c.index);
    c.out.address = value.address;
    c.out.prefix = value.prefix;
    c.out.flags = value.owner ? NET_CONFIG_DHCP : 0;
  }

  static inline void describe_link(Context &c) noexcept
  {
    auto &out = c.out;
    auto &interface = *c.interface;
    out.mtu = interface.mtu();
    out.address_count = interface.address_count();
    out.flags = (interface.up() ? NET_LINK_UP : 0) |
                (interface.carrier() ? NET_LINK_CARRIER : 0) |
                (interface.is_tunnel() ? NET_LINK_TUNNEL : 0) |
                (interface.is_bridge() ? NET_LINK_BRIDGE : 0) |
                (interface.master() ? NET_LINK_SLAVE : 0);
    out.source = interface.master();
    for (unsigned i = 0; i < sizeof(out.name) - 1 && interface.name()[i]; ++i)
    {
      out.name[i] = interface.name()[i];
    }
    if (auto *mac = interface.mac())
    {
      out.flags |= NET_LINK_MAC;
      for (unsigned i = 0; i < 6; ++i)
      {
        out.mac[i] = mac->bytes[i];
      }
    }
    auto stats = interface.statistics();
    out.rx_packets = stats.rx_packets;
    out.rx_bytes = stats.rx_bytes;
    out.rx_dropped = stats.rx_dropped;
    out.tx_packets = stats.tx_packets;
    out.tx_bytes = stats.tx_bytes;
    out.tx_errors = stats.tx_errors;
  }

  static inline void commit(Context &c) noexcept
  {
    c.request = c.out;
    c.result = 1;
  }

  static inline void missing(Context &c) noexcept
  {
    c.result = c.id ? -ENODEV : 0;
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -EINVAL;
  }
  struct Initialize;
  struct Route;
  struct DescribeRoute;
  struct Interface;
  struct Identify;
  struct Address;
  struct DescribeAddress;
  struct Link;
  struct DescribeLink;
  struct Commit;
  struct Missing;
  struct Invalid;
  using Graph = flow::graph<
      Initialize,
      flow::types<
          flow::node<Initialize, flow::action<&initialize>,
                     flow::branch<flow::condition<&operation<NETCTL_ROUTE>>,
                                  flow::go<Route>, flow::go<Interface>>>,
          flow::node<Route, flow::noop,
                     flow::branch<flow::condition<&route>, flow::go<DescribeRoute>,
                                  flow::stop>>,
          flow::node<DescribeRoute, flow::action<&describe_route>, flow::go<Commit>>,
          flow::node<Interface, flow::action<&find_interface>,
                     flow::branch<flow::condition<&found>, flow::go<Identify>,
                                  flow::go<Missing>>>,
          flow::node<Identify, flow::action<&identify>,
                     flow::branch<flow::condition<&operation<NETCTL_ADDRESS>>,
                                  flow::go<Address>, flow::go<Link>>>,
          flow::node<Address, flow::noop,
                     flow::branch<flow::condition<&address>, flow::go<DescribeAddress>,
                                  flow::stop>>,
          flow::node<DescribeAddress, flow::action<&describe_address>,
                     flow::go<Commit>>,
          flow::node<Link, flow::noop,
                     flow::branch<flow::condition<&operation<NETCTL_LINK>>,
                                  flow::go<DescribeLink>, flow::go<Invalid>>>,
          flow::node<DescribeLink, flow::action<&describe_link>, flow::go<Commit>>,
          flow::node<Commit, flow::action<&commit>, flow::stop>,
          flow::node<Missing, flow::action<&missing>, flow::stop>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>>>;
};

struct NetworkManager::MutationFlow
{
  struct Context
  {
    NetworkManager &owner;
    unsigned operation;
    const netctl_request &request;
    NetworkInterface &interface;
    int result = 0;
  };

  template <unsigned Operation> static bool operation(Context &c) noexcept
  {
    return c.operation == Operation;
  }

  static inline bool loopback(Context &c) noexcept
  {
    return &c.interface == &c.owner.loopback_;
  }

  static inline bool valid_link_flags(Context &c) noexcept
  {
    return !(c.request.flags & ~NET_LINK_UP);
  }

  static inline void link(Context &c) noexcept
  {
    c.interface.configure_link(bool(c.request.flags & NET_LINK_UP));
  }

  static inline void add_address(Context &c) noexcept
  {
    c.result = c.owner.addresses_.add_static(c.interface,
                                             {c.request.address, c.request.prefix});
  }

  static inline void remove_address(Context &c) noexcept
  {
    c.result = c.owner.addresses_.remove_static(c.interface,
                                                {c.request.address, c.request.prefix});
  }

  static inline bool route_operation(Context &c) noexcept
  {
    return c.operation == NETCTL_ADD_ROUTE || c.operation == NETCTL_DEL_ROUTE;
  }

  static inline bool valid_route(Context &c) noexcept
  {
    return c.request.prefix <= 32 && !c.request.flags;
  }

  static inline void add_route(Context &c) noexcept
  {
    const auto &r = c.request;
    c.result = c.owner.routes_.add_static(r.address, r.prefix, c.interface, r.gateway,
                                          r.metric, r.source);
  }

  static inline void remove_route(Context &c) noexcept
  {
    const auto &r = c.request;
    c.result = c.owner.routes_.remove_static(r.address, r.prefix, c.interface,
                                             r.gateway, r.metric, r.source);
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -EINVAL;
  }

  static inline void unsupported(Context &c) noexcept
  {
    c.result = -EOPNOTSUPP;
  }
  struct SetLink;
  struct Loopback;
  struct LinkFlags;
  struct Link;
  struct AddAddress;
  struct AddressAdd;
  struct RemoveAddress;
  struct AddressRemove;
  struct RouteOperation;
  struct RouteFlags;
  struct AddRoute;
  struct RouteAdd;
  struct RouteRemove;
  struct Invalid;
  struct Unsupported;
  using Graph = flow::graph<
      SetLink,
      flow::types<
          flow::node<SetLink, flow::noop,
                     flow::branch<flow::condition<&operation<NETCTL_SET_LINK>>,
                                  flow::go<Loopback>, flow::go<AddAddress>>>,
          flow::node<Loopback, flow::noop,
                     flow::branch<flow::condition<&loopback>, flow::go<Unsupported>,
                                  flow::go<LinkFlags>>>,
          flow::node<LinkFlags, flow::noop,
                     flow::branch<flow::condition<&valid_link_flags>, flow::go<Link>,
                                  flow::go<Invalid>>>,
          flow::node<Link, flow::action<&link>, flow::stop>,
          flow::node<AddAddress, flow::noop,
                     flow::branch<flow::condition<&operation<NETCTL_ADD_ADDRESS>>,
                                  flow::go<AddressAdd>, flow::go<RemoveAddress>>>,
          flow::node<AddressAdd, flow::action<&add_address>, flow::stop>,
          flow::node<RemoveAddress, flow::noop,
                     flow::branch<flow::condition<&operation<NETCTL_DEL_ADDRESS>>,
                                  flow::go<AddressRemove>, flow::go<RouteOperation>>>,
          flow::node<AddressRemove, flow::action<&remove_address>, flow::stop>,
          flow::node<RouteOperation, flow::noop,
                     flow::branch<flow::condition<&route_operation>,
                                  flow::go<RouteFlags>, flow::go<Invalid>>>,
          flow::node<RouteFlags, flow::noop,
                     flow::branch<flow::condition<&valid_route>, flow::go<AddRoute>,
                                  flow::go<Invalid>>>,
          flow::node<AddRoute, flow::noop,
                     flow::branch<flow::condition<&operation<NETCTL_ADD_ROUTE>>,
                                  flow::go<RouteAdd>, flow::go<RouteRemove>>>,
          flow::node<RouteAdd, flow::action<&add_route>, flow::stop>,
          flow::node<RouteRemove, flow::action<&remove_route>, flow::stop>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>,
          flow::node<Unsupported, flow::action<&unsupported>, flow::stop>>>;
};

} // namespace xv::net
