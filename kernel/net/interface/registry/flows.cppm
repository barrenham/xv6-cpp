module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:interface_registry_flows;
import :interface;

namespace xv::net
{
namespace flow = xv::flow;

struct InterfaceRegistry::RegistrationFlow
{
  struct Context
  {
    const InterfaceRegistry &owner;
    const NetworkInterface &interface;
    unsigned index = 0;
    int result = 0;
  };

  static inline bool valid(Context &c) noexcept
  {
    return c.interface.id() && c.interface.name() && c.interface.mtu() >= 68 &&
           c.interface.mtu() <= 4082;
  }

  static inline bool identifier_unique(Context &c) noexcept
  {
    return !c.owner.find(c.interface.id());
  }

  static inline bool more(Context &c) noexcept
  {
    return c.index < c.owner.count_;
  }

  static inline bool name_unique(Context &c) noexcept
  {
    auto *existing = c.owner.interfaces_[c.index]->name();
    auto *name = c.interface.name();
    while (*existing && *existing == *name)
    {
      ++existing;
      ++name;
    }
    return *existing != *name;
  }

  static inline bool addresses_unique(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.interface.address_count(); ++i)
    {
      if (c.owner.interfaces_[c.index]->owns(c.interface.address(i).address))
      {
        return false;
      }
    }
    return true;
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }

  static inline bool capacity(Context &c) noexcept
  {
    return c.owner.count_ != InterfaceRegistry::Capacity;
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
  struct Attributes;
  struct Identifier;
  struct Scan;
  struct Name;
  struct Addresses;
  struct Advance;
  struct Capacity;
  struct Invalid;
  struct Exists;
  struct Full;
  using Graph = flow::graph<
      Attributes,
      flow::types<flow::node<Attributes, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Identifier>,
                                          flow::go<Invalid>>>,
                  flow::node<Identifier, flow::noop,
                             flow::branch<flow::condition<&identifier_unique>,
                                          flow::go<Scan>, flow::go<Exists>>>,
                  flow::node<Scan, flow::noop,
                             flow::branch<flow::condition<&more>, flow::go<Name>,
                                          flow::go<Capacity>>>,
                  flow::node<Name, flow::noop,
                             flow::branch<flow::condition<&name_unique>,
                                          flow::go<Addresses>, flow::go<Exists>>>,
                  flow::node<Addresses, flow::noop,
                             flow::branch<flow::condition<&addresses_unique>,
                                          flow::go<Advance>, flow::go<Exists>>>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
                  flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&capacity>, flow::stop,
                                          flow::go<Full>>>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Exists, flow::action<&exists>, flow::stop>,
                  flow::node<Full, flow::action<&full>, flow::stop>>>;
};

struct NetworkInterface::DeliverFlow
{
  struct Context
  {
    NetworkInterface &owner;
    Packet &packet;
  };

  static inline bool eligible(Context &c) noexcept
  {
    return c.owner.up() && c.owner.receiver_;
  }

  static inline void count(Context &c) noexcept
  {
    __atomic_fetch_add(&c.owner.statistics_.rx_packets, 1ULL, __ATOMIC_RELAXED);
    __atomic_fetch_add(&c.owner.statistics_.rx_bytes,
                       static_cast<unsigned long long>(c.packet.size()),
                       __ATOMIC_RELAXED);
  }

  static inline void deliver(Context &c) noexcept
  {
    c.owner.receiver_->receive(c.owner, c.packet);
  }

  static inline void drop(Context &c) noexcept
  {
    c.owner.receive_drop();
  }
  struct Eligible;
  struct Count;
  struct Deliver;
  struct Drop;
  using Graph = flow::graph<
      Eligible, flow::types<flow::node<Eligible, flow::noop,
                                       flow::branch<flow::condition<&eligible>,
                                                    flow::go<Count>, flow::go<Drop>>>,
                            flow::node<Count, flow::action<&count>, flow::go<Deliver>>,
                            flow::node<Deliver, flow::action<&deliver>, flow::stop>,
                            flow::node<Drop, flow::action<&drop>, flow::stop>>>;
};

} // namespace xv::net
