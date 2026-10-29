module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:loopback_flows;
import :loopback;

namespace xv::net
{
namespace flow = xv::flow;

struct LoopbackInterface::TransmitFlow
{
  struct Context
  {
    LoopbackInterface &owner;
    Packet &packet;
    int result = 0;
  };

  static inline bool online(Context &c) noexcept
  {
    return c.owner.up();
  }

  static inline bool fits(Context &c) noexcept
  {
    return c.packet.size() <= c.owner.mtu();
  }

  static inline bool enqueue(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock_);
    if (c.owner.count_ == 16)
    {
      return false;
    }
    c.owner.packets_[(c.owner.head_ + c.owner.count_) % 16] = {
        c.packet.release(), c.packet.size(), c.packet.context()};
    ++c.owner.count_;
    return true;
  }

  static inline void unreachable(Context &c) noexcept
  {
    c.result = -ENETUNREACH;
  }

  static inline void too_large(Context &c) noexcept
  {
    c.result = -EMSGSIZE;
  }

  static inline void full(Context &c) noexcept
  {
    c.result = -ENOBUFS;
  }
  struct Online;
  struct Size;
  struct Queue;
  struct Unreachable;
  struct TooLarge;
  struct Full;
  using Graph = flow::graph<
      Online,
      flow::types<flow::node<Online, flow::noop,
                             flow::branch<flow::condition<&online>, flow::go<Size>,
                                          flow::go<Unreachable>>>,
                  flow::node<Size, flow::noop,
                             flow::branch<flow::condition<&fits>, flow::go<Queue>,
                                          flow::go<TooLarge>>>,
                  flow::node<Queue, flow::noop,
                             flow::branch<flow::condition<&enqueue>, flow::stop,
                                          flow::go<Full>>>,
                  flow::node<Unreachable, flow::action<&unreachable>, flow::stop>,
                  flow::node<TooLarge, flow::action<&too_large>, flow::stop>,
                  flow::node<Full, flow::action<&full>, flow::stop>>>;
};

struct LoopbackInterface::DrainFlow
{
  struct Context
  {
    LoopbackInterface &owner;
    unsigned budget = 0;
    Pending pending{};
  };

  static inline bool within_budget(Context &c) noexcept
  {
    return c.budget < 64;
  }

  static inline bool dequeue(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock_);
    if (!c.owner.count_)
    {
      return false;
    }
    c.pending = c.owner.packets_[c.owner.head_];
    c.owner.packets_[c.owner.head_] = {};
    c.owner.head_ = (c.owner.head_ + 1) % 16;
    --c.owner.count_;
    return true;
  }

  static inline void deliver(Context &c) noexcept
  {
    Packet packet(c.pending.page, c.pending.length, c.pending.context);
    c.pending = {};
    c.owner.deliver(packet);
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.budget;
  }
  struct Budget;
  struct Dequeue;
  struct Deliver;
  struct Advance;
  using Graph = flow::graph<
      Budget,
      flow::types<flow::node<Budget, flow::noop,
                             flow::branch<flow::condition<&within_budget>,
                                          flow::go<Dequeue>, flow::stop>>,
                  flow::node<Dequeue, flow::noop,
                             flow::branch<flow::condition<&dequeue>, flow::go<Deliver>,
                                          flow::stop>>,
                  flow::node<Deliver, flow::action<&deliver>, flow::go<Advance>>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Budget>>>>;
};

} // namespace xv::net
