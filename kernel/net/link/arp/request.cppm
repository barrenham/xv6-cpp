module;
#include "kernel/support/flow.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/net/errno.h"
module xv.net:arp_request_flow;
import :arp;
import :wire;

namespace xv::net
{
namespace flow = xv::flow;

struct ArpCache::RequestFlow
{
  struct Context
  {
    ArpCache &owner;
    unsigned address, source = 0;
    Packet packet{};
  };

  static inline void source(Context &c) noexcept
  {
    ConfigurationGuard guard;
    c.source = c.owner.interface_.up() ? c.owner.interface_.source_for(c.address) : 0;
  }

  static inline bool routed(Context &c) noexcept
  {
    return c.source != 0;
  }

  static inline void allocate(Context &c) noexcept
  {
    c.packet.~Packet();
    new (&c.packet) Packet(Packet::allocate(42));
  }

  static inline bool allocated(Context &c) noexcept
  {
    return bool(c.packet);
  }

  static inline void encode(Context &c) noexcept
  {
    auto *p = c.packet.data();
    wire::zero(p, 42);
    wire::write16(p + 14, 1);
    wire::write16(p + 16, 0x800);
    p[18] = 6;
    p[19] = 4;
    wire::write16(p + 20, 1);
    wire::copy(p + 22, c.owner.link_.mac().bytes, 6);
    wire::write32(p + 28, c.source);
    wire::write32(p + 38, c.address);
  }

  static inline void send(Context &c) noexcept
  {
    constexpr unsigned char broadcast[6] = {255, 255, 255, 255, 255, 255};
    c.owner.link_.send(c.packet, broadcast, 0x806);
  }
  struct Source;
  struct Allocate;
  struct Encode;
  struct Send;
  using Graph =
      flow::graph<Source,
                  flow::types<flow::node<Source, flow::action<&source>,
                                         flow::branch<flow::condition<&routed>,
                                                      flow::go<Allocate>, flow::stop>>,
                              flow::node<Allocate, flow::action<&allocate>,
                                         flow::branch<flow::condition<&allocated>,
                                                      flow::go<Encode>, flow::stop>>,
                              flow::node<Encode, flow::action<&encode>, flow::go<Send>>,
                              flow::node<Send, flow::action<&send>, flow::stop>>>;
};

} // namespace xv::net
