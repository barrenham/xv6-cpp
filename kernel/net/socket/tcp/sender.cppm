module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:tcp_sender_flow;
import :tcp_socket;

namespace xv::net
{
namespace flow = xv::flow;

struct TcpSocket::PumpFlow
{
  struct Context
  {
    TcpSocket &owner;
    unsigned maximum = 0, flight = 0, limit = 0, amount = 0;
    int result = 0;
  };

  static inline bool active(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::Established ||
           c.owner.state_ == TcpState::CloseWait;
  }

  static inline void maximum(Context &c) noexcept
  {
    c.maximum = c.owner.mss();
  }

  static inline bool routable(Context &c) noexcept
  {
    return c.maximum != 0;
  }

  static inline bool pending(Context &c) noexcept
  {
    return wire::before(c.owner.send_next_, c.owner.transmit_.end());
  }

  static inline void calculate_credit(Context &c) noexcept
  {
    c.flight = c.owner.send_next_ - c.owner.send_unacknowledged_;
    c.limit = c.owner.peer_window_ < c.owner.congestion_.window()
                  ? c.owner.peer_window_
                  : c.owner.congestion_.window();
  }

  static inline bool has_credit(Context &c) noexcept
  {
    return c.flight < c.limit;
  }

  static inline void send_segment(Context &c) noexcept
  {
    auto &s = c.owner;
    c.amount = s.transmit_.end() - s.send_next_;
    if (c.amount > c.maximum)
    {
      c.amount = c.maximum;
    }
    if (c.amount > c.limit - c.flight)
    {
      c.amount = c.limit - c.flight;
    }
    const auto *data = s.transmit_.span(s.send_next_, c.amount);
    c.result = s.segment(s.send_next_, 24, data, c.amount);
  }

  static inline bool sent(Context &c) noexcept
  {
    return c.result >= 0;
  }

  static inline void advance(Context &c) noexcept
  {
    auto now = detail::milliseconds();
    if (!c.flight)
    {
      c.owner.retransmission_.start(now);
    }
    c.owner.send_next_ += c.amount;
    c.owner.retransmission_.sent(c.owner.send_next_, now);
  }

  static inline void finish(Context &c) noexcept
  {
    c.owner.finish();
  }
  struct Active;
  struct Maximum;
  struct Pending;
  struct Credit;
  struct Send;
  struct Advance;
  struct Finish;
  using Graph = flow::graph<
      Active,
      flow::types<flow::node<Active, flow::noop,
                             flow::branch<flow::condition<&active>, flow::go<Maximum>,
                                          flow::stop>>,
                  flow::node<Maximum, flow::action<&maximum>,
                             flow::branch<flow::condition<&routable>, flow::go<Pending>,
                                          flow::stop>>,
                  flow::node<Pending, flow::noop,
                             flow::branch<flow::condition<&pending>, flow::go<Credit>,
                                          flow::go<Finish>>>,
                  flow::node<Credit, flow::action<&calculate_credit>,
                             flow::branch<flow::condition<&has_credit>, flow::go<Send>,
                                          flow::go<Finish>>>,
                  flow::node<Send, flow::action<&send_segment>,
                             flow::branch<flow::condition<&sent>, flow::go<Advance>,
                                          flow::go<Finish>>>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Pending>>,
                  flow::node<Finish, flow::action<&finish>, flow::stop>>>;
};

struct TcpSocket::TickFlow
{
  struct Context
  {
    TcpSocket &owner;
    unsigned long now;
    bool exhausted = false;
  };

  static inline bool retransmission_due(Context &c) noexcept
  {
    auto &s = c.owner;
    return s.send_next_ != s.send_unacknowledged_ &&
           (s.peer_window_ || s.syn_pending_) && s.retransmission_.expired(c.now);
  }

  static inline void count_retry(Context &c) noexcept
  {
    c.exhausted = c.owner.retries_++ >= 6;
  }

  static inline bool exhausted(Context &c) noexcept
  {
    return c.exhausted;
  }

  static inline void timeout(Context &c) noexcept
  {
    c.owner.emit(20);
    c.owner.fail(ETIMEDOUT);
  }

  static inline void retransmit(Context &c) noexcept
  {
    auto &s = c.owner;
    if (!s.syn_pending_)
    {
      s.congestion_.timeout(s.send_next_ - s.send_unacknowledged_, s.send_next_);
    }
    s.retransmit();
    s.retransmission_.retransmit(c.now);
  }

  static inline bool idle_expired(Context &c) noexcept
  {
    return (c.owner.state_ == TcpState::TimeWait ||
            c.owner.state_ == TcpState::FinWait2) &&
           c.now >= c.owner.idle_deadline_;
  }

  static inline void close_idle(Context &c) noexcept
  {
    c.owner.state_ = TcpState::Closed;
    c.owner.notify();
  }

  static inline bool active(Context &c) noexcept
  {
    auto state = c.owner.state_;
    return state == TcpState::Established || state == TcpState::CloseWait ||
           state == TcpState::FinWait1 || state == TcpState::LastAck;
  }

  static inline bool stalled(Context &c) noexcept
  {
    auto &s = c.owner;
    return (s.transmit_.size() || (s.write_shutdown_ && !s.fin_acked_)) &&
           c.now >= s.last_activity_ && c.now - s.last_activity_ >= 120000;
  }

  static inline void persist(Context &c) noexcept
  {
    c.owner.persist(c.now);
  }

  static inline void pump(Context &c) noexcept
  {
    c.owner.pump();
  }

  static inline bool unowned_closed(Context &c) noexcept
  {
    return !c.owner.owned_ && c.owner.state_ == TcpState::Closed;
  }

  static inline void destroy(Context &c) noexcept
  {
    c.owner.registry_.destroy(c.owner);
  }
  struct Retransmission;
  struct Retry;
  struct Timeout;
  struct Retransmit;
  struct Idle;
  struct CloseIdle;
  struct Active;
  struct Stall;
  struct Abort;
  struct Persist;
  struct Pump;
  struct Reclaim;
  struct Destroy;
  using Graph = flow::graph<
      Retransmission,
      flow::types<flow::node<Retransmission, flow::noop,
                             flow::branch<flow::condition<&retransmission_due>,
                                          flow::go<Retry>, flow::go<Idle>>>,
                  flow::node<Retry, flow::action<&count_retry>,
                             flow::branch<flow::condition<&exhausted>,
                                          flow::go<Timeout>, flow::go<Retransmit>>>,
                  flow::node<Timeout, flow::action<&timeout>, flow::go<Idle>>,
                  flow::node<Retransmit, flow::action<&retransmit>, flow::go<Idle>>,
                  flow::node<Idle, flow::noop,
                             flow::branch<flow::condition<&idle_expired>,
                                          flow::go<CloseIdle>, flow::go<Active>>>,
                  flow::node<CloseIdle, flow::action<&close_idle>, flow::go<Active>>,
                  flow::node<Active, flow::noop,
                             flow::branch<flow::condition<&active>, flow::go<Stall>,
                                          flow::go<Reclaim>>>,
                  flow::node<Stall, flow::noop,
                             flow::branch<flow::condition<&stalled>, flow::go<Abort>,
                                          flow::go<Persist>>>,
                  flow::node<Abort, flow::action<&timeout>, flow::go<Reclaim>>,
                  flow::node<Persist, flow::action<&persist>, flow::go<Pump>>,
                  flow::node<Pump, flow::action<&pump>, flow::go<Reclaim>>,
                  flow::node<Reclaim, flow::noop,
                             flow::branch<flow::condition<&unowned_closed>,
                                          flow::go<Destroy>, flow::stop>>,
                  flow::node<Destroy, flow::action<&destroy>, flow::stop>>>;
};

} // namespace xv::net
