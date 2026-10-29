module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:routing_lookup_flow;
import :routing;

namespace xv::net
{
namespace flow = xv::flow;

struct RoutingTable::LookupFlow
{
  struct Context
  {
    const RoutingTable &owner;
    unsigned destination;
    RouteSelection &result;
    unsigned source;
    const NetworkInterface *via;
    unsigned index = 0, selected_source = 0;
    const Route *best = nullptr;
    bool found = false;
  };

  static inline void reset(Context &c) noexcept
  {
    c.result = {};
  }

  static inline bool valid_destination(Context &c) noexcept
  {
    return c.destination && ((c.destination >> 28) < 14 || c.destination == 0xffffffff);
  }

  static inline bool more(Context &c) noexcept
  {
    return c.index < c.owner.count_;
  }

  static inline bool source_owned(Context &c) noexcept
  {
    const auto &r = c.owner.routes_[c.index];
    return !r.source || c.owner.interfaces_.owns(r.source);
  }

  static inline bool gateway_connected(Context &c) noexcept
  {
    const auto &r = c.owner.routes_[c.index];
    if (!r.gateway)
    {
      return true;
    }
    auto &interface = *r.interface;
    bool connected = false;
    for (unsigned i = 0; i < interface.address_count(); ++i)
    {
      if (interface.address(i).contains(r.gateway))
      {
        connected = true;
      }
    }
    return connected && !interface.owns(r.gateway) && !interface.broadcast(r.gateway);
  }

  static inline bool matches_path(Context &c) noexcept
  {
    const auto &r = c.owner.routes_[c.index];
    auto &interface = *r.interface;
    unsigned mask = Ipv4Address{0, r.prefix}.mask();
    return !interface.master() && interface.up() && interface.carrier() &&
           (!c.via || c.via == &interface) && (c.destination & mask) == r.network &&
           (!c.source || interface.owns(c.source) || c.source == r.source);
  }

  static inline bool available_source(Context &c) noexcept
  {
    const auto &r = c.owner.routes_[c.index];
    return c.source ? c.source
                    : (r.source ? r.source : r.interface->source_for(c.destination));
  }

  static inline void consider(Context &c) noexcept
  {
    const auto &r = c.owner.routes_[c.index];
    if (!c.best || r.prefix > c.best->prefix ||
        (r.prefix == c.best->prefix && r.metric < c.best->metric))
    {
      c.best = &r;
    }
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }

  static inline bool best(Context &c) noexcept
  {
    return c.best != nullptr;
  }

  static inline void select_source(Context &c) noexcept
  {
    c.selected_source =
        c.source ? c.source
                 : (c.best->source ? c.best->source
                                   : c.best->interface->source_for(c.destination));
  }

  static inline bool source(Context &c) noexcept
  {
    return c.selected_source != 0;
  }

  static inline void commit(Context &c) noexcept
  {
    auto &interface = *c.best->interface;
    unsigned next_hop = interface.broadcast(c.destination)
                            ? 0xffffffffU
                            : (c.best->gateway ? c.best->gateway : c.destination);
    c.result = {&interface, next_hop, c.selected_source};
    c.found = true;
  }
  struct Destination;
  struct Scan;
  struct SourceOwned;
  struct Gateway;
  struct Path;
  struct SourceAvailable;
  struct Consider;
  struct Advance;
  struct Best;
  struct SelectSource;
  struct Commit;
  using Graph = flow::graph<
      Destination,
      flow::types<
          flow::node<Destination, flow::action<&reset>,
                     flow::branch<flow::condition<&valid_destination>, flow::go<Scan>,
                                  flow::stop>>,
          flow::node<Scan, flow::noop,
                     flow::branch<flow::condition<&more>, flow::go<SourceOwned>,
                                  flow::go<Best>>>,
          flow::node<SourceOwned, flow::noop,
                     flow::branch<flow::condition<&source_owned>, flow::go<Gateway>,
                                  flow::go<Advance>>>,
          flow::node<Gateway, flow::noop,
                     flow::branch<flow::condition<&gateway_connected>, flow::go<Path>,
                                  flow::go<Advance>>>,
          flow::node<Path, flow::noop,
                     flow::branch<flow::condition<&matches_path>,
                                  flow::go<SourceAvailable>, flow::go<Advance>>>,
          flow::node<SourceAvailable, flow::noop,
                     flow::branch<flow::condition<&available_source>,
                                  flow::go<Consider>, flow::go<Advance>>>,
          flow::node<Consider, flow::action<&consider>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
          flow::node<
              Best, flow::noop,
              flow::branch<flow::condition<&best>, flow::go<SelectSource>, flow::stop>>,
          flow::node<
              SelectSource, flow::action<&select_source>,
              flow::branch<flow::condition<&source>, flow::go<Commit>, flow::stop>>,
          flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

} // namespace xv::net
