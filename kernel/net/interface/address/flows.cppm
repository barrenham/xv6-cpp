module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:address_flows;
import :address_configuration;

namespace xv::net
{
namespace flow = xv::flow;

struct AddressConfiguration::StaticFlow
{
  struct Context
  {
    AddressConfiguration &owner;
    NetworkInterface &interface;
    Ipv4Address address;
    unsigned position = 0;
    int result = 0;
  };

  static inline bool registered(Context &c) noexcept
  {
    return c.owner.interfaces_.find(c.interface.id()) == &c.interface;
  }

  static inline bool loopback(Context &c) noexcept
  {
    return &c.interface == &c.owner.loopback_;
  }

  static inline bool enslaved(Context &c) noexcept
  {
    return c.interface.master();
  }

  static inline bool valid_address(Context &c) noexcept
  {
    return c.address.prefix <= 32 && c.address.address &&
           (c.address.address >> 28) < 14;
  }

  static inline bool capacity(Context &c) noexcept
  {
    return c.interface.address_count_ < NetworkInterface::AddressCapacity &&
           c.owner.routes_.available() >= 2;
  }

  static inline bool unique(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.owner.interfaces_.count(); ++i)
    {
      if (c.owner.interfaces_.at(i)->owns(c.address.address))
      {
        return false;
      }
    }
    return true;
  }

  static inline bool host_address(Context &c) noexcept
  {
    unsigned host = c.address.address & ~c.address.mask();
    return c.address.prefix >= 31 || (host && host != ~c.address.mask());
  }

  static inline bool add_routes(Context &c) noexcept
  {
    c.address.owner = 0;
    return c.owner.routes_.add_address_routes(c.interface, c.owner.loopback_,
                                              c.address);
  }

  static inline void commit_add(Context &c) noexcept
  {
    c.interface.addresses_[c.interface.address_count_++] = c.address;
    ++c.interface.generation_;
  }

  static inline void locate(Context &c) noexcept
  {
    c.position = c.interface.address_count_;
    for (unsigned i = 0; i < c.interface.address_count_; ++i)
    {
      if (c.interface.addresses_[i].address == c.address.address &&
          c.interface.addresses_[i].prefix == c.address.prefix)
      {
        c.position = i;
      }
    }
  }

  static inline bool found(Context &c) noexcept
  {
    return c.position < c.interface.address_count_;
  }

  static inline bool owned(Context &c) noexcept
  {
    return c.interface.addresses_[c.position].owner;
  }

  static inline void commit_remove(Context &c) noexcept
  {
    for (unsigned i = c.position + 1; i < c.interface.address_count_; ++i)
    {
      c.interface.addresses_[i - 1] = c.interface.addresses_[i];
    }
    c.interface.addresses_[--c.interface.address_count_] = {};
    c.owner.routes_.remove_address_routes(c.interface, c.address.address);
    ++c.interface.generation_;
  }

  static inline void no_device(Context &c) noexcept
  {
    c.result = -ENODEV;
  }

  static inline void unsupported(Context &c) noexcept
  {
    c.result = -EOPNOTSUPP;
  }

  static inline void busy(Context &c) noexcept
  {
    c.result = -EBUSY;
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -EINVAL;
  }

  static inline void full(Context &c) noexcept
  {
    c.result = -ENOBUFS;
  }

  static inline void exists(Context &c) noexcept
  {
    c.result = -EEXIST;
  }

  static inline void missing(Context &c) noexcept
  {
    c.result = -ENOENT;
  }
  struct Device;
  struct Loopback;
  struct Slave;
  struct Address;
  struct Capacity;
  struct Unique;
  struct Host;
  struct Routes;
  struct Commit;
  struct NoDevice;
  struct Unsupported;
  struct Busy;
  struct Invalid;
  struct Full;
  struct Exists;
  struct Missing;
  using AddGraph = flow::graph<
      Device,
      flow::types<flow::node<Device, flow::noop,
                             flow::branch<flow::condition<&registered>,
                                          flow::go<Loopback>, flow::go<NoDevice>>>,
                  flow::node<Loopback, flow::noop,
                             flow::branch<flow::condition<&loopback>,
                                          flow::go<Unsupported>, flow::go<Slave>>>,
                  flow::node<Slave, flow::noop,
                             flow::branch<flow::condition<&enslaved>, flow::go<Busy>,
                                          flow::go<Address>>>,
                  flow::node<Address, flow::noop,
                             flow::branch<flow::condition<&valid_address>,
                                          flow::go<Capacity>, flow::go<Invalid>>>,
                  flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&capacity>, flow::go<Unique>,
                                          flow::go<Full>>>,
                  flow::node<Unique, flow::noop,
                             flow::branch<flow::condition<&unique>, flow::go<Host>,
                                          flow::go<Exists>>>,
                  flow::node<Host, flow::noop,
                             flow::branch<flow::condition<&host_address>,
                                          flow::go<Routes>, flow::go<Invalid>>>,
                  flow::node<Routes, flow::noop,
                             flow::branch<flow::condition<&add_routes>,
                                          flow::go<Commit>, flow::go<Full>>>,
                  flow::node<Commit, flow::action<&commit_add>, flow::stop>,
                  flow::node<NoDevice, flow::action<&no_device>, flow::stop>,
                  flow::node<Unsupported, flow::action<&unsupported>, flow::stop>,
                  flow::node<Busy, flow::action<&busy>, flow::stop>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Full, flow::action<&full>, flow::stop>,
                  flow::node<Exists, flow::action<&exists>, flow::stop>,
                  flow::node<Missing, flow::action<&missing>, flow::stop>>>;
  struct Locate;
  struct Owner;
  struct Remove;
  using RemoveGraph = flow::graph<
      Device,
      flow::types<flow::node<Device, flow::noop,
                             flow::branch<flow::condition<&registered>,
                                          flow::go<Loopback>, flow::go<NoDevice>>>,
                  flow::node<Loopback, flow::noop,
                             flow::branch<flow::condition<&loopback>,
                                          flow::go<Unsupported>, flow::go<Slave>>>,
                  flow::node<Slave, flow::noop,
                             flow::branch<flow::condition<&enslaved>, flow::go<Busy>,
                                          flow::go<Address>>>,
                  flow::node<Address, flow::noop,
                             flow::branch<flow::condition<&valid_address>,
                                          flow::go<Locate>, flow::go<Invalid>>>,
                  flow::node<Locate, flow::action<&locate>,
                             flow::branch<flow::condition<&found>, flow::go<Owner>,
                                          flow::go<Missing>>>,
                  flow::node<Owner, flow::noop,
                             flow::branch<flow::condition<&owned>, flow::go<Busy>,
                                          flow::go<Remove>>>,
                  flow::node<Remove, flow::action<&commit_remove>, flow::stop>,
                  flow::node<NoDevice, flow::action<&no_device>, flow::stop>,
                  flow::node<Unsupported, flow::action<&unsupported>, flow::stop>,
                  flow::node<Busy, flow::action<&busy>, flow::stop>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Full, flow::action<&full>, flow::stop>,
                  flow::node<Exists, flow::action<&exists>, flow::stop>,
                  flow::node<Missing, flow::action<&missing>, flow::stop>>>;
};

struct AddressConfiguration::LeaseFlow
{
  struct Context
  {
    AddressConfiguration &owner;
    NetworkInterface &interface;
    Ipv4Address address;
    unsigned gateway, metric;
    bool &changed;
    unsigned owner_id = 0, position = 0, retained = 0;
    int result = 0;
  };

  static inline void reset(Context &c) noexcept
  {
    c.changed = false;
  }

  static inline bool valid(Context &c) noexcept
  {
    return c.address.address && c.address.prefix <= 32;
  }

  static inline bool reachable(Context &c) noexcept
  {
    return c.owner.interfaces_.find(c.interface.id()) == &c.interface &&
           c.interface.up() && !c.interface.master();
  }

  static inline void locate(Context &c) noexcept
  {
    c.owner_id = c.interface.id();
    c.position = c.interface.address_count_;
    for (unsigned i = 0; i < c.interface.address_count_; ++i)
    {
      if (c.interface.addresses_[i].owner == c.owner_id)
      {
        c.position = i;
      }
    }
  }

  static inline bool unique(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.owner.interfaces_.count(); ++i)
    {
      auto *other = c.owner.interfaces_.at(i);
      if (other != &c.interface && other->owns(c.address.address))
      {
        return false;
      }
    }
    for (unsigned i = 0; i < c.interface.address_count_; ++i)
    {
      if (i != c.position && c.interface.addresses_[i].address == c.address.address)
      {
        return false;
      }
    }
    return true;
  }

  static inline bool capacity(Context &c) noexcept
  {
    return c.position != c.interface.address_count_ ||
           c.position != NetworkInterface::AddressCapacity;
  }

  static inline void assign_owner(Context &c) noexcept
  {
    c.address.owner = c.owner_id;
  }

  static inline bool unchanged(Context &c) noexcept
  {
    return c.position < c.interface.address_count_ &&
           c.interface.addresses_[c.position].address == c.address.address &&
           c.interface.addresses_[c.position].prefix == c.address.prefix &&
           c.owner.routes_.lease_routes_match(c.interface, c.owner.loopback_, c.address,
                                              c.gateway, c.metric);
  }

  static inline bool replace_routes(Context &c) noexcept
  {
    return c.owner.routes_.replace_lease_routes(c.interface, c.owner.loopback_,
                                                c.address, c.gateway, c.metric);
  }

  static inline void commit(Context &c) noexcept
  {
    if (c.position == c.interface.address_count_)
    {
      ++c.interface.address_count_;
    }
    c.interface.addresses_[c.position] = c.address;
    ++c.interface.generation_;
    c.changed = true;
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -EINVAL;
  }

  static inline void unreachable(Context &c) noexcept
  {
    c.result = -ENETUNREACH;
  }

  static inline void conflict(Context &c) noexcept
  {
    c.result = -EADDRINUSE;
  }

  static inline void full(Context &c) noexcept
  {
    c.result = -ENOBUFS;
  }
  struct Input;
  struct Interface;
  struct Locate;
  struct Capacity;
  struct Owner;
  struct Routes;
  struct Commit;
  struct Invalid;
  struct Unreachable;
  struct Conflict;
  struct Full;
  using Graph = flow::graph<
      Input,
      flow::types<flow::node<Input, flow::action<&reset>,
                             flow::branch<flow::condition<&valid>, flow::go<Interface>,
                                          flow::go<Invalid>>>,
                  flow::node<Interface, flow::noop,
                             flow::branch<flow::condition<&reachable>, flow::go<Locate>,
                                          flow::go<Unreachable>>>,
                  flow::node<Locate, flow::action<&locate>,
                             flow::branch<flow::condition<&unique>, flow::go<Capacity>,
                                          flow::go<Conflict>>>,
                  flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&capacity>, flow::go<Owner>,
                                          flow::go<Full>>>,
                  flow::node<Owner, flow::action<&assign_owner>,
                             flow::branch<flow::condition<&unchanged>, flow::stop,
                                          flow::go<Routes>>>,
                  flow::node<Routes, flow::noop,
                             flow::branch<flow::condition<&replace_routes>,
                                          flow::go<Commit>, flow::go<Full>>>,
                  flow::node<Commit, flow::action<&commit>, flow::stop>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Unreachable, flow::action<&unreachable>, flow::stop>,
                  flow::node<Conflict, flow::action<&conflict>, flow::stop>,
                  flow::node<Full, flow::action<&full>, flow::stop>>>;
};

} // namespace xv::net
