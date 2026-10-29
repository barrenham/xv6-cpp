module;
#include "kernel/support/flow.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/net/errno.h"
module xv.net:udp_socket_flows;
import :udp_socket;

namespace xv::net
{
namespace flow = xv::flow;

struct UdpSocket::SendFlow
{
  struct Context
  {
    UdpSocket &owner;
    xv::io::Buffer data;
    unsigned length;
    const Endpoint *destination;
    Packet staged{};
    Endpoint peer{};
    int result = 0;
  };

  static inline bool fits(Context &c) noexcept
  {
    return c.length <= 4042;
  }

  static inline void allocate(Context &c) noexcept
  {
    c.staged.~Packet();
    new (&c.staged) Packet(Packet::allocate(c.length));
  }

  static inline bool allocated(Context &c) noexcept
  {
    return bool(c.staged);
  }

  static inline bool copy_in(Context &c) noexcept
  {
    return detail::copy_in(c.staged.data(), c.data.address, c.length) >= 0;
  }

  static inline void select_peer(Context &c) noexcept
  {
    c.peer = c.destination ? *c.destination : c.owner.peer_;
  }

  static inline bool connected(Context &c) noexcept
  {
    return c.peer.address && c.peer.port;
  }

  static inline void bind(Context &c) noexcept
  {
    c.result = c.owner.auto_bind(c.peer.address);
  }

  static inline bool bound(Context &c) noexcept
  {
    return c.result >= 0;
  }

  static inline void send(Context &c) noexcept
  {
    c.result =
        c.owner.registry_.udp().send(c.owner.local_, c.peer, c.staged.data(), c.length);
    if (c.result >= 0)
    {
      c.result = static_cast<int>(c.length);
    }
  }

  static inline void not_connected(Context &c) noexcept
  {
    c.result = -ENOTCONN;
  }
  struct Peer;
  struct Bind;
  struct Send;
  struct NotConnected;
  using LockedGraph = flow::graph<
      Peer,
      flow::types<
          flow::node<Peer, flow::action<&select_peer>,
                     flow::branch<flow::condition<&connected>, flow::go<Bind>,
                                  flow::go<NotConnected>>>,
          flow::node<Bind, flow::action<&bind>,
                     flow::branch<flow::condition<&bound>, flow::go<Send>, flow::stop>>,
          flow::node<Send, flow::action<&send>, flow::stop>,
          flow::node<NotConnected, flow::action<&not_connected>, flow::stop>>>;

  static inline void send_locked(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.registry_.lock());
    (void)LockedGraph::run<0>(c);
  }

  static inline void too_large(Context &c) noexcept
  {
    c.result = -EMSGSIZE;
  }

  static inline void no_buffer(Context &c) noexcept
  {
    c.result = -ENOBUFS;
  }

  static inline void bad_copy(Context &c) noexcept
  {
    c.result = -1;
  }
  struct Size;
  struct Allocate;
  struct Copy;
  struct SendLocked;
  struct TooLarge;
  struct NoBuffer;
  struct BadCopy;
  using Graph = flow::graph<
      Size,
      flow::types<flow::node<Size, flow::noop,
                             flow::branch<flow::condition<&fits>, flow::go<Allocate>,
                                          flow::go<TooLarge>>>,
                  flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&allocated>, flow::go<Copy>,
                                          flow::go<NoBuffer>>>,
                  flow::node<Copy, flow::noop,
                             flow::branch<flow::condition<&copy_in>,
                                          flow::go<SendLocked>, flow::go<BadCopy>>>,
                  flow::node<SendLocked, flow::action<&send_locked>, flow::stop>,
                  flow::node<TooLarge, flow::action<&too_large>, flow::stop>,
                  flow::node<NoBuffer, flow::action<&no_buffer>, flow::stop>,
                  flow::node<BadCopy, flow::action<&bad_copy>, flow::stop>>>;
};

struct UdpSocket::ReceiveFlow
{
  struct Context
  {
    UdpSocket &owner;
    xv::io::Buffer data;
    unsigned length;
    Endpoint *peer;
    Message message{};
    Packet packet{};
    unsigned copied = 0;
    bool locked = true;
    int result = 0;
  };

  static inline bool readable(Context &c) noexcept
  {
    return c.owner.count_ != 0;
  }

  static inline void dequeue(Context &c) noexcept
  {
    auto &s = c.owner;
    c.message = s.messages_[s.head_];
    s.messages_[s.head_] = {};
    s.head_ = (s.head_ + 1) % 16;
    --s.count_;
    s.registry_.lock().release();
    c.locked = false;
    c.packet.~Packet();
    new (&c.packet)
        Packet(c.message.page, c.message.offset + c.message.length, c.message.context);
    c.copied = c.message.length < c.length ? c.message.length : c.length;
  }

  static inline bool copy_out(Context &c) noexcept
  {
    return detail::copy_out(c.data.address, c.packet.data() + c.message.offset,
                            c.copied) >= 0;
  }

  static inline void success(Context &c) noexcept
  {
    if (c.peer)
    {
      *c.peer = c.message.source;
    }
    c.result = c.copied;
  }

  static inline bool interrupted(Context &) noexcept
  {
    return detail::interrupted();
  }

  static inline bool nonblocking(Context &c) noexcept
  {
    return c.data.nonblocking;
  }

  static inline void wait(Context &c) noexcept
  {
    c.owner.wait();
  }

  static inline void interrupted_error(Context &c) noexcept
  {
    c.result = -1;
  }

  static inline void again(Context &c) noexcept
  {
    c.result = -EAGAIN;
  }

  static inline void unlock(Context &c) noexcept
  {
    if (c.locked)
    {
      c.owner.registry_.lock().release();
      c.locked = false;
    }
  }
  struct Readable;
  struct Dequeue;
  struct Success;
  struct Interrupted;
  struct Nonblocking;
  struct Wait;
  struct BadCopy;
  struct Again;
  struct Unlock;
  using Graph = flow::graph<
      Readable,
      flow::types<
          flow::node<Readable, flow::noop,
                     flow::branch<flow::condition<&readable>, flow::go<Dequeue>,
                                  flow::go<Interrupted>>>,
          flow::node<Dequeue, flow::action<&dequeue>,
                     flow::branch<flow::condition<&copy_out>, flow::go<Success>,
                                  flow::go<BadCopy>>>,
          flow::node<Success, flow::action<&success>, flow::go<Unlock>>,
          flow::node<Interrupted, flow::noop,
                     flow::branch<flow::condition<&interrupted>, flow::go<BadCopy>,
                                  flow::go<Nonblocking>>>,
          flow::node<Nonblocking, flow::noop,
                     flow::branch<flow::condition<&nonblocking>, flow::go<Again>,
                                  flow::go<Wait>>>,
          flow::node<Wait, flow::action<&wait>, flow::go<Readable>>,
          flow::node<BadCopy, flow::action<&interrupted_error>, flow::go<Unlock>>,
          flow::node<Again, flow::action<&again>, flow::go<Unlock>>,
          flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

} // namespace xv::net
