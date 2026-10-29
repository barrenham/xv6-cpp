module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
#include "kernel/net/control.h"
module xv.net:bridge_manager_flows;
import :bridge;
import :wire;
import :platform;

namespace xv::net
{
namespace flow = xv::flow;

struct BridgeManager::MembershipFlow
{
  struct Context
  {
    BridgeManager &owner;
    BridgeInterface &bridge;
    NetworkInterface &interface;
    bool add;
    Layer2Port *port = nullptr;
    unsigned slot = 4;
    bool valid = false;
    int result = 0;
  };

  static inline void inspect(Context &c) noexcept
  {
    c.port = c.interface.layer2();
  }

  static inline bool supported(Context &c) noexcept
  {
    return c.port && !c.interface.is_bridge() && c.port->frame_mtu() >= c.bridge.mtu();
  }

  static inline bool adding(Context &c) noexcept
  {
    return c.add;
  }

  static inline bool unused(Context &c) noexcept
  {
    return !c.port->master_ && c.owner.unused(c.interface);
  }

  static inline void find_empty(Context &c) noexcept
  {
    for (unsigned i = 0; i < 4; ++i)
    {
      if (!c.bridge.members_[i].port)
      {
        c.slot = i;
        break;
      }
    }
  }

  static inline bool slot_available(Context &c) noexcept
  {
    return c.slot != 4;
  }

  static inline void find_member(Context &c) noexcept
  {
    for (unsigned i = 0; i < 4; ++i)
    {
      if (c.bridge.members_[i].port == c.port)
      {
        c.slot = i;
      }
    }
  }

  static inline bool member(Context &c) noexcept
  {
    return c.slot != 4 && c.port->master_ == &c.bridge;
  }

  static inline void remove(Context &c) noexcept
  {
    c.port->master_ = nullptr;
    ++c.port->epoch_;
    c.bridge.members_[c.slot] = {};
    for (auto &entry : c.bridge.fdb_)
    {
      if (entry.member.port == c.port)
      {
        entry = {};
      }
    }
    c.interface.topology_changed();
  }

  static inline void busy(Context &c) noexcept
  {
    c.result = -EBUSY;
  }

  static inline void full(Context &c) noexcept
  {
    c.result = -ENOBUFS;
  }

  static inline void missing(Context &c) noexcept
  {
    c.result = -ENOENT;
  }
  struct Adding;
  struct Unused;
  struct FindEmpty;
  struct FindMember;
  struct Remove;
  struct Busy;
  struct Full;
  struct Missing;
  using PrepareGraph = flow::graph<
      Adding,
      flow::types<flow::node<Adding, flow::noop,
                             flow::branch<flow::condition<&adding>, flow::go<Unused>,
                                          flow::go<FindMember>>>,
                  flow::node<Unused, flow::noop,
                             flow::branch<flow::condition<&unused>, flow::go<FindEmpty>,
                                          flow::go<Busy>>>,
                  flow::node<FindEmpty, flow::action<&find_empty>,
                             flow::branch<flow::condition<&slot_available>, flow::stop,
                                          flow::go<Full>>>,
                  flow::node<FindMember, flow::action<&find_member>,
                             flow::branch<flow::condition<&member>, flow::go<Remove>,
                                          flow::go<Missing>>>,
                  flow::node<Remove, flow::action<&remove>, flow::stop>,
                  flow::node<Busy, flow::action<&busy>, flow::stop>,
                  flow::node<Full, flow::action<&full>, flow::stop>,
                  flow::node<Missing, flow::action<&missing>, flow::stop>>>;

  static inline void prepare(Context &c) noexcept
  {
    ConfigurationGuard guard;
    (void)PrepareGraph::run<0>(c);
  }

  static inline bool prepared(Context &c) noexcept
  {
    return c.result >= 0;
  }

  static inline bool enable_promiscuous(Context &c) noexcept
  {
    return c.port->promiscuous(true);
  }

  static inline void disable_promiscuous(Context &c) noexcept
  {
    c.port->promiscuous(false);
  }

  static inline void commit(Context &c) noexcept
  {
    ConfigurationGuard guard;
    c.valid = c.bridge.active_ && !c.port->master_ && c.owner.unused(c.interface) &&
              !c.bridge.members_[c.slot].port;
    if (c.valid)
    {
      c.port->master_ = &c.bridge;
      ++c.port->epoch_;
      c.interface.topology_changed();
      c.bridge.members_[c.slot] = {&c.interface, c.port, c.port->epoch_};
    }
  }

  static inline bool committed(Context &c) noexcept
  {
    return c.valid;
  }

  static inline void rollback(Context &c) noexcept
  {
    c.port->promiscuous(false);
    c.result = -EBUSY;
  }

  static inline void invalidate(Context &c) noexcept
  {
    c.interface.invalidate();
  }

  static inline void unsupported(Context &c) noexcept
  {
    c.result = -EOPNOTSUPP;
  }
  struct Port;
  struct Prepare;
  struct Add;
  struct Disable;
  struct Promiscuous;
  struct Commit;
  struct Rollback;
  struct Invalidate;
  struct Unsupported;
  using Graph = flow::graph<
      Port,
      flow::types<
          flow::node<Port, flow::action<&inspect>,
                     flow::branch<flow::condition<&supported>, flow::go<Prepare>,
                                  flow::go<Unsupported>>>,
          flow::node<
              Prepare, flow::action<&prepare>,
              flow::branch<flow::condition<&prepared>, flow::go<Add>, flow::stop>>,
          flow::node<Add, flow::noop,
                     flow::branch<flow::condition<&adding>, flow::go<Promiscuous>,
                                  flow::go<Disable>>>,
          flow::node<Disable, flow::action<&disable_promiscuous>, flow::go<Invalidate>>,
          flow::node<Promiscuous, flow::noop,
                     flow::branch<flow::condition<&enable_promiscuous>,
                                  flow::go<Commit>, flow::go<Unsupported>>>,
          flow::node<Commit, flow::action<&commit>,
                     flow::branch<flow::condition<&committed>, flow::go<Invalidate>,
                                  flow::go<Rollback>>>,
          flow::node<Rollback, flow::action<&rollback>, flow::stop>,
          flow::node<Invalidate, flow::action<&invalidate>, flow::stop>,
          flow::node<Unsupported, flow::action<&unsupported>, flow::stop>>>;
};

struct BridgeManager::ExecuteFlow
{
  struct Context
  {
    BridgeManager &owner;
    unsigned operation;
    netctl_request &request;
    BridgeInterface *bridge = nullptr;
    NetworkInterface *interface = nullptr;
    bool complete = false, deleted = false;
    int result = 0;
  };

  template <unsigned Operation> static bool operation(Context &c) noexcept
  {
    return c.operation == Operation;
  }

  static inline bool version(Context &c) noexcept
  {
    return c.request.version == NETCTL_VERSION && c.request.size == sizeof(c.request);
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -EINVAL;
    c.complete = true;
  }

  static inline void missing(Context &c) noexcept
  {
    c.result = -ENODEV;
    c.complete = true;
  }

  static inline void busy(Context &c) noexcept
  {
    c.result = -EBUSY;
    c.complete = true;
  }

  static inline void exists(Context &c) noexcept
  {
    c.result = -EEXIST;
  }

  static inline void find_name(Context &c) noexcept
  {
    for (auto &bridge : c.owner.bridges_)
    {
      unsigned i = 0;
      while (i < sizeof(c.request.name) && c.request.name[i] &&
             c.request.name[i] == bridge.name()[i])
      {
        ++i;
      }
      if (i < sizeof(c.request.name) && !c.request.name[i] && !bridge.name()[i])
      {
        c.bridge = &bridge;
      }
    }
  }

  static inline bool found(Context &c) noexcept
  {
    return c.bridge != nullptr;
  }

  static inline bool already_active(Context &c) noexcept
  {
    return c.bridge->active_ || c.owner.interfaces_.find(c.bridge->id());
  }

  static inline void publish(Context &c) noexcept
  {
    c.result = c.owner.interfaces_.publish(*c.bridge);
  }

  static inline bool published(Context &c) noexcept
  {
    return c.result >= 0;
  }

  static inline void activate(Context &c) noexcept
  {
    c.bridge->active_ = true;
  }
  struct Name;
  struct Active;
  struct Publish;
  struct Activate;
  struct Exists;
  struct InvalidName;
  using CreateGraph = flow::graph<
      Name,
      flow::types<flow::node<Name, flow::action<&find_name>,
                             flow::branch<flow::condition<&found>, flow::go<Active>,
                                          flow::go<InvalidName>>>,
                  flow::node<Active, flow::noop,
                             flow::branch<flow::condition<&already_active>,
                                          flow::go<Exists>, flow::go<Publish>>>,
                  flow::node<Publish, flow::action<&publish>,
                             flow::branch<flow::condition<&published>,
                                          flow::go<Activate>, flow::stop>>,
                  flow::node<Activate, flow::action<&activate>, flow::stop>,
                  flow::node<Exists, flow::action<&exists>, flow::stop>,
                  flow::node<InvalidName, flow::action<&invalid>, flow::stop>>>;

  static inline void create(Context &c) noexcept
  {
    ConfigurationGuard guard;
    (void)CreateGraph::run<0>(c);
  }

  static inline void find_bridge(Context &c) noexcept
  {
    c.bridge = c.owner.find(c.request.ifindex);
  }

  static inline bool deletable(Context &c) noexcept
  {
    if (c.bridge->up() || !c.owner.unused(*c.bridge))
    {
      return false;
    }
    for (const auto &member : c.bridge->members_)
    {
      if (member.port)
      {
        return false;
      }
    }
    return true;
  }

  static inline void delete_bridge(Context &c) noexcept
  {
    c.bridge->active_ = false;
    c.owner.interfaces_.withdraw(*c.bridge);
    for (auto &entry : c.bridge->fdb_)
    {
      entry = {};
    }
    c.deleted = true;
  }

  static inline bool inspection(Context &c) noexcept
  {
    return c.operation == NETCTL_BRIDGE_PORT || c.operation == NETCTL_BRIDGE_FDB;
  }

  static inline void inspect(Context &c) noexcept
  {
    netctl_request out{};
    out.version = NETCTL_VERSION;
    out.size = sizeof(out);
    out.ifindex = c.bridge->id();
    out.index = c.request.index;
    unsigned index = c.request.index;
    bool found = false;
    if (c.operation == NETCTL_BRIDGE_PORT)
    {
      for (const auto &member : c.bridge->members_)
      {
        if (member.port && !index--)
        {
          out.source = member.interface->id();
          out.flags = member.interface->up() ? NET_LINK_UP : 0;
          found = true;
          break;
        }
      }
    }
    else
    {
      auto now = detail::milliseconds();
      for (const auto &entry : c.bridge->fdb_)
      {
        if (entry.member.port && entry.expires > now && !index--)
        {
          out.source = entry.member.interface->id();
          wire::copy(out.mac, entry.mac.bytes, 6);
          out.expires_ms = entry.expires - now;
          found = true;
          break;
        }
      }
    }
    if (found)
    {
      c.request = out;
      c.result = 1;
    }
    c.complete = true;
  }

  static inline void flush(Context &c) noexcept
  {
    for (auto &entry : c.bridge->fdb_)
    {
      entry = {};
    }
    c.complete = true;
  }

  static inline bool membership_operation(Context &c) noexcept
  {
    return c.operation == NETCTL_BRIDGE_ADD_PORT ||
           c.operation == NETCTL_BRIDGE_DEL_PORT;
  }
  struct Bridge;
  struct DeleteOperation;
  struct Deletable;
  struct DeleteBridge;
  struct Inspection;
  struct Inspect;
  struct FlushOperation;
  struct Flush;
  struct MembershipOperation;
  struct Missing;
  struct Busy;
  struct Invalid;
  using ConfigurationGraph = flow::graph<
      Bridge,
      flow::types<
          flow::node<Bridge, flow::action<&find_bridge>,
                     flow::branch<flow::condition<&found>, flow::go<DeleteOperation>,
                                  flow::go<Missing>>>,
          flow::node<DeleteOperation, flow::noop,
                     flow::branch<flow::condition<&operation<NETCTL_BRIDGE_DELETE>>,
                                  flow::go<Deletable>, flow::go<Inspection>>>,
          flow::node<Deletable, flow::noop,
                     flow::branch<flow::condition<&deletable>, flow::go<DeleteBridge>,
                                  flow::go<Busy>>>,
          flow::node<DeleteBridge, flow::action<&delete_bridge>, flow::stop>,
          flow::node<Inspection, flow::noop,
                     flow::branch<flow::condition<&inspection>, flow::go<Inspect>,
                                  flow::go<FlushOperation>>>,
          flow::node<Inspect, flow::action<&inspect>, flow::stop>,
          flow::node<FlushOperation, flow::noop,
                     flow::branch<flow::condition<&operation<NETCTL_BRIDGE_FLUSH_FDB>>,
                                  flow::go<Flush>, flow::go<MembershipOperation>>>,
          flow::node<Flush, flow::action<&flush>, flow::stop>,
          flow::node<MembershipOperation, flow::noop,
                     flow::branch<flow::condition<&membership_operation>, flow::stop,
                                  flow::go<Invalid>>>,
          flow::node<Missing, flow::action<&missing>, flow::stop>,
          flow::node<Busy, flow::action<&busy>, flow::stop>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>>>;

  static inline void configure(Context &c) noexcept
  {
    ConfigurationGuard guard;
    (void)ConfigurationGraph::run<0>(c);
  }

  static inline bool complete(Context &c) noexcept
  {
    return c.complete;
  }

  static inline bool deleted(Context &c) noexcept
  {
    return c.deleted;
  }

  static inline void invalidate(Context &c) noexcept
  {
    c.bridge->invalidate();
  }

  static inline void find_interface(Context &c) noexcept
  {
    ConfigurationGuard guard;
    c.interface = c.owner.interfaces_.find(c.request.source);
  }

  static inline bool interface_found(Context &c) noexcept
  {
    return c.interface != nullptr;
  }

  static inline void membership(Context &c) noexcept
  {
    c.result = c.owner.membership(*c.bridge, *c.interface,
                                  c.operation == NETCTL_BRIDGE_ADD_PORT);
  }
  struct CreateOperation;
  struct Create;
  struct Configure;
  struct Deleted;
  struct Invalidate;
  struct Interface;
  struct Membership;
  struct NoInterface;
  using AdminGraph = flow::graph<
      CreateOperation,
      flow::types<
          flow::node<CreateOperation, flow::noop,
                     flow::branch<flow::condition<&operation<NETCTL_BRIDGE_CREATE>>,
                                  flow::go<Create>, flow::go<Configure>>>,
          flow::node<Create, flow::action<&create>, flow::stop>,
          flow::node<
              Configure, flow::action<&configure>,
              flow::branch<flow::condition<&complete>, flow::stop, flow::go<Deleted>>>,
          flow::node<Deleted, flow::noop,
                     flow::branch<flow::condition<&deleted>, flow::go<Invalidate>,
                                  flow::go<Interface>>>,
          flow::node<Invalidate, flow::action<&invalidate>, flow::stop>,
          flow::node<Interface, flow::action<&find_interface>,
                     flow::branch<flow::condition<&interface_found>,
                                  flow::go<Membership>, flow::go<NoInterface>>>,
          flow::node<Membership, flow::action<&membership>, flow::stop>,
          flow::node<NoInterface, flow::action<&missing>, flow::stop>>>;

  static inline void administer(Context &c) noexcept
  {
    xv::sync::ScopedLock administrative(c.owner.admin_lock_);
    (void)AdminGraph::run<0>(c);
  }
  struct Version;
  struct Administer;
  struct BadVersion;
  using Graph = flow::graph<
      Version,
      flow::types<flow::node<Version, flow::noop,
                             flow::branch<flow::condition<&version>,
                                          flow::go<Administer>, flow::go<BadVersion>>>,
                  flow::node<Administer, flow::action<&administer>, flow::stop>,
                  flow::node<BadVersion, flow::action<&invalid>, flow::stop>>>;
};

} // namespace xv::net
