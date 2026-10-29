module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:reassembly_accept_flow;
import :reassembly;
import :wire;
import :platform;

namespace xv::net
{
namespace flow = xv::flow;

struct Ipv4Reassembly::AcceptFlow
{
  struct Context
  {
    Ipv4Reassembly &owner;
    const void *ingress;
    const Packet &fragment;
    unsigned header_length, total_length;
    unsigned long now;
    const unsigned char *header = nullptr;
    unsigned flags = 0, offset = 0, length = 0;
    Key key{};
    Assembly *assembly = nullptr;
    bool inserted = false;
    void *page = nullptr;
    unsigned complete_length = 0;
    Packet::Context context{};
  };

  static inline bool valid_input(Context &c) noexcept
  {
    return c.fragment && c.header_length == 20 && c.total_length >= c.header_length &&
           c.total_length <= c.fragment.size();
  }

  static inline void inspect(Context &c) noexcept
  {
    c.header = c.fragment.data();
    c.flags = wire::read16(c.header + 6);
    c.offset = (c.flags & 0x1fff) * 8;
    c.length = c.total_length - c.header_length;
    c.key = {c.ingress, wire::read32(c.header + 12), wire::read32(c.header + 16),
             wire::read16(c.header + 4), c.header[9]};
  }

  static inline bool valid_fragment(Context &c) noexcept
  {
    return c.length && c.offset + c.length <= MaxPayload &&
           (!(c.flags & 0x2000) || !(c.length & 7));
  }

  static inline void locate(Context &c) noexcept
  {
    c.assembly = c.owner.find_or_create(c.key, c.fragment, c.now);
  }

  static inline bool found(Context &c) noexcept
  {
    return c.assembly != nullptr;
  }

  static inline void insert(Context &c) noexcept
  {
    c.inserted = c.owner.insert(*c.assembly, c.header + c.header_length, c.offset,
                                c.length, !(c.flags & 0x2000));
  }

  static inline bool waiting(Context &c) noexcept
  {
    return c.inserted &&
           !(c.assembly->last && c.assembly->received == c.assembly->total);
  }

  static inline void detach(Context &c) noexcept
  {
    c.page = c.assembly->page;
    c.context = c.assembly->context;
    if (c.inserted)
    {
      c.complete_length = 20 + c.assembly->total;
    }
    *c.assembly = {};
  }
  struct Locate;
  struct Insert;
  struct Detach;
  using LockedGraph =
      flow::graph<Locate,
                  flow::types<flow::node<Locate, flow::action<&locate>,
                                         flow::branch<flow::condition<&found>,
                                                      flow::go<Insert>, flow::stop>>,
                              flow::node<Insert, flow::action<&insert>,
                                         flow::branch<flow::condition<&waiting>,
                                                      flow::stop, flow::go<Detach>>>,
                              flow::node<Detach, flow::action<&detach>, flow::stop>>>;

  static inline void assemble(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock_);
    (void)LockedGraph::run<0>(c);
  }

  static inline bool detached(Context &c) noexcept
  {
    return c.page != nullptr;
  }

  static inline bool complete(Context &c) noexcept
  {
    return c.complete_length != 0;
  }

  static inline void discard(Context &c) noexcept
  {
    Packet discarded(c.page, 0);
    c.page = nullptr;
    c.context = {};
  }

  static inline void repair(Context &c) noexcept
  {
    auto *ip = static_cast<unsigned char *>(c.page);
    wire::write16(ip + 2, c.complete_length);
    wire::write16(ip + 6, 0);
    wire::write16(ip + 10, 0);
    wire::write16(ip + 10, ~wire::sum(ip, 20));
  }
  struct Input;
  struct Header;
  struct Assemble;
  struct Complete;
  struct Repair;
  struct Discard;
  using Graph = flow::graph<
      Input, flow::types<flow::node<Input, flow::noop,
                                    flow::branch<flow::condition<&valid_input>,
                                                 flow::go<Header>, flow::stop>>,
                         flow::node<Header, flow::action<&inspect>,
                                    flow::branch<flow::condition<&valid_fragment>,
                                                 flow::go<Assemble>, flow::stop>>,
                         flow::node<Assemble, flow::action<&assemble>,
                                    flow::branch<flow::condition<&detached>,
                                                 flow::go<Complete>, flow::stop>>,
                         flow::node<Complete, flow::noop,
                                    flow::branch<flow::condition<&complete>,
                                                 flow::go<Repair>, flow::go<Discard>>>,
                         flow::node<Repair, flow::action<&repair>, flow::stop>,
                         flow::node<Discard, flow::action<&discard>, flow::stop>>>;
};

} // namespace xv::net
