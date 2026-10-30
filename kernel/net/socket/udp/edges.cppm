module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:udp_socket_edges;
import :udp_socket;

namespace xv::net
{
namespace flow = xv::flow;

struct DatagramSocket::ConnectFlow
{
  struct Context
  {
    DatagramSocket &owner;
    Endpoint peer;
    int result = 0;
  };

  static inline bool valid(Context &c) noexcept
  {
    return c.peer.address && (c.peer.port || c.owner.protocol() == 1);
  }

  static inline void bind(Context &c) noexcept
  {
    c.result = c.owner.auto_bind(c.peer.address);
  }

  static inline bool bound(Context &c) noexcept
  {
    return c.result >= 0;
  }

  static inline void connect(Context &c) noexcept
  {
    if (!c.owner.local_.address)
    {
      c.owner.local_.address = c.owner.registry_.ip().source_for(c.peer.address);
    }
    c.owner.peer_ = c.peer;
    c.result = 0;
  }
  struct Bind;
  struct Connect;
  using LockedGraph =
      flow::graph<Bind,
                  flow::types<flow::node<Bind, flow::action<&bind>,
                                         flow::branch<flow::condition<&bound>,
                                                      flow::go<Connect>, flow::stop>>,
                              flow::node<Connect, flow::action<&connect>, flow::stop>>>;

  static inline void locked(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.registry_.lock());
    (void)LockedGraph::run<0>(c);
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -1;
  }
  struct Peer;
  struct Locked;
  struct Invalid;
  using Graph = flow::graph<
      Peer, flow::types<flow::node<Peer, flow::noop,
                                   flow::branch<flow::condition<&valid>,
                                                flow::go<Locked>, flow::go<Invalid>>>,
                        flow::node<Locked, flow::action<&locked>, flow::stop>,
                        flow::node<Invalid, flow::action<&invalid>, flow::stop>>>;
};

struct DatagramSocket::DeliveryFlow
{
  struct Context
  {
    DatagramSocket &owner;
    Packet &packet;
    Endpoint source, destination;
    unsigned offset, length;
    bool result = false;
  };

  static inline bool matches(Context &c) noexcept
  {
    auto &s = c.owner;
    return s.local_.port && s.local_.port == c.destination.port &&
           (!s.local_.address || s.local_.address == c.destination.address) &&
           (s.protocol() == 1 ? (!s.peer_.address || s.peer_.address == c.source.address)
                            : (!s.peer_.port ||
                               (s.peer_.port == c.source.port && s.peer_.address == c.source.address)));
  }

  static inline void claim(Context &c) noexcept
  {
    c.result = true;
  }

  static inline bool space(Context &c) noexcept
  {
    return c.owner.count_ < 16;
  }

  static inline void queue(Context &c) noexcept
  {
    auto &s = c.owner;
    s.messages_[(s.head_ + s.count_) % 16] = {
        c.packet.release(), c.source, static_cast<unsigned short>(c.offset),
        static_cast<unsigned short>(c.length), c.packet.context()};
    ++s.count_;
    s.notify();
  }
  struct Match;
  struct Claim;
  struct Queue;
  using Graph =
      flow::graph<Match,
                  flow::types<flow::node<Match, flow::noop,
                                         flow::branch<flow::condition<&matches>,
                                                      flow::go<Claim>, flow::stop>>,
                              flow::node<Claim, flow::action<&claim>,
                                         flow::branch<flow::condition<&space>,
                                                      flow::go<Queue>, flow::stop>>,
                              flow::node<Queue, flow::action<&queue>, flow::stop>>>;
};

} // namespace xv::net
