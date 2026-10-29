module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
#include "kernel/fs/poll.h"
module xv.net:tcp_lifecycle_flow;
import :tcp_socket;

namespace xv::net
{
namespace flow = xv::flow;

struct TcpSocket::ListenFlow
{
  struct Context
  {
    TcpSocket &owner;
    int backlog, result = 0;
  };

  static inline bool eligible(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::Closed || c.owner.state_ == TcpState::Listen;
  }

  static inline void choose_port(Context &c) noexcept
  {
    if (!c.owner.local_.port)
    {
      c.owner.local_.port = c.owner.registry_.ephemeral(6);
    }
  }

  static inline bool has_port(Context &c) noexcept
  {
    return c.owner.local_.port != 0;
  }

  static inline void listen(Context &c) noexcept
  {
    c.owner.backlog_ = c.backlog < 1 ? 1 : (c.backlog > 8 ? 8 : c.backlog);
    c.owner.state_ = TcpState::Listen;
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -1;
  }

  static inline void used(Context &c) noexcept
  {
    c.result = -EADDRINUSE;
  }
  struct State;
  struct Port;
  struct Listen;
  struct Invalid;
  struct Used;
  using Graph = flow::graph<
      State, flow::types<flow::node<State, flow::noop,
                                    flow::branch<flow::condition<&eligible>,
                                                 flow::go<Port>, flow::go<Invalid>>>,
                         flow::node<Port, flow::action<&choose_port>,
                                    flow::branch<flow::condition<&has_port>,
                                                 flow::go<Listen>, flow::go<Used>>>,
                         flow::node<Listen, flow::action<&listen>, flow::stop>,
                         flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                         flow::node<Used, flow::action<&used>, flow::stop>>>;
};

struct TcpSocket::PassiveFlow
{
  struct Context
  {
    TcpSocket &owner, &listener;
    Endpoint local, peer;
    const unsigned char *syn;
    unsigned length;
  };

  static inline void claim(Context &c) noexcept
  {
    auto &s = c.owner;
    s.owned_ = false;
    s.listener_ = &c.listener;
    s.local_ = c.local;
    s.peer_ = c.peer;
    s.peer_mss_ = tcp_peer_mss(c.syn, c.length);
  }

  static inline bool valid_mss(Context &c) noexcept
  {
    return c.owner.peer_mss_ != 0;
  }

  static inline void initialize(Context &c) noexcept
  {
    auto &s = c.owner;
    unsigned sequence = wire::read32(c.syn + 4);
    s.receive_.reset(sequence + 1);
    s.peer_window_ = wire::read16(c.syn + 14);
    s.window_sequence_ = sequence;
    s.window_ack_ = 0;
    unsigned maximum = s.mss();
    s.congestion_.initialize(maximum ? maximum : s.peer_mss_);
    s.state_ = TcpState::SynReceived;
  }

  static inline void synchronize(Context &c) noexcept
  {
    c.owner.synchronize(c.owner.registry_.initial_sequence());
  }

  static inline void reset(Context &c) noexcept
  {
    c.owner.fail(ECONNRESET);
  }
  struct Claim;
  struct Initialize;
  struct Synchronize;
  struct Reset;
  using Graph = flow::graph<
      Claim,
      flow::types<
          flow::node<Claim, flow::action<&claim>,
                     flow::branch<flow::condition<&valid_mss>, flow::go<Initialize>,
                                  flow::go<Reset>>>,
          flow::node<Initialize, flow::action<&initialize>, flow::go<Synchronize>>,
          flow::node<Synchronize, flow::action<&synchronize>, flow::stop>,
          flow::node<Reset, flow::action<&reset>, flow::stop>>>;
};

struct TcpSocket::PollFlow
{
  struct Context
  {
    TcpSocket &owner;
    xv::sync::PollTable *table;
    unsigned events = 0;
  };

  static inline void watch(Context &c) noexcept
  {
    if (c.table)
    {
      c.table->watch(&c.owner);
    }
    c.events = c.owner.error_ ? POLLERR : 0;
  }

  static inline bool listening(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::Listen;
  }

  static inline bool pending_child(Context &c) noexcept
  {
    return c.owner.registry_.has_ready_child(c.owner);
  }

  static inline bool readable(Context &c) noexcept
  {
    return c.owner.receive_.readable() || c.owner.receive_.finished() ||
           c.owner.read_shutdown_;
  }

  static inline bool writable(Context &c) noexcept
  {
    auto &s = c.owner;
    return (s.state_ == TcpState::Established || s.state_ == TcpState::CloseWait) &&
           !s.write_shutdown_ && s.transmit_.space();
  }

  static inline bool hung_up(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::Closed && (c.owner.peer_.port || c.owner.error_);
  }

  static inline void incoming(Context &c) noexcept
  {
    c.events |= POLLIN;
  }

  static inline void outgoing(Context &c) noexcept
  {
    c.events |= POLLOUT;
  }

  static inline void hangup(Context &c) noexcept
  {
    c.events |= POLLHUP | POLLOUT;
  }
  struct Watch;
  struct Pending;
  struct PendingEvent;
  struct Readable;
  struct ReadEvent;
  struct Writable;
  struct WriteEvent;
  struct HungUp;
  struct Hangup;
  using Graph = flow::graph<
      Watch,
      flow::types<flow::node<Watch, flow::action<&watch>,
                             flow::branch<flow::condition<&listening>,
                                          flow::go<Pending>, flow::go<Readable>>>,
                  flow::node<Pending, flow::noop,
                             flow::branch<flow::condition<&pending_child>,
                                          flow::go<PendingEvent>, flow::stop>>,
                  flow::node<PendingEvent, flow::action<&incoming>, flow::stop>,
                  flow::node<Readable, flow::noop,
                             flow::branch<flow::condition<&readable>,
                                          flow::go<ReadEvent>, flow::go<Writable>>>,
                  flow::node<ReadEvent, flow::action<&incoming>, flow::go<Writable>>,
                  flow::node<Writable, flow::noop,
                             flow::branch<flow::condition<&writable>,
                                          flow::go<WriteEvent>, flow::go<HungUp>>>,
                  flow::node<WriteEvent, flow::action<&outgoing>, flow::go<HungUp>>,
                  flow::node<HungUp, flow::noop,
                             flow::branch<flow::condition<&hung_up>, flow::go<Hangup>,
                                          flow::stop>>,
                  flow::node<Hangup, flow::action<&hangup>, flow::stop>>>;
};

struct TcpSocket::ShutdownFlow
{
  struct Context
  {
    TcpSocket &owner;
    int how, result = 0;
  };

  static inline bool valid(Context &c) noexcept
  {
    return c.how >= 0 && c.how <= 2;
  }

  static inline bool connected(Context &c) noexcept
  {
    auto state = c.owner.state_;
    return state == TcpState::Established || state == TcpState::CloseWait ||
           state == TcpState::FinWait1 || state == TcpState::FinWait2;
  }

  static inline bool read(Context &c) noexcept
  {
    return c.how == 0 || c.how == 2;
  }

  static inline void close_read(Context &c) noexcept
  {
    c.owner.read_shutdown_ = true;
    c.owner.receive_.discard();
    c.owner.emit(16);
  }

  static inline bool write(Context &c) noexcept
  {
    return c.how == 1 || c.how == 2;
  }

  static inline void close_write(Context &c) noexcept
  {
    if (!c.owner.write_shutdown_)
    {
      c.owner.last_activity_ = detail::milliseconds();
    }
    c.owner.write_shutdown_ = true;
    c.owner.pump();
  }

  static inline void notify(Context &c) noexcept
  {
    c.owner.notify();
  }

  static inline void disconnected(Context &c) noexcept
  {
    c.result = -ENOTCONN;
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -1;
  }
  struct Connected;
  struct Read;
  struct CloseRead;
  struct Write;
  struct CloseWrite;
  struct Notify;
  struct Disconnected;
  using LockedGraph = flow::graph<
      Connected,
      flow::types<flow::node<Connected, flow::noop,
                             flow::branch<flow::condition<&connected>, flow::go<Read>,
                                          flow::go<Disconnected>>>,
                  flow::node<Read, flow::noop,
                             flow::branch<flow::condition<&read>, flow::go<CloseRead>,
                                          flow::go<Write>>>,
                  flow::node<CloseRead, flow::action<&close_read>, flow::go<Write>>,
                  flow::node<Write, flow::noop,
                             flow::branch<flow::condition<&write>, flow::go<CloseWrite>,
                                          flow::go<Notify>>>,
                  flow::node<CloseWrite, flow::action<&close_write>, flow::go<Notify>>,
                  flow::node<Notify, flow::action<&notify>, flow::stop>,
                  flow::node<Disconnected, flow::action<&disconnected>, flow::stop>>>;

  static inline void locked(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.registry_.lock());
    (void)LockedGraph::run<0>(c);
  }
  struct How;
  struct Locked;
  struct Invalid;
  using Graph = flow::graph<
      How, flow::types<flow::node<How, flow::noop,
                                  flow::branch<flow::condition<&valid>,
                                               flow::go<Locked>, flow::go<Invalid>>>,
                       flow::node<Locked, flow::action<&locked>, flow::stop>,
                       flow::node<Invalid, flow::action<&invalid>, flow::stop>>>;
};

struct TcpSocket::CloseFlow
{
  struct Context
  {
    TcpSocket &owner;
  };

  static inline void unown(Context &c) noexcept
  {
    c.owner.owned_ = false;
  }

  static inline bool listening(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::Listen;
  }

  static inline void close_listener(Context &c) noexcept
  {
    c.owner.registry_.close_children(c.owner);
    c.owner.state_ = TcpState::Closed;
  }

  static inline bool connecting(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::SynSent ||
           c.owner.state_ == TcpState::SynReceived;
  }

  static inline void reset(Context &c) noexcept
  {
    c.owner.emit(20);
    c.owner.state_ = TcpState::Closed;
  }

  static inline void shutdown(Context &c) noexcept
  {
    if (!c.owner.write_shutdown_)
    {
      c.owner.last_activity_ = detail::milliseconds();
    }
    c.owner.read_shutdown_ = c.owner.write_shutdown_ = true;
    c.owner.receive_.discard();
  }

  static inline void pump(Context &c) noexcept
  {
    c.owner.pump();
  }

  static inline bool closed(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::Closed;
  }

  static inline void destroy(Context &c) noexcept
  {
    c.owner.registry_.destroy(c.owner);
  }
  struct Ownership;
  struct Listener;
  struct Connecting;
  struct Reset;
  struct Shutdown;
  struct Pump;
  struct Reclaim;
  struct Destroy;
  using Graph = flow::graph<
      Ownership,
      flow::types<
          flow::node<Ownership, flow::action<&unown>,
                     flow::branch<flow::condition<&listening>, flow::go<Listener>,
                                  flow::go<Connecting>>>,
          flow::node<Listener, flow::action<&close_listener>, flow::go<Reclaim>>,
          flow::node<Connecting, flow::noop,
                     flow::branch<flow::condition<&connecting>, flow::go<Reset>,
                                  flow::go<Shutdown>>>,
          flow::node<Reset, flow::action<&reset>, flow::go<Reclaim>>,
          flow::node<Shutdown, flow::action<&shutdown>, flow::go<Pump>>,
          flow::node<Pump, flow::action<&pump>, flow::go<Reclaim>>,
          flow::node<
              Reclaim, flow::noop,
              flow::branch<flow::condition<&closed>, flow::go<Destroy>, flow::stop>>,
          flow::node<Destroy, flow::action<&destroy>, flow::stop>>>;
};

struct TcpSocket::WindowFlow
{
  struct Context
  {
    TcpSocket &owner;
    unsigned sequence, acknowledgement, value;
    bool changed = false;
  };

  static inline bool current(Context &c) noexcept
  {
    auto &s = c.owner;
    return !wire::before(c.acknowledgement, s.send_unacknowledged_) &&
           !wire::before(s.send_next_, c.acknowledgement) &&
           !wire::before(c.sequence, s.window_sequence_) &&
           (c.sequence != s.window_sequence_ ||
            !wire::before(c.acknowledgement, s.window_ack_));
  }

  static inline void compare(Context &c) noexcept
  {
    c.changed = c.value != c.owner.peer_window_;
  }

  static inline bool reopened(Context &c) noexcept
  {
    return !c.owner.peer_window_ && c.value;
  }

  static inline void reset_persist(Context &c) noexcept
  {
    c.owner.persist_deadline_ = 0;
    c.owner.persist_interval_ = 1000;
    c.owner.retransmission_.start(detail::milliseconds());
  }

  static inline void assign(Context &c) noexcept
  {
    c.owner.peer_window_ = c.value;
    c.owner.window_sequence_ = c.sequence;
    c.owner.window_ack_ = c.acknowledgement;
  }
  struct Current;
  struct Compare;
  struct ResetPersist;
  struct Assign;
  using Graph = flow::graph<
      Current,
      flow::types<
          flow::node<
              Current, flow::noop,
              flow::branch<flow::condition<&current>, flow::go<Compare>, flow::stop>>,
          flow::node<Compare, flow::action<&compare>,
                     flow::branch<flow::condition<&reopened>, flow::go<ResetPersist>,
                                  flow::go<Assign>>>,
          flow::node<ResetPersist, flow::action<&reset_persist>, flow::go<Assign>>,
          flow::node<Assign, flow::action<&assign>, flow::stop>>>;
};

} // namespace xv::net
