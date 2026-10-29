module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:tcp_io_flow;
import :tcp_socket;

namespace xv::net
{
namespace flow = xv::flow;

struct TcpSocket::ConnectFlow
{
  struct Context
  {
    TcpSocket &owner;
    Endpoint peer;
    bool nonblocking;
    int result = 0;
    bool locked = false;
  };

  static inline bool valid_peer(Context &c) noexcept
  {
    return c.peer.address && c.peer.port;
  }

  static inline void lock(Context &c) noexcept
  {
    c.owner.registry_.lock().acquire();
    c.locked = true;
  }

  static inline bool closed(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::Closed;
  }

  static inline void state_error(Context &c) noexcept
  {
    c.result = c.owner.state_ == TcpState::SynSent ? -EALREADY : -EISCONN;
  }

  static inline void bind(Context &c) noexcept
  {
    c.result = c.owner.auto_bind(c.peer.address);
  }

  static inline bool bound(Context &c) noexcept
  {
    return c.result >= 0;
  }

  static inline void initialize(Context &c) noexcept
  {
    auto &s = c.owner;
    s.peer_ = c.peer;
    s.error_ = 0;
    s.state_ = TcpState::SynSent;
    s.peer_mss_ = 536;
    s.peer_window_ = 65535;
    s.receive_.reset(0);
    s.congestion_.initialize(s.peer_mss_);
    s.persist_deadline_ = 0;
    s.persist_interval_ = 1000;
    s.synchronize(s.registry_.initial_sequence());
  }

  static inline bool nonblocking(Context &c) noexcept
  {
    return c.nonblocking;
  }

  static inline void in_progress(Context &c) noexcept
  {
    c.result = -EINPROGRESS;
  }

  static inline bool waiting(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::SynSent ||
           c.owner.state_ == TcpState::SynReceived;
  }

  static inline bool interrupted(Context &) noexcept
  {
    return detail::interrupted();
  }

  static inline void wait(Context &c) noexcept
  {
    c.owner.wait();
  }

  static inline void completed(Context &c) noexcept
  {
    c.result = c.owner.state_ == TcpState::Established
                   ? 0
                   : -(c.owner.error_ ? c.owner.error_ : ECONNREFUSED);
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -1;
  }

  static inline void unlock(Context &c) noexcept
  {
    if (c.locked)
    {
      c.owner.registry_.lock().release();
      c.locked = false;
    }
  }
  struct Peer;
  struct Lock;
  struct StateError;
  struct Bind;
  struct Initialize;
  struct Progress;
  struct Waiting;
  struct Interrupted;
  struct Wait;
  struct Completed;
  struct Invalid;
  struct Unlock;
  using Graph = flow::graph<
      Peer,
      flow::types<flow::node<Peer, flow::noop,
                             flow::branch<flow::condition<&valid_peer>, flow::go<Lock>,
                                          flow::go<Invalid>>>,
                  flow::node<Lock, flow::action<&lock>,
                             flow::branch<flow::condition<&closed>, flow::go<Bind>,
                                          flow::go<StateError>>>,
                  flow::node<StateError, flow::action<&state_error>, flow::go<Unlock>>,
                  flow::node<Bind, flow::action<&bind>,
                             flow::branch<flow::condition<&bound>, flow::go<Initialize>,
                                          flow::go<Unlock>>>,
                  flow::node<Initialize, flow::action<&initialize>,
                             flow::branch<flow::condition<&nonblocking>,
                                          flow::go<Progress>, flow::go<Waiting>>>,
                  flow::node<Progress, flow::action<&in_progress>, flow::go<Unlock>>,
                  flow::node<Waiting, flow::noop,
                             flow::branch<flow::condition<&waiting>,
                                          flow::go<Interrupted>, flow::go<Completed>>>,
                  flow::node<Interrupted, flow::noop,
                             flow::branch<flow::condition<&interrupted>,
                                          flow::go<Invalid>, flow::go<Wait>>>,
                  flow::node<Wait, flow::action<&wait>, flow::go<Waiting>>,
                  flow::node<Completed, flow::action<&completed>, flow::go<Unlock>>,
                  flow::node<Invalid, flow::action<&invalid>, flow::go<Unlock>>,
                  flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

struct TcpSocket::AcceptFlow
{
  struct Context
  {
    TcpSocket &owner;
    bool nonblocking;
    int &error;
    Socket *child = nullptr;
  };

  static inline bool listening(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::Listen;
  }

  static inline void accept(Context &c) noexcept
  {
    c.child = c.owner.registry_.accept_child(c.owner);
  }

  static inline bool accepted(Context &c) noexcept
  {
    return c.child != nullptr;
  }

  static inline bool interrupted(Context &) noexcept
  {
    return detail::interrupted();
  }

  static inline bool nonblocking(Context &c) noexcept
  {
    return c.nonblocking;
  }

  static inline void wait(Context &c) noexcept
  {
    c.owner.wait();
  }

  static inline void invalid(Context &c) noexcept
  {
    c.error = -1;
  }

  static inline void again(Context &c) noexcept
  {
    c.error = -EAGAIN;
  }

  static inline void success(Context &c) noexcept
  {
    c.error = 0;
  }

  static inline void unlock(Context &c) noexcept
  {
    c.owner.registry_.lock().release();
  }
  struct Listening;
  struct Accept;
  struct Interrupted;
  struct Nonblocking;
  struct Wait;
  struct Invalid;
  struct Again;
  struct Success;
  struct Unlock;
  using Graph = flow::graph<
      Listening,
      flow::types<flow::node<Listening, flow::noop,
                             flow::branch<flow::condition<&listening>, flow::go<Accept>,
                                          flow::go<Invalid>>>,
                  flow::node<Accept, flow::action<&accept>,
                             flow::branch<flow::condition<&accepted>, flow::go<Success>,
                                          flow::go<Interrupted>>>,
                  flow::node<Interrupted, flow::noop,
                             flow::branch<flow::condition<&interrupted>,
                                          flow::go<Invalid>, flow::go<Nonblocking>>>,
                  flow::node<Nonblocking, flow::noop,
                             flow::branch<flow::condition<&nonblocking>,
                                          flow::go<Again>, flow::go<Wait>>>,
                  flow::node<Wait, flow::action<&wait>, flow::go<Listening>>,
                  flow::node<Invalid, flow::action<&invalid>, flow::go<Unlock>>,
                  flow::node<Again, flow::action<&again>, flow::go<Unlock>>,
                  flow::node<Success, flow::action<&success>, flow::go<Unlock>>,
                  flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

struct TcpSocket::ReadFlow
{
  struct Context
  {
    TcpSocket &owner;
    xv::io::Buffer destination;
    int count;
    unsigned char staged[1024];
    unsigned amount = 0;
    int result = 0;
    bool locked = false;

    Context(TcpSocket &socket, xv::io::Buffer buffer, int size)
        : owner(socket), destination(buffer), count(size)
    {
    }
  };

  static inline bool negative(Context &c) noexcept
  {
    return c.count < 0;
  }

  static inline bool empty(Context &c) noexcept
  {
    return c.count == 0;
  }

  static inline void lock(Context &c) noexcept
  {
    c.owner.registry_.lock().acquire();
    c.locked = true;
  }

  static inline bool readable(Context &c) noexcept
  {
    return c.owner.receive_.readable();
  }

  static inline void read(Context &c) noexcept
  {
    unsigned amount = static_cast<unsigned>(c.count);
    if (amount > sizeof(c.staged))
    {
      amount = sizeof(c.staged);
    }
    unsigned before = c.owner.window();
    c.amount = c.owner.receive_.read(c.staged, amount);
    unsigned after = c.owner.window();
    if (!before || after == TcpPageBuffer::Capacity ||
        c.owner.receive_.next() + after - c.owner.advertised_right_ >=
            c.owner.peer_mss_)
    {
      c.owner.emit(16);
    }
  }

  static inline void unlock(Context &c) noexcept
  {
    if (c.locked)
    {
      c.owner.registry_.lock().release();
      c.locked = false;
    }
  }

  static inline void copy_out(Context &c) noexcept
  {
    c.result = detail::copy_out(c.destination.address, c.staged, c.amount) < 0
                   ? -1
                   : static_cast<int>(c.amount);
  }

  static inline bool eof(Context &c) noexcept
  {
    return c.owner.read_shutdown_ || c.owner.receive_.finished();
  }

  static inline bool error(Context &c) noexcept
  {
    return c.owner.error_ != 0;
  }

  static inline void socket_error(Context &c) noexcept
  {
    c.result = -c.owner.error_;
  }

  static inline bool connected(Context &c) noexcept
  {
    auto state = c.owner.state_;
    return state == TcpState::Established || state == TcpState::FinWait1 ||
           state == TcpState::FinWait2;
  }

  static inline void not_connected(Context &c) noexcept
  {
    c.result = -ENOTCONN;
  }

  static inline bool interrupted(Context &) noexcept
  {
    return detail::interrupted();
  }

  static inline bool nonblocking(Context &c) noexcept
  {
    return c.destination.nonblocking;
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -1;
  }

  static inline void again(Context &c) noexcept
  {
    c.result = -EAGAIN;
  }

  static inline void wait(Context &c) noexcept
  {
    c.owner.wait();
  }
  struct Count;
  struct Zero;
  struct Lock;
  struct Readable;
  struct Read;
  struct UnlockCopy;
  struct Copy;
  struct Eof;
  struct Error;
  struct SocketError;
  struct Connected;
  struct NotConnected;
  struct Interrupted;
  struct Nonblocking;
  struct Wait;
  struct Invalid;
  struct Again;
  struct Unlock;
  using Graph = flow::graph<
      Count,
      flow::types<
          flow::node<Count, flow::noop,
                     flow::branch<flow::condition<&negative>, flow::go<Invalid>,
                                  flow::go<Zero>>>,
          flow::node<Zero, flow::noop,
                     flow::branch<flow::condition<&empty>, flow::stop, flow::go<Lock>>>,
          flow::node<Lock, flow::action<&lock>, flow::go<Readable>>,
          flow::node<
              Readable, flow::noop,
              flow::branch<flow::condition<&readable>, flow::go<Read>, flow::go<Eof>>>,
          flow::node<Read, flow::action<&read>, flow::go<UnlockCopy>>,
          flow::node<UnlockCopy, flow::action<&unlock>, flow::go<Copy>>,
          flow::node<Copy, flow::action<&copy_out>, flow::stop>,
          flow::node<
              Eof, flow::noop,
              flow::branch<flow::condition<&eof>, flow::go<Unlock>, flow::go<Error>>>,
          flow::node<Error, flow::noop,
                     flow::branch<flow::condition<&error>, flow::go<SocketError>,
                                  flow::go<Connected>>>,
          flow::node<SocketError, flow::action<&socket_error>, flow::go<Unlock>>,
          flow::node<Connected, flow::noop,
                     flow::branch<flow::condition<&connected>, flow::go<Interrupted>,
                                  flow::go<NotConnected>>>,
          flow::node<NotConnected, flow::action<&not_connected>, flow::go<Unlock>>,
          flow::node<Interrupted, flow::noop,
                     flow::branch<flow::condition<&interrupted>, flow::go<Invalid>,
                                  flow::go<Nonblocking>>>,
          flow::node<Nonblocking, flow::noop,
                     flow::branch<flow::condition<&nonblocking>, flow::go<Again>,
                                  flow::go<Wait>>>,
          flow::node<Wait, flow::action<&wait>, flow::go<Readable>>,
          flow::node<Invalid, flow::action<&invalid>, flow::go<Unlock>>,
          flow::node<Again, flow::action<&again>, flow::go<Unlock>>,
          flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

struct TcpSocket::WriteFlow
{
  struct Context
  {
    TcpSocket &owner;
    xv::io::Buffer source;
    int count;
    unsigned char staged[1024];
    unsigned amount = 0;
    int result = 0;
    bool locked = false;

    Context(TcpSocket &socket, xv::io::Buffer buffer, int size)
        : owner(socket), source(buffer), count(size)
    {
    }
  };

  static inline bool negative(Context &c) noexcept
  {
    return c.count < 0;
  }

  static inline bool empty(Context &c) noexcept
  {
    return c.count == 0;
  }

  static inline void stage(Context &c) noexcept
  {
    c.amount = static_cast<unsigned>(c.count);
    if (c.amount > sizeof(c.staged))
    {
      c.amount = sizeof(c.staged);
    }
  }

  static inline bool copied(Context &c) noexcept
  {
    return detail::copy_in(c.staged, c.source.address, c.amount) >= 0;
  }

  static inline void lock(Context &c) noexcept
  {
    c.owner.registry_.lock().acquire();
    c.locked = true;
  }

  static inline bool shut(Context &c) noexcept
  {
    return c.owner.write_shutdown_;
  }

  static inline bool error(Context &c) noexcept
  {
    return c.owner.error_ != 0;
  }

  static inline bool connected(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::Established ||
           c.owner.state_ == TcpState::CloseWait;
  }

  static inline bool space(Context &c) noexcept
  {
    return c.owner.transmit_.space() != 0;
  }

  static inline bool routable(Context &c) noexcept
  {
    return c.owner.mss() != 0;
  }

  static inline void append(Context &c) noexcept
  {
    auto &s = c.owner;
    if (!s.transmit_.size() && s.send_next_ == s.send_unacknowledged_)
    {
      s.last_activity_ = detail::milliseconds();
    }
    c.result = s.transmit_.append(c.staged, c.amount);
    s.pump();
  }

  static inline bool interrupted(Context &) noexcept
  {
    return detail::interrupted();
  }

  static inline bool nonblocking(Context &c) noexcept
  {
    return c.source.nonblocking;
  }

  static inline void wait(Context &c) noexcept
  {
    c.owner.wait();
  }

  static inline void pipe(Context &c) noexcept
  {
    c.result = -EPIPE;
  }

  static inline void socket_error(Context &c) noexcept
  {
    c.result = -c.owner.error_;
  }

  static inline void not_connected(Context &c) noexcept
  {
    c.result = -ENOTCONN;
  }

  static inline void unreachable(Context &c) noexcept
  {
    c.result = -ENETUNREACH;
  }

  static inline void invalid(Context &c) noexcept
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
  struct Count;
  struct Zero;
  struct Stage;
  struct Lock;
  struct Shutdown;
  struct Error;
  struct Connected;
  struct Space;
  struct Route;
  struct Append;
  struct Interrupted;
  struct Nonblocking;
  struct Wait;
  struct Pipe;
  struct SocketError;
  struct NotConnected;
  struct Unreachable;
  struct Invalid;
  struct Again;
  struct Unlock;
  using Graph = flow::graph<
      Count,
      flow::types<
          flow::node<Count, flow::noop,
                     flow::branch<flow::condition<&negative>, flow::go<Invalid>,
                                  flow::go<Zero>>>,
          flow::node<
              Zero, flow::noop,
              flow::branch<flow::condition<&empty>, flow::stop, flow::go<Stage>>>,
          flow::node<Stage, flow::action<&stage>,
                     flow::branch<flow::condition<&copied>, flow::go<Lock>,
                                  flow::go<Invalid>>>,
          flow::node<Lock, flow::action<&lock>, flow::go<Shutdown>>,
          flow::node<
              Shutdown, flow::noop,
              flow::branch<flow::condition<&shut>, flow::go<Pipe>, flow::go<Error>>>,
          flow::node<Error, flow::noop,
                     flow::branch<flow::condition<&error>, flow::go<SocketError>,
                                  flow::go<Connected>>>,
          flow::node<Connected, flow::noop,
                     flow::branch<flow::condition<&connected>, flow::go<Space>,
                                  flow::go<NotConnected>>>,
          flow::node<Space, flow::noop,
                     flow::branch<flow::condition<&space>, flow::go<Route>,
                                  flow::go<Interrupted>>>,
          flow::node<Route, flow::noop,
                     flow::branch<flow::condition<&routable>, flow::go<Append>,
                                  flow::go<Unreachable>>>,
          flow::node<Append, flow::action<&append>, flow::go<Unlock>>,
          flow::node<Interrupted, flow::noop,
                     flow::branch<flow::condition<&interrupted>, flow::go<Invalid>,
                                  flow::go<Nonblocking>>>,
          flow::node<Nonblocking, flow::noop,
                     flow::branch<flow::condition<&nonblocking>, flow::go<Again>,
                                  flow::go<Wait>>>,
          flow::node<Wait, flow::action<&wait>, flow::go<Shutdown>>,
          flow::node<Pipe, flow::action<&pipe>, flow::go<Unlock>>,
          flow::node<SocketError, flow::action<&socket_error>, flow::go<Unlock>>,
          flow::node<NotConnected, flow::action<&not_connected>, flow::go<Unlock>>,
          flow::node<Unreachable, flow::action<&unreachable>, flow::go<Unlock>>,
          flow::node<Invalid, flow::action<&invalid>, flow::go<Unlock>>,
          flow::node<Again, flow::action<&again>, flow::go<Unlock>>,
          flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

} // namespace xv::net
