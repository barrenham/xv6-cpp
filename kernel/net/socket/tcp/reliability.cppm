module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:tcp_reliability_flow;
import :tcp_socket;

namespace xv::net
{
namespace flow = xv::flow;

struct TcpSocket::AcknowledgeFlow
{
  struct Context
  {
    TcpSocket &owner;
    unsigned acknowledgement;
    bool duplicate;
    unsigned long now = 0;
    bool handshake = false;
    unsigned bytes = 0;
  };

  static inline bool valid(Context &c) noexcept
  {
    return !wire::before(c.acknowledgement, c.owner.send_unacknowledged_) &&
           !wire::before(c.owner.send_next_, c.acknowledgement);
  }

  static inline void clock(Context &c) noexcept
  {
    c.now = detail::milliseconds();
  }

  static inline bool duplicate_ack(Context &c) noexcept
  {
    return c.acknowledgement == c.owner.send_unacknowledged_;
  }

  static inline void handle_duplicate(Context &c) noexcept
  {
    auto &s = c.owner;
    if (c.duplicate && !s.syn_pending_ &&
        s.congestion_.duplicate(c.acknowledgement,
                                s.send_next_ - s.send_unacknowledged_, s.send_next_))
    {
      s.retransmit();
      s.retransmission_.start(c.now);
    }
    else if (!c.duplicate)
    {
      s.congestion_.reset_duplicates();
    }
  }

  static inline void advance(Context &c) noexcept
  {
    auto &s = c.owner;
    s.send_unacknowledged_ = c.acknowledgement;
    c.handshake = s.syn_pending_;
    if (s.syn_pending_ && wire::before(s.syn_sequence_, c.acknowledgement))
    {
      s.syn_pending_ = false;
    }
    c.bytes = s.transmit_.discard(c.acknowledgement);
    s.retries_ = 0;
    s.retransmission_.acknowledge(c.acknowledgement, c.now,
                                  s.send_unacknowledged_ != s.send_next_);
  }

  static inline bool partial_recovery(Context &c) noexcept
  {
    return !c.handshake && c.owner.congestion_.acknowledged(c.bytes, c.acknowledgement);
  }

  static inline void retransmit(Context &c) noexcept
  {
    c.owner.retransmit();
  }

  static inline bool fin_ack(Context &c) noexcept
  {
    return c.owner.fin_sent_ && !c.owner.fin_acked_ &&
           wire::before(c.owner.fin_sequence_, c.acknowledgement);
  }

  static inline void finish(Context &c) noexcept
  {
    auto &s = c.owner;
    s.fin_acked_ = true;
    if (s.state_ == TcpState::FinWait1)
    {
      s.state_ = TcpState::FinWait2;
      s.idle_deadline_ = c.now + 30000;
    }
    else if (s.state_ == TcpState::Closing)
    {
      s.enter_time_wait();
    }
    else if (s.state_ == TcpState::LastAck)
    {
      s.state_ = TcpState::Closed;
    }
  }

  static inline void notify(Context &c) noexcept
  {
    c.owner.notify();
  }
  struct Range;
  struct Clock;
  struct Duplicate;
  struct Advance;
  struct Retransmit;
  struct Fin;
  struct Finish;
  struct Notify;
  using Graph = flow::graph<
      Range,
      flow::types<flow::node<Range, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Clock>,
                                          flow::stop>>,
                  flow::node<Clock, flow::action<&clock>,
                             flow::branch<flow::condition<&duplicate_ack>,
                                          flow::go<Duplicate>, flow::go<Advance>>>,
                  flow::node<Duplicate, flow::action<&handle_duplicate>, flow::stop>,
                  flow::node<Advance, flow::action<&advance>,
                             flow::branch<flow::condition<&partial_recovery>,
                                          flow::go<Retransmit>, flow::go<Fin>>>,
                  flow::node<Retransmit, flow::action<&retransmit>, flow::go<Fin>>,
                  flow::node<Fin, flow::noop,
                             flow::branch<flow::condition<&fin_ack>, flow::go<Finish>,
                                          flow::go<Notify>>>,
                  flow::node<Finish, flow::action<&finish>, flow::go<Notify>>,
                  flow::node<Notify, flow::action<&notify>, flow::stop>>>;
};

struct TcpSocket::RetransmitFlow
{
  struct Context
  {
    TcpSocket &owner;
    unsigned amount = 0, maximum = 0;
  };

  static inline void uncertain(Context &c) noexcept
  {
    c.owner.retransmission_.uncertain();
  }

  static inline bool syn(Context &c) noexcept
  {
    return c.owner.syn_pending_;
  }

  static inline void send_syn(Context &c) noexcept
  {
    c.owner.segment(c.owner.syn_sequence_,
                    c.owner.state_ == TcpState::SynSent ? 2 : 18);
  }

  static inline bool data(Context &c) noexcept
  {
    return wire::before(c.owner.send_unacknowledged_, c.owner.transmit_.end());
  }

  static inline void measure(Context &c) noexcept
  {
    c.amount = c.owner.transmit_.end() - c.owner.send_unacknowledged_;
    unsigned outstanding = c.owner.send_next_ - c.owner.send_unacknowledged_;
    if (c.amount > outstanding)
    {
      c.amount = outstanding;
    }
    c.maximum = c.owner.mss();
  }

  static inline bool available(Context &c) noexcept
  {
    return c.maximum && c.amount;
  }

  static inline void send_data(Context &c) noexcept
  {
    if (c.amount > c.maximum)
    {
      c.amount = c.maximum;
    }
    auto *data = c.owner.transmit_.span(c.owner.send_unacknowledged_, c.amount);
    c.owner.segment(c.owner.send_unacknowledged_, 24, data, c.amount);
  }

  static inline bool fin(Context &c) noexcept
  {
    return c.owner.fin_sent_ && !c.owner.fin_acked_;
  }

  static inline void send_fin(Context &c) noexcept
  {
    c.owner.segment(c.owner.fin_sequence_, 17);
  }
  struct Uncertain;
  struct Syn;
  struct Data;
  struct Measure;
  struct SendData;
  struct Fin;
  struct SendFin;
  using Graph = flow::graph<
      Uncertain, flow::types<flow::node<Uncertain, flow::action<&uncertain>,
                                        flow::branch<flow::condition<&syn>,
                                                     flow::go<Syn>, flow::go<Data>>>,
                             flow::node<Syn, flow::action<&send_syn>, flow::stop>,
                             flow::node<Data, flow::noop,
                                        flow::branch<flow::condition<&data>,
                                                     flow::go<Measure>, flow::go<Fin>>>,
                             flow::node<Measure, flow::action<&measure>,
                                        flow::branch<flow::condition<&available>,
                                                     flow::go<SendData>, flow::stop>>,
                             flow::node<SendData, flow::action<&send_data>, flow::stop>,
                             flow::node<Fin, flow::noop,
                                        flow::branch<flow::condition<&fin>,
                                                     flow::go<SendFin>, flow::stop>>,
                             flow::node<SendFin, flow::action<&send_fin>, flow::stop>>>;
};

struct TcpSocket::FinishFlow
{
  struct Context
  {
    TcpSocket &owner;
    unsigned flight = 0;
  };

  static inline bool eligible(Context &c) noexcept
  {
    auto &s = c.owner;
    return s.write_shutdown_ && !s.fin_sent_ && s.send_next_ == s.transmit_.end() &&
           !s.syn_pending_ &&
           (s.state_ == TcpState::Established || s.state_ == TcpState::CloseWait);
  }

  static inline void measure(Context &c) noexcept
  {
    c.flight = c.owner.send_next_ - c.owner.send_unacknowledged_;
  }

  static inline bool credit(Context &c) noexcept
  {
    return c.flight < c.owner.peer_window_ && c.flight < c.owner.congestion_.window();
  }

  static inline bool send(Context &c) noexcept
  {
    return c.owner.segment(c.owner.send_next_, 17) >= 0;
  }

  static inline void commit(Context &c) noexcept
  {
    auto &s = c.owner;
    s.fin_sequence_ = s.send_next_++;
    s.fin_sent_ = true;
    s.state_ =
        s.state_ == TcpState::Established ? TcpState::FinWait1 : TcpState::LastAck;
    auto now = detail::milliseconds();
    if (!c.flight)
    {
      s.retransmission_.start(now);
    }
    s.retransmission_.sent(s.send_next_, now);
  }
  struct Eligible;
  struct Measure;
  struct Send;
  struct Commit;
  using Graph =
      flow::graph<Eligible,
                  flow::types<flow::node<Eligible, flow::noop,
                                         flow::branch<flow::condition<&eligible>,
                                                      flow::go<Measure>, flow::stop>>,
                              flow::node<Measure, flow::action<&measure>,
                                         flow::branch<flow::condition<&credit>,
                                                      flow::go<Send>, flow::stop>>,
                              flow::node<Send, flow::noop,
                                         flow::branch<flow::condition<&send>,
                                                      flow::go<Commit>, flow::stop>>,
                              flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

struct TcpSocket::PersistFlow
{
  struct Context
  {
    TcpSocket &owner;
    unsigned long now;
  };

  static inline bool eligible(Context &c) noexcept
  {
    auto &s = c.owner;
    return !s.peer_window_ && !s.syn_pending_ &&
           (s.transmit_.size() || (s.write_shutdown_ && !s.fin_acked_));
  }

  static inline void reset(Context &c) noexcept
  {
    c.owner.persist_deadline_ = 0;
    c.owner.persist_interval_ = 1000;
  }

  static inline bool due(Context &c) noexcept
  {
    return !c.owner.persist_deadline_ || c.now >= c.owner.persist_deadline_;
  }

  static inline bool data(Context &c) noexcept
  {
    return wire::before(c.owner.send_unacknowledged_, c.owner.transmit_.end());
  }

  static inline void probe_data(Context &c) noexcept
  {
    auto &s = c.owner;
    unsigned sequence = s.send_unacknowledged_, amount = 1;
    auto *data = s.transmit_.span(sequence, amount);
    if (s.segment(sequence, 16, data, 1) >= 0 && sequence == s.send_next_)
    {
      ++s.send_next_;
    }
    s.retransmission_.uncertain();
  }

  static inline void probe_ack(Context &c) noexcept
  {
    c.owner.segment(c.owner.send_next_ - 1, 16);
  }

  static inline void schedule(Context &c) noexcept
  {
    auto &s = c.owner;
    s.persist_deadline_ = c.now + s.persist_interval_;
    if (s.persist_interval_ < 60000)
    {
      s.persist_interval_ *= 2;
      if (s.persist_interval_ > 60000)
      {
        s.persist_interval_ = 60000;
      }
    }
  }
  struct Eligible;
  struct Reset;
  struct Deadline;
  struct Data;
  struct ProbeData;
  struct ProbeAck;
  struct Schedule;
  using Graph = flow::graph<
      Eligible,
      flow::types<
          flow::node<Eligible, flow::noop,
                     flow::branch<flow::condition<&eligible>, flow::go<Deadline>,
                                  flow::go<Reset>>>,
          flow::node<Reset, flow::action<&reset>, flow::stop>,
          flow::node<Deadline, flow::noop,
                     flow::branch<flow::condition<&due>, flow::go<Data>, flow::stop>>,
          flow::node<Data, flow::noop,
                     flow::branch<flow::condition<&data>, flow::go<ProbeData>,
                                  flow::go<ProbeAck>>>,
          flow::node<ProbeData, flow::action<&probe_data>, flow::go<Schedule>>,
          flow::node<ProbeAck, flow::action<&probe_ack>, flow::go<Schedule>>,
          flow::node<Schedule, flow::action<&schedule>, flow::stop>>>;
};

} // namespace xv::net
