module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
#include "kernel/net/control.h"
module xv.net:control_execute_flow;
import :control;

namespace xv::net
{
namespace flow = xv::flow;

struct NetworkManager::ExecuteFlow
{
  struct Context
  {
    NetworkManager &owner;
    unsigned operation;
    netctl_request &request;
    NetworkInterface *interface = nullptr;
    bool complete = false;
    int result = 0;
  };

  static inline bool valid(Context &c) noexcept
  {
    return c.request.version == NETCTL_VERSION && c.request.size == sizeof(c.request);
  }

  static inline bool bridge(Context &c) noexcept
  {
    return c.operation >= NETCTL_BRIDGE_CREATE &&
           c.operation <= NETCTL_BRIDGE_FLUSH_FDB;
  }

  static inline bool dhcp(Context &c) noexcept
  {
    return c.operation >= NETCTL_DHCP_START && c.operation <= NETCTL_DHCP_RENEW;
  }

  static inline void bridge_command(Context &c) noexcept
  {
    c.result = c.owner.bridges_ ? c.owner.bridges_->execute(c.operation, c.request)
                                : -EOPNOTSUPP;
  }

  static inline void dhcp_command(Context &c) noexcept
  {
    c.result =
        c.owner.dhcp_ ? c.owner.dhcp_->execute(c.operation, c.request) : -EOPNOTSUPP;
  }

  static inline bool inspection(Context &c) noexcept
  {
    return c.operation == NETCTL_LINK || c.operation == NETCTL_ADDRESS ||
           c.operation == NETCTL_ROUTE;
  }

  static inline void inspect(Context &c) noexcept
  {
    c.result = c.owner.inspect(c.operation, c.request);
    c.complete = true;
  }

  static inline void find_interface(Context &c) noexcept
  {
    c.interface = c.owner.interfaces_.find(c.request.ifindex);
  }

  static inline bool found(Context &c) noexcept
  {
    return c.interface != nullptr;
  }

  static inline bool neighbor_operation(Context &c) noexcept
  {
    return c.operation == NETCTL_NEIGHBOR || c.operation == NETCTL_FLUSH_NEIGHBORS;
  }

  static inline void mutate(Context &c) noexcept
  {
    c.result = c.owner.mutate(c.operation, c.request, *c.interface);
  }

  static inline void missing(Context &c) noexcept
  {
    c.result = -ENODEV;
    c.complete = true;
  }
  struct Inspection;
  struct Inspect;
  struct Interface;
  struct NeighborOperation;
  struct Mutate;
  struct Missing;
  using LockedGraph = flow::graph<
      Inspection,
      flow::types<
          flow::node<Inspection, flow::noop,
                     flow::branch<flow::condition<&inspection>, flow::go<Inspect>,
                                  flow::go<Interface>>>,
          flow::node<Inspect, flow::action<&inspect>, flow::stop>,
          flow::node<Interface, flow::action<&find_interface>,
                     flow::branch<flow::condition<&found>, flow::go<NeighborOperation>,
                                  flow::go<Missing>>>,
          flow::node<NeighborOperation, flow::noop,
                     flow::branch<flow::condition<&neighbor_operation>, flow::stop,
                                  flow::go<Mutate>>>,
          flow::node<Mutate, flow::action<&mutate>, flow::stop>,
          flow::node<Missing, flow::action<&missing>, flow::stop>>>;

  static inline void configured(Context &c) noexcept
  {
    ConfigurationGuard guard;
    (void)LockedGraph::run<0>(c);
  }

  static inline bool complete(Context &c) noexcept
  {
    return c.complete;
  }

  static inline bool neighbor(Context &c) noexcept
  {
    return c.operation == NETCTL_NEIGHBOR;
  }

  static inline void inspect_neighbor(Context &c) noexcept
  {
    NeighborInfo neighbor;
    if (!c.interface->neighbor(c.request.index, neighbor))
    {
      c.result = 0;
      return;
    }
    netctl_request out{};
    out.version = NETCTL_VERSION;
    out.size = sizeof(out);
    out.ifindex = c.interface->id();
    out.index = c.request.index;
    out.address = neighbor.address;
    out.expires_ms = neighbor.expires_ms;
    for (unsigned i = 0; i < 6; ++i)
    {
      out.mac[i] = neighbor.mac[i];
    }
    c.request = out;
    c.result = 1;
  }

  static inline bool needs_invalidation(Context &c) noexcept
  {
    return c.result == 0 &&
           (c.operation == NETCTL_SET_LINK || c.operation == NETCTL_ADD_ADDRESS ||
            c.operation == NETCTL_DEL_ADDRESS || c.operation == NETCTL_FLUSH_NEIGHBORS);
  }

  static inline void invalidate(Context &c) noexcept
  {
    c.interface->invalidate();
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -EINVAL;
  }
  struct Version;
  struct Bridge;
  struct BridgeCommand;
  struct Dhcp;
  struct DhcpCommand;
  struct Configuration;
  struct Neighbor;
  struct InspectNeighbor;
  struct Invalidation;
  struct Invalidate;
  struct Invalid;
  using Graph = flow::graph<
      Version,
      flow::types<
          flow::node<Version, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<Bridge>,
                                  flow::go<Invalid>>>,
          flow::node<Bridge, flow::noop,
                     flow::branch<flow::condition<&bridge>, flow::go<BridgeCommand>,
                                  flow::go<Dhcp>>>,
          flow::node<BridgeCommand, flow::action<&bridge_command>, flow::stop>,
          flow::node<Dhcp, flow::noop,
                     flow::branch<flow::condition<&dhcp>, flow::go<DhcpCommand>,
                                  flow::go<Configuration>>>,
          flow::node<DhcpCommand, flow::action<&dhcp_command>, flow::stop>,
          flow::node<Configuration, flow::action<&configured>,
                     flow::branch<flow::condition<&complete>, flow::stop,
                                  flow::go<Neighbor>>>,
          flow::node<Neighbor, flow::noop,
                     flow::branch<flow::condition<&neighbor>, flow::go<InspectNeighbor>,
                                  flow::go<Invalidation>>>,
          flow::node<InspectNeighbor, flow::action<&inspect_neighbor>, flow::stop>,
          flow::node<Invalidation, flow::noop,
                     flow::branch<flow::condition<&needs_invalidation>,
                                  flow::go<Invalidate>, flow::stop>>,
          flow::node<Invalidate, flow::action<&invalidate>, flow::stop>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>>>;
};

} // namespace xv::net
