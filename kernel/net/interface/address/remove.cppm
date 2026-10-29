module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:address_remove_flow;
import :address_configuration;

namespace xv::net
{
namespace flow = xv::flow;

struct AddressConfiguration::RemoveLeaseFlow
{
  struct Context
  {
    AddressConfiguration &owner;
    NetworkInterface &interface;
    unsigned owner_id = 0, retained = 0;
    bool changed = false;
  };

  static inline bool registered(Context &c) noexcept
  {
    return c.owner.interfaces_.find(c.interface.id()) == &c.interface;
  }

  static inline void identify(Context &c) noexcept
  {
    c.owner_id = c.interface.id();
  }

  static inline void compact_addresses(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.interface.address_count_; ++i)
    {
      if (c.interface.addresses_[i].owner == c.owner_id)
      {
        c.changed = true;
      }
      else
      {
        c.interface.addresses_[c.retained++] = c.interface.addresses_[i];
      }
    }
    for (unsigned i = c.retained; i < c.interface.address_count_; ++i)
    {
      c.interface.addresses_[i] = {};
    }
    c.interface.address_count_ = c.retained;
  }

  static inline bool remove_routes(Context &c) noexcept
  {
    return c.owner.routes_.remove_lease_routes(c.interface);
  }

  static inline void changed(Context &c) noexcept
  {
    c.changed = true;
  }

  static inline bool any_change(Context &c) noexcept
  {
    return c.changed;
  }

  static inline void advance_generation(Context &c) noexcept
  {
    ++c.interface.generation_;
  }
  struct Registered;
  struct Owner;
  struct Addresses;
  struct Changed;
  struct ChangeCheck;
  struct Generation;
  using Graph = flow::graph<
      Registered,
      flow::types<
          flow::node<
              Registered, flow::noop,
              flow::branch<flow::condition<&registered>, flow::go<Owner>, flow::stop>>,
          flow::node<Owner, flow::action<&identify>, flow::go<Addresses>>,
          flow::node<Addresses, flow::action<&compact_addresses>,
                     flow::branch<flow::condition<&remove_routes>, flow::go<Changed>,
                                  flow::go<ChangeCheck>>>,
          flow::node<Changed, flow::action<&changed>, flow::go<ChangeCheck>>,
          flow::node<ChangeCheck, flow::noop,
                     flow::branch<flow::condition<&any_change>, flow::go<Generation>,
                                  flow::stop>>,
          flow::node<Generation, flow::action<&advance_generation>, flow::stop>>>;
};

} // namespace xv::net
