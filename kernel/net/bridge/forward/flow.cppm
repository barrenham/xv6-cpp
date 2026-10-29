module;
#include "kernel/support/flow.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/net/errno.h"
module xv.net:bridge_forward_flow;
import :bridge;
import :wire;
import :platform;

namespace xv::net
{
namespace flow = xv::flow;

struct BridgeInterface::ForwardFlow
{
  struct Context
  {
    BridgeInterface &owner;
    Layer2Port *ingress;
    unsigned epoch;
    Packet &frame;
    const unsigned char *bytes = nullptr;
    Member incoming{}, targets[4]{};
    Entry *destination = nullptr;
    unsigned count = 0, index = 0;
    unsigned long now = 0;
    bool own = false, group = false, local = false, selected = false, sent = false,
         result = false;
    Packet copy{};
  };

  static inline bool valid_size(Context &c) noexcept
  {
    return c.frame && c.frame.size() >= 14 && c.frame.size() <= c.owner.mtu() + 14;
  }

  static inline void inspect(Context &c) noexcept
  {
    c.bytes = c.frame.data();
  }

  static inline bool valid_frame(Context &c) noexcept
  {
    const auto *p = c.bytes;
    unsigned type = wire::read16(p + 12);
    if ((p[6] & 1) || type == 0x8100 || type == 0x88a8 ||
        (p[0] == 1 && p[1] == 0x80 && p[2] == 0xc2 && !p[3] && !p[4] && p[5] <= 15))
    {
      return false;
    }
    bool nonzero = false;
    for (unsigned i = 6; i < 12; ++i)
    {
      nonzero |= p[i] != 0;
    }
    return nonzero;
  }

  static inline bool online(Context &c) noexcept
  {
    return c.owner.active_ && c.owner.up();
  }

  static inline void find_ingress(Context &c) noexcept
  {
    for (const auto &member : c.owner.members_)
    {
      if (member.port == c.ingress)
      {
        c.incoming = member;
      }
    }
  }

  static inline bool valid_ingress(Context &c) noexcept
  {
    return !c.ingress ||
           (c.incoming.interface && c.ingress->master_ == &c.owner &&
            c.ingress->epoch_ == c.incoming.epoch && c.incoming.epoch == c.epoch &&
            c.incoming.interface->up() && c.incoming.interface->carrier());
  }

  static inline bool own_source(Context &c) noexcept
  {
    return c.ingress && wire::equal(c.bytes + 6, c.owner.bridge_mac_.bytes, 6);
  }

  static inline void learn(Context &c) noexcept
  {
    c.now = detail::milliseconds();
    if (!c.ingress)
    {
      return;
    }
    auto *slot = &c.owner.fdb_[0];
    for (auto &entry : c.owner.fdb_)
    {
      if (entry.member.port && wire::equal(entry.mac.bytes, c.bytes + 6, 6))
      {
        slot = &entry;
        break;
      }
      if (!entry.member.port || entry.expires < slot->expires)
      {
        slot = &entry;
      }
    }
    wire::copy(slot->mac.bytes, c.bytes + 6, 6);
    slot->member = c.incoming;
    slot->expires = c.now + 300000;
  }

  static inline void classify(Context &c) noexcept
  {
    c.own = wire::equal(c.bytes, c.owner.bridge_mac_.bytes, 6);
    c.group = c.bytes[0] & 1;
    c.local = c.ingress && (c.own || c.group);
    if (!c.own && !c.group)
    {
      for (auto &entry : c.owner.fdb_)
      {
        if (entry.member.port && entry.expires > c.now &&
            wire::equal(entry.mac.bytes, c.bytes, 6))
        {
          c.destination = &entry;
          break;
        }
      }
    }
  }

  static inline void collect(Context &c) noexcept
  {
    if (!c.own)
    {
      for (const auto &member : c.owner.members_)
      {
        if (member.port && member.port != c.ingress && member.interface->up() &&
            member.interface->carrier() &&
            (!c.destination || (c.destination->member.port == member.port &&
                                c.destination->member.epoch == member.epoch)))
        {
          c.targets[c.count++] = member;
        }
      }
    }
    c.selected = true;
  }
  struct Online;
  struct Ingress;
  struct OwnSource;
  struct Learn;
  struct Classify;
  struct Collect;
  using SelectionGraph = flow::graph<
      Online,
      flow::types<flow::node<Online, flow::noop,
                             flow::branch<flow::condition<&online>, flow::go<Ingress>,
                                          flow::stop>>,
                  flow::node<Ingress, flow::action<&find_ingress>,
                             flow::branch<flow::condition<&valid_ingress>,
                                          flow::go<OwnSource>, flow::stop>>,
                  flow::node<OwnSource, flow::noop,
                             flow::branch<flow::condition<&own_source>, flow::stop,
                                          flow::go<Learn>>>,
                  flow::node<Learn, flow::action<&learn>, flow::go<Classify>>,
                  flow::node<Classify, flow::action<&classify>, flow::go<Collect>>,
                  flow::node<Collect, flow::action<&collect>, flow::stop>>>;

  static inline void select_targets(Context &c) noexcept
  {
    ConfigurationGuard guard;
    (void)SelectionGraph::run<0>(c);
  }

  static inline bool selected(Context &c) noexcept
  {
    return c.selected;
  }

  static inline bool more_targets(Context &c) noexcept
  {
    return c.index < c.count;
  }

  static inline void clone(Context &c) noexcept
  {
    c.copy.~Packet();
    new (&c.copy) Packet(c.frame.clone());
  }

  static inline bool cloned(Context &c) noexcept
  {
    return bool(c.copy);
  }

  static inline bool still_valid(Context &c) noexcept
  {
    ConfigurationGuard guard;
    const auto &member = c.targets[c.index];
    return c.owner.active_ && c.owner.up() && member.port->master_ == &c.owner &&
           member.port->epoch_ == member.epoch && member.interface->up() &&
           member.interface->carrier();
  }

  static inline void send(Context &c) noexcept
  {
    if (c.targets[c.index].port->transmit_frame(c.copy) == 0)
    {
      c.sent = true;
    }
  }

  static inline void release_copy(Context &c) noexcept
  {
    c.copy.~Packet();
    new (&c.copy) Packet();
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }

  static inline bool local(Context &c) noexcept
  {
    return c.local;
  }

  static inline void deliver(Context &c) noexcept
  {
    c.owner.receiver().receive_frame(c.frame.release(), c.frame.size());
    c.result = true;
  }

  static inline void forwarded(Context &c) noexcept
  {
    c.result = c.sent;
  }
  struct Size;
  struct Header;
  struct Selection;
  struct Targets;
  struct Clone;
  struct Revalidate;
  struct Send;
  struct ReleaseCopy;
  struct Advance;
  struct Local;
  struct Deliver;
  struct Forwarded;
  using Graph = flow::graph<
      Size,
      flow::types<
          flow::node<
              Size, flow::noop,
              flow::branch<flow::condition<&valid_size>, flow::go<Header>, flow::stop>>,
          flow::node<Header, flow::action<&inspect>,
                     flow::branch<flow::condition<&valid_frame>, flow::go<Selection>,
                                  flow::stop>>,
          flow::node<
              Selection, flow::action<&select_targets>,
              flow::branch<flow::condition<&selected>, flow::go<Targets>, flow::stop>>,
          flow::node<Targets, flow::noop,
                     flow::branch<flow::condition<&more_targets>, flow::go<Clone>,
                                  flow::go<Local>>>,
          flow::node<Clone, flow::action<&clone>,
                     flow::branch<flow::condition<&cloned>, flow::go<Revalidate>,
                                  flow::go<ReleaseCopy>>>,
          flow::node<Revalidate, flow::noop,
                     flow::branch<flow::condition<&still_valid>, flow::go<Send>,
                                  flow::go<ReleaseCopy>>>,
          flow::node<Send, flow::action<&send>, flow::go<ReleaseCopy>>,
          flow::node<ReleaseCopy, flow::action<&release_copy>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Targets>>,
          flow::node<Local, flow::noop,
                     flow::branch<flow::condition<&local>, flow::go<Deliver>,
                                  flow::go<Forwarded>>>,
          flow::node<Deliver, flow::action<&deliver>, flow::stop>,
          flow::node<Forwarded, flow::action<&forwarded>, flow::stop>>>;
};

} // namespace xv::net
