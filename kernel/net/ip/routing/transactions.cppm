module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:routing_transaction_flows;
import :routing;

namespace xv::net
{
namespace flow = xv::flow;

struct RoutingTable::ValidationFlow
{
  struct Context
  {
    const RoutingTable &owner;
    const Route &route;
    bool connected = false, result = false;
  };

  static inline bool attributes(Context &c) noexcept
  {
    const auto &r = c.route;
    auto &interface = *r.interface;
    return !interface.master() && r.prefix <= 32 &&
           c.owner.interfaces_.find(interface.id()) == &interface &&
           (!r.gateway || interface.source_for(r.gateway)) &&
           (!r.source || (c.owner.interfaces_.owns(r.source) &&
                          (!interface.mac() || interface.owns(r.source))));
  }

  static inline bool gateway(Context &c) noexcept
  {
    return c.route.gateway != 0;
  }

  static inline void connectivity(Context &c) noexcept
  {
    auto &interface = *c.route.interface;
    for (unsigned i = 0; i < interface.address_count(); ++i)
    {
      if (interface.address(i).contains(c.route.gateway))
      {
        c.connected = true;
      }
    }
  }

  static inline bool valid_gateway(Context &c) noexcept
  {
    return c.connected && !c.route.interface->owns(c.route.gateway) &&
           !c.route.interface->broadcast(c.route.gateway);
  }

  static inline void success(Context &c) noexcept
  {
    c.result = true;
  }
  struct Attributes;
  struct Gateway;
  struct Connectivity;
  struct Success;
  using Graph = flow::graph<
      Attributes,
      flow::types<flow::node<Attributes, flow::noop,
                             flow::branch<flow::condition<&attributes>,
                                          flow::go<Gateway>, flow::stop>>,
                  flow::node<Gateway, flow::noop,
                             flow::branch<flow::condition<&gateway>,
                                          flow::go<Connectivity>, flow::go<Success>>>,
                  flow::node<Connectivity, flow::action<&connectivity>,
                             flow::branch<flow::condition<&valid_gateway>,
                                          flow::go<Success>, flow::stop>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct RoutingTable::MutationFlow
{
  struct Context
  {
    RoutingTable &owner;
    unsigned network, prefix;
    NetworkInterface &interface;
    unsigned gateway, metric, source;
    Route route{};
    int index = -1, result = 0;
  };

  static inline bool valid_prefix(Context &c) noexcept
  {
    return c.prefix <= 32;
  }

  static inline void describe(Context &c) noexcept
  {
    c.route = {c.network & Ipv4Address{0, c.prefix}.mask(),
               c.prefix,
               c.gateway,
               c.metric,
               c.source,
               &c.interface};
  }

  static inline void lookup(Context &c) noexcept
  {
    c.index = c.owner.find(c.route);
  }

  static inline bool found(Context &c) noexcept
  {
    return c.index >= 0;
  }

  static inline bool capacity(Context &c) noexcept
  {
    return c.owner.count_ != RoutingTable::Capacity;
  }

  static inline bool valid(Context &c) noexcept
  {
    return c.owner.valid(c.route);
  }

  static inline void append(Context &c) noexcept
  {
    c.owner.routes_[c.owner.count_++] = c.route;
  }

  static inline bool managed(Context &c) noexcept
  {
    return c.owner.routes_[c.index].address_owner ||
           c.owner.routes_[c.index].lease_owner;
  }

  static inline void erase(Context &c) noexcept
  {
    c.owner.erase(c.index);
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -EINVAL;
  }

  static inline void exists(Context &c) noexcept
  {
    c.result = -EEXIST;
  }

  static inline void full(Context &c) noexcept
  {
    c.result = -ENOBUFS;
  }

  static inline void missing(Context &c) noexcept
  {
    c.result = -ENOENT;
  }

  static inline void busy(Context &c) noexcept
  {
    c.result = -EBUSY;
  }
  struct Prefix;
  struct Describe;
  struct Lookup;
  struct Capacity;
  struct Valid;
  struct Append;
  struct Invalid;
  struct Exists;
  struct Full;
  using AddGraph = flow::graph<
      Prefix,
      flow::types<flow::node<Prefix, flow::noop,
                             flow::branch<flow::condition<&valid_prefix>,
                                          flow::go<Describe>, flow::go<Invalid>>>,
                  flow::node<Describe, flow::action<&describe>, flow::go<Lookup>>,
                  flow::node<Lookup, flow::action<&lookup>,
                             flow::branch<flow::condition<&found>, flow::go<Exists>,
                                          flow::go<Capacity>>>,
                  flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&capacity>, flow::go<Valid>,
                                          flow::go<Full>>>,
                  flow::node<Valid, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Append>,
                                          flow::go<Invalid>>>,
                  flow::node<Append, flow::action<&append>, flow::stop>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Exists, flow::action<&exists>, flow::stop>,
                  flow::node<Full, flow::action<&full>, flow::stop>>>;

  struct RemovePrefix;
  struct RemoveDescribe;
  struct RemoveLookup;
  struct Managed;
  struct Erase;
  struct RemoveInvalid;
  struct Missing;
  struct Busy;
  using RemoveGraph = flow::graph<
      RemovePrefix,
      flow::types<
          flow::node<RemovePrefix, flow::noop,
                     flow::branch<flow::condition<&valid_prefix>,
                                  flow::go<RemoveDescribe>, flow::go<RemoveInvalid>>>,
          flow::node<RemoveDescribe, flow::action<&describe>, flow::go<RemoveLookup>>,
          flow::node<RemoveLookup, flow::action<&lookup>,
                     flow::branch<flow::condition<&found>, flow::go<Managed>,
                                  flow::go<Missing>>>,
          flow::node<
              Managed, flow::noop,
              flow::branch<flow::condition<&managed>, flow::go<Busy>, flow::go<Erase>>>,
          flow::node<Erase, flow::action<&erase>, flow::stop>,
          flow::node<RemoveInvalid, flow::action<&invalid>, flow::stop>,
          flow::node<Missing, flow::action<&missing>, flow::stop>,
          flow::node<Busy, flow::action<&busy>, flow::stop>>>;
};

struct RoutingTable::LeaseFlow
{
  struct Context
  {
    RoutingTable &owner;
    NetworkInterface &interface, &loopback;
    Ipv4Address address;
    unsigned gateway, metric;
    RouteSet desired{};
    unsigned index = 0;
    bool result = false;
  };

  struct MatchContext
  {
    const RoutingTable &owner;
    NetworkInterface &interface, &loopback;
    Ipv4Address address;
    unsigned gateway, metric;
    RouteSet desired{};
    unsigned index = 0;
    bool result = false;
  };

  template <class State> static inline bool valid_prefix(State &c) noexcept
  {
    return c.address.prefix <= 32;
  }

  template <class State> static inline void describe(State &c) noexcept
  {
    c.desired = c.owner.address_routes(c.interface, c.loopback, c.address,
                                       c.interface.id(), c.gateway, c.metric);
  }

  static inline bool same_count(MatchContext &c) noexcept
  {
    return c.owner.lease_route_count(c.interface.id()) == c.desired.count;
  }

  static inline bool more(MatchContext &c) noexcept
  {
    return c.index < c.desired.count;
  }

  static inline bool matched(MatchContext &c) noexcept
  {
    bool found = false;
    for (unsigned i = 0; i < c.owner.count_; ++i)
    {
      if (c.owner.routes_[i].lease_owner == c.interface.id() &&
          c.owner.same_path(c.owner.routes_[i], c.desired.entries[c.index]))
      {
        found = true;
      }
    }
    return found;
  }

  static inline void advance(MatchContext &c) noexcept
  {
    ++c.index;
  }

  static inline void success(MatchContext &c) noexcept
  {
    c.result = true;
  }

  static inline bool capacity(Context &c) noexcept
  {
    return c.owner.count_ - c.owner.lease_route_count(c.interface.id()) +
               c.desired.count <=
           RoutingTable::Capacity;
  }

  static inline void remove_previous(Context &c) noexcept
  {
    c.owner.remove_lease_routes(c.interface);
  }

  static inline void append(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.desired.count; ++i)
    {
      c.owner.routes_[c.owner.count_++] = c.desired.entries[i];
    }
    c.result = true;
  }
  struct MatchPrefix;
  struct MatchDescribe;
  struct Scan;
  struct Match;
  struct Advance;
  struct Success;
  using MatchGraph = flow::graph<
      MatchPrefix,
      flow::types<flow::node<MatchPrefix, flow::noop,
                             flow::branch<flow::condition<&valid_prefix<MatchContext>>,
                                          flow::go<MatchDescribe>, flow::stop>>,
                  flow::node<MatchDescribe, flow::action<&describe<MatchContext>>,
                             flow::branch<flow::condition<&same_count>, flow::go<Scan>,
                                          flow::stop>>,
                  flow::node<Scan, flow::noop,
                             flow::branch<flow::condition<&more>, flow::go<Match>,
                                          flow::go<Success>>>,
                  flow::node<Match, flow::noop,
                             flow::branch<flow::condition<&matched>, flow::go<Advance>,
                                          flow::stop>>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
  struct ReplacePrefix;
  struct ReplaceDescribe;
  struct RemovePrevious;
  struct Append;
  using ReplaceGraph = flow::graph<
      ReplacePrefix,
      flow::types<
          flow::node<ReplacePrefix, flow::noop,
                     flow::branch<flow::condition<&valid_prefix<Context>>,
                                  flow::go<ReplaceDescribe>, flow::stop>>,
          flow::node<ReplaceDescribe, flow::action<&describe<Context>>,
                     flow::branch<flow::condition<&capacity>, flow::go<RemovePrevious>,
                                  flow::stop>>,
          flow::node<RemovePrevious, flow::action<&remove_previous>, flow::go<Append>>,
          flow::node<Append, flow::action<&append>, flow::stop>>>;
};

} // namespace xv::net
