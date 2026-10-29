module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:arp_flows;
import :arp;
import :wire;
import :platform;

namespace xv::net
{
namespace flow = xv::flow;

struct ArpCache::SendFlow
{
  struct Context
  {
    ArpCache &owner;
    Packet &packet;
    unsigned next_hop;
    unsigned generation = 0;
    unsigned char mac[6]{};
    unsigned resolution = 0;
    unsigned long now = 0;
    int result = 0;
  };

  static inline bool valid_interface(Context &c) noexcept
  {
    ConfigurationGuard guard;
    if (!c.owner.interface_.up() || c.packet.size() < 20 ||
        !c.owner.interface_.owns(wire::read32(c.packet.data() + 12)))
    {
      return false;
    }
    c.generation = c.owner.interface_.generation();
    return true;
  }

  static inline bool find_neighbor(Context &c) noexcept
  {
    auto now = c.now;
    for (auto &entry : c.owner.entries_)
    {
      if (entry.address == c.next_hop && entry.expires > now)
      {
        wire::copy(c.mac, entry.mac, 6);
        c.resolution = 1;
        return true;
      }
    }
    return false;
  }

  static inline void queue(Context &c) noexcept
  {
    auto now = c.now;
    for (auto &pending : c.owner.pending_)
    {
      if (!pending.page)
      {
        pending = {c.packet.release(), c.packet.size(), c.next_hop,        1,
                   now + 1000,         c.generation,    c.packet.context()};
        c.resolution = 2;
        return;
      }
    }
  }
  struct Cached;
  struct Queue;
  using ResolutionGraph =
      flow::graph<Cached,
                  flow::types<flow::node<Cached, flow::noop,
                                         flow::branch<flow::condition<&find_neighbor>,
                                                      flow::stop, flow::go<Queue>>>,
                              flow::node<Queue, flow::action<&queue>, flow::stop>>>;

  static inline void resolve(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock_);
    c.now = detail::milliseconds();
    (void)ResolutionGraph::run<0>(c);
  }

  static inline bool cached(Context &c) noexcept
  {
    return c.resolution == 1;
  }

  static inline bool queued(Context &c) noexcept
  {
    return c.resolution == 2;
  }

  static inline void send(Context &c) noexcept
  {
    c.result = c.owner.link_.send_ip(c.packet, c.mac);
  }

  static inline void request(Context &c) noexcept
  {
    c.owner.request(c.next_hop);
  }

  static inline void unreachable(Context &c) noexcept
  {
    c.result = -ENETUNREACH;
  }

  static inline void full(Context &c) noexcept
  {
    c.result = -ENOBUFS;
  }
  struct Interface;
  struct Resolve;
  struct Queued;
  struct Send;
  struct Request;
  struct Unreachable;
  struct Full;
  using Graph = flow::graph<
      Interface,
      flow::types<flow::node<Interface, flow::noop,
                             flow::branch<flow::condition<&valid_interface>,
                                          flow::go<Resolve>, flow::go<Unreachable>>>,
                  flow::node<Resolve, flow::action<&resolve>,
                             flow::branch<flow::condition<&cached>, flow::go<Send>,
                                          flow::go<Queued>>>,
                  flow::node<Queued, flow::noop,
                             flow::branch<flow::condition<&queued>, flow::go<Request>,
                                          flow::go<Full>>>,
                  flow::node<Send, flow::action<&send>, flow::stop>,
                  flow::node<Request, flow::action<&request>, flow::stop>,
                  flow::node<Unreachable, flow::action<&unreachable>, flow::stop>,
                  flow::node<Full, flow::action<&full>, flow::stop>>>;
};

struct ArpCache::ReceiveFlow
{
  struct Context
  {
    ArpCache &owner;
    Packet &packet;
    unsigned char *bytes = nullptr;
    unsigned operation = 0, source = 0, index = 0;
  };

  static inline bool eligible(Context &c) noexcept
  {
    return c.owner.interface_.up() && c.packet.size() >= 42;
  }

  static inline void inspect(Context &c) noexcept
  {
    c.bytes = c.packet.data();
    c.operation = wire::read16(c.bytes + 20);
    c.source = wire::read32(c.bytes + 28);
  }

  static inline bool valid_header(Context &c) noexcept
  {
    auto *p = c.bytes;
    return wire::read16(p + 14) == 1 && wire::read16(p + 16) == 0x800 && p[18] == 6 &&
           p[19] == 4 && (c.operation == 1 || c.operation == 2) &&
           wire::equal(p + 6, p + 22, 6) && !(p[22] & 1) &&
           c.owner.interface_.owns(wire::read32(p + 38));
  }

  static inline bool valid_source(Context &c) noexcept
  {
    return c.source && !c.owner.interface_.owns(c.source) && c.source != 0xffffffff;
  }

  static inline void learn(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock_);
    auto *slot = &c.owner.entries_[0];
    for (auto &entry : c.owner.entries_)
    {
      if (entry.address == c.source)
      {
        slot = &entry;
        break;
      }
      if (entry.expires < slot->expires)
      {
        slot = &entry;
      }
    }
    *slot = {c.source, {}, detail::milliseconds() + 60000};
    wire::copy(slot->mac, c.bytes + 22, 6);
  }

  static inline bool more_pending(Context &c) noexcept
  {
    return c.index < sizeof(c.owner.pending_) / sizeof(c.owner.pending_[0]);
  }

  static inline void flush_one(Context &c) noexcept
  {
    auto &queued = c.owner.pending_[c.index];
    c.owner.lock_.acquire();
    if (!queued.page || queued.address != c.source)
    {
      c.owner.lock_.release();
      return;
    }
    bool valid =
        queued.generation == c.owner.interface_.generation() && c.owner.interface_.up();
    Packet ready(queued.page, queued.length, queued.context);
    queued = {};
    c.owner.lock_.release();
    if (valid)
    {
      c.owner.link_.send_ip(ready, c.bytes + 22);
    }
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }

  static inline bool request(Context &c) noexcept
  {
    return c.operation == 1;
  }

  static inline void reply(Context &c) noexcept
  {
    auto packet = Packet::allocate(42);
    if (!packet)
    {
      return;
    }
    auto *p = c.bytes;
    wire::copy(packet.data() + 14, p + 14, 28);
    wire::write16(packet.data() + 20, 2);
    wire::copy(packet.data() + 32, p + 22, 6);
    wire::write32(packet.data() + 38, c.source);
    wire::copy(packet.data() + 22, c.owner.link_.mac().bytes, 6);
    wire::write32(packet.data() + 28, wire::read32(p + 38));
    c.owner.link_.send(packet, p + 22, 0x806);
  }
  struct Eligible;
  struct Header;
  struct Source;
  struct Learn;
  struct Pending;
  struct Flush;
  struct Advance;
  struct Request;
  struct Reply;
  using Graph = flow::graph<
      Eligible,
      flow::types<flow::node<Eligible, flow::noop,
                             flow::branch<flow::condition<&eligible>, flow::go<Header>,
                                          flow::stop>>,
                  flow::node<Header, flow::action<&inspect>,
                             flow::branch<flow::condition<&valid_header>,
                                          flow::go<Source>, flow::stop>>,
                  flow::node<Source, flow::noop,
                             flow::branch<flow::condition<&valid_source>,
                                          flow::go<Learn>, flow::stop>>,
                  flow::node<Learn, flow::action<&learn>, flow::go<Pending>>,
                  flow::node<Pending, flow::noop,
                             flow::branch<flow::condition<&more_pending>,
                                          flow::go<Flush>, flow::go<Request>>>,
                  flow::node<Flush, flow::action<&flush_one>, flow::go<Advance>>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Pending>>,
                  flow::node<Request, flow::noop,
                             flow::branch<flow::condition<&request>, flow::go<Reply>,
                                          flow::stop>>,
                  flow::node<Reply, flow::action<&reply>, flow::stop>>>;
};

struct ArpCache::TickFlow
{
  struct Context
  {
    ArpCache &owner;
    unsigned long now;
    unsigned index = 0, address = 0;
    bool exhausted = false;
    bool scheduled = false;
  };

  static inline void invalidate(Context &c) noexcept
  {
    auto &p = c.owner.pending_[c.index];
    if (p.page &&
        (p.generation != c.owner.interface_.generation() || !c.owner.interface_.up()))
    {
      Packet expired(p.page, p.length);
      p = {};
    }
  }

  static inline bool due(Context &c) noexcept
  {
    auto &p = c.owner.pending_[c.index];
    return p.page && p.deadline <= c.now;
  }

  static inline void count_retry(Context &c) noexcept
  {
    c.exhausted = c.owner.pending_[c.index].retries++ >= 3;
  }

  static inline bool exhausted(Context &c) noexcept
  {
    return c.exhausted;
  }

  static inline void expire(Context &c) noexcept
  {
    auto &p = c.owner.pending_[c.index];
    Packet expired(p.page, p.length);
    p = {};
  }

  static inline void schedule(Context &c) noexcept
  {
    auto &p = c.owner.pending_[c.index];
    c.address = p.address;
    c.scheduled = true;
    p.deadline = c.now + 1000;
  }
  struct Invalidate;
  struct CountRetry;
  struct Expire;
  struct Schedule;
  using PendingGraph = flow::graph<
      Invalidate,
      flow::types<flow::node<Invalidate, flow::action<&invalidate>,
                             flow::branch<flow::condition<&due>, flow::go<CountRetry>,
                                          flow::stop>>,
                  flow::node<CountRetry, flow::action<&count_retry>,
                             flow::branch<flow::condition<&exhausted>, flow::go<Expire>,
                                          flow::go<Schedule>>>,
                  flow::node<Expire, flow::action<&expire>, flow::stop>,
                  flow::node<Schedule, flow::action<&schedule>, flow::stop>>>;

  static inline bool more(Context &c) noexcept
  {
    return c.index < sizeof(c.owner.pending_) / sizeof(c.owner.pending_[0]);
  }

  static inline void inspect(Context &c) noexcept
  {
    c.scheduled = false;
    xv::sync::ScopedLock guard(c.owner.lock_);
    (void)PendingGraph::run<0>(c);
  }

  static inline bool retry(Context &c) noexcept
  {
    return c.scheduled;
  }

  static inline void request(Context &c) noexcept
  {
    c.owner.request(c.address);
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }
  struct Scan;
  struct Inspect;
  struct Request;
  struct Advance;
  using Graph = flow::graph<
      Scan, flow::types<flow::node<Scan, flow::noop,
                                   flow::branch<flow::condition<&more>,
                                                flow::go<Inspect>, flow::stop>>,
                        flow::node<Inspect, flow::action<&inspect>,
                                   flow::branch<flow::condition<&retry>,
                                                flow::go<Request>, flow::go<Advance>>>,
                        flow::node<Request, flow::action<&request>, flow::go<Advance>>,
                        flow::node<Advance, flow::action<&advance>, flow::go<Scan>>>>;
};

} // namespace xv::net
