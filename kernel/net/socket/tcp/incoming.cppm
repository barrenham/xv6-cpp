module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:tcp_incoming_flow;
import :tcp_socket;

namespace xv::net
{
namespace flow = xv::flow;

struct TcpSocket::IncomingFlow
{
  struct Context
  {
    TcpSocket &owner;
    const unsigned char *segment;
    unsigned length;
    unsigned flags = 0, sequence = 0, acknowledgement = 0, header = 0, payload = 0,
             progress = 0;
    unsigned long now = 0;
    bool simultaneous = false, changed = false, reopened = false, was_finished = false;
  };

  static inline void decode(Context &c) noexcept
  {
    c.flags = c.segment[13];
    c.sequence = wire::read32(c.segment + 4);
    c.acknowledgement = wire::read32(c.segment + 8);
    c.header = (c.segment[12] >> 4) * 4;
    c.payload = c.length - c.header;
    c.now = detail::milliseconds();
  }

  static inline bool active_open(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::SynSent;
  }

  static inline bool bad_syn_ack(Context &c) noexcept
  {
    return (c.flags & 16) && c.acknowledgement != c.owner.send_next_;
  }

  static inline bool reset(Context &c) noexcept
  {
    return c.flags & 4;
  }

  static inline bool ack(Context &c) noexcept
  {
    return c.flags & 16;
  }

  static inline bool syn(Context &c) noexcept
  {
    return c.flags & 2;
  }

  static inline void reject_syn_ack(Context &c) noexcept
  {
    c.owner.registry_.tcp().transmit(c.owner.local_, c.owner.peer_, c.acknowledgement,
                                     0, 4, 0, nullptr, 0);
  }

  static inline void refused(Context &c) noexcept
  {
    c.owner.fail(ECONNREFUSED);
  }

  static inline void parse_mss(Context &c) noexcept
  {
    c.owner.peer_mss_ = tcp_peer_mss(c.segment, c.length);
  }

  static inline bool valid_mss(Context &c) noexcept
  {
    return c.owner.peer_mss_ != 0;
  }

  static inline void initialize_peer(Context &c) noexcept
  {
    auto &s = c.owner;
    s.receive_.reset(c.sequence + 1);
    s.peer_window_ = wire::read16(c.segment + 14);
    s.window_sequence_ = c.sequence;
    s.window_ack_ = c.acknowledgement;
    s.last_activity_ = c.now;
    unsigned maximum = s.mss();
    s.congestion_.initialize(maximum ? maximum : s.peer_mss_);
  }

  static inline void simultaneous_open(Context &c) noexcept
  {
    c.owner.state_ = TcpState::SynReceived;
    c.owner.retransmit();
  }

  static inline void establish_active(Context &c) noexcept
  {
    c.owner.acknowledge(c.acknowledgement, false);
    c.owner.state_ = TcpState::Established;
    c.owner.notify();
    ++c.sequence;
    c.flags &= ~2U;
  }

  static inline bool empty_handshake(Context &c) noexcept
  {
    return !c.payload && !(c.flags & 1);
  }

  static inline void emit_ack(Context &c) noexcept
  {
    c.owner.emit(16);
  }

  static inline void pump(Context &c) noexcept
  {
    c.owner.pump();
  }

  static inline bool time_wait(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::TimeWait;
  }

  static inline void refresh_time_wait(Context &c) noexcept
  {
    if (!(c.flags & 4) && (c.flags & 1) &&
        c.sequence + c.payload + 1 == c.owner.receive_.next())
    {
      c.owner.emit(16);
      c.owner.idle_deadline_ = c.now + 30000;
    }
  }

  static inline bool repeated_syn(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::SynReceived && (c.flags & 2) &&
           c.sequence + 1 == c.owner.receive_.next();
  }

  static inline bool acknowledges_syn(Context &c) noexcept
  {
    return c.acknowledgement == c.owner.send_next_;
  }

  static inline void retransmit(Context &c) noexcept
  {
    c.owner.retransmit();
  }

  static inline void consume_syn(Context &c) noexcept
  {
    ++c.sequence;
    c.flags &= ~2U;
    c.simultaneous = true;
  }

  static inline bool acceptable(Context &c) noexcept
  {
    return c.owner.receive_.acceptable(c.sequence, c.payload + ((c.flags & 1) ? 1 : 0));
  }

  static inline bool exact_reset(Context &c) noexcept
  {
    return c.sequence == c.owner.receive_.next();
  }

  static inline void connection_reset(Context &c) noexcept
  {
    c.owner.fail(ECONNRESET);
  }

  static inline bool future_ack(Context &c) noexcept
  {
    return wire::before(c.owner.send_next_, c.acknowledgement);
  }

  static inline bool valid_final_ack(Context &c) noexcept
  {
    return c.owner.state_ != TcpState::SynReceived ||
           (c.acknowledgement == c.owner.send_next_ &&
            c.sequence == c.owner.receive_.next());
  }

  static inline void acknowledge(Context &c) noexcept
  {
    auto &s = c.owner;
    c.reopened = !s.peer_window_;
    c.changed =
        s.update_window(c.sequence, c.acknowledgement, wire::read16(c.segment + 14));
    bool duplicate = !c.payload && !(c.flags & 1) && !c.changed &&
                     c.acknowledgement == s.send_unacknowledged_;
    s.acknowledge(c.acknowledgement, duplicate);
  }

  static inline bool closed(Context &c) noexcept
  {
    return c.owner.state_ == TcpState::Closed;
  }

  static inline void activate_peer(Context &c) noexcept
  {
    auto &s = c.owner;
    if (c.reopened && s.peer_window_ && s.send_unacknowledged_ != s.send_next_)
    {
      s.retransmit();
      s.retransmission_.start(c.now);
    }
    s.last_activity_ = c.now;
    if (s.state_ == TcpState::SynReceived)
    {
      s.state_ = TcpState::Established;
      if (s.listener_)
      {
        s.listener_->notify();
      }
      s.notify();
    }
  }

  static inline void insert_payload(Context &c) noexcept
  {
    auto &s = c.owner;
    c.was_finished = s.receive_.finished();
    c.progress =
        s.receive_.insert(c.sequence, c.segment + c.header, c.payload, c.flags & 1);
    if (s.read_shutdown_)
    {
      s.receive_.discard();
    }
  }

  static inline bool new_fin(Context &c) noexcept
  {
    return !c.was_finished && c.owner.receive_.finished();
  }

  static inline void apply_fin_state(Context &c) noexcept
  {
    auto &s = c.owner;
    if (s.state_ == TcpState::Established)
    {
      s.state_ = TcpState::CloseWait;
    }
    else if (s.state_ == TcpState::FinWait2)
    {
      s.enter_time_wait();
    }
    else if (s.state_ == TcpState::FinWait1)
    {
      s.state_ = TcpState::Closing;
    }
  }

  static inline bool reply_needed(Context &c) noexcept
  {
    return c.payload || (c.flags & 1) || c.simultaneous;
  }

  static inline bool notify_needed(Context &c) noexcept
  {
    return c.progress || c.changed;
  }

  static inline void notify(Context &c) noexcept
  {
    c.owner.notify();
  }
  struct Decode;
  struct SynAck;
  struct RejectResetCheck;
  struct RejectSynAck;
  struct SynReset;
  struct SynResetAck;
  struct Refused;
  struct SynFlag;
  struct Mss;
  struct Peer;
  struct SimultaneousOpen;
  struct EstablishActive;
  struct HandshakeAck;
  struct TimeWait;
  struct RefreshTimeWait;
  struct RepeatedSyn;
  struct RepeatedAck;
  struct Retransmit;
  struct SynAcknowledged;
  struct ConsumeSyn;
  struct Acceptable;
  struct UnacceptableReset;
  struct Reset;
  struct ExactReset;
  struct ConnectionReset;
  struct UnexpectedSyn;
  struct AckPresent;
  struct FutureAck;
  struct FinalAck;
  struct Challenge;
  struct Acknowledge;
  struct ActivatePeer;
  struct InsertPayload;
  struct FinState;
  struct Reply;
  struct ReplyAck;
  struct NotifyCheck;
  struct Notify;
  struct Pump;
  using Graph = flow::graph<
      Decode,
      flow::types<
          flow::node<Decode, flow::action<&decode>,
                     flow::branch<flow::condition<&active_open>, flow::go<SynAck>,
                                  flow::go<TimeWait>>>,
          flow::node<SynAck, flow::noop,
                     flow::branch<flow::condition<&bad_syn_ack>,
                                  flow::go<RejectResetCheck>, flow::go<SynReset>>>,
          flow::node<RejectResetCheck, flow::noop,
                     flow::branch<flow::condition<&reset>, flow::stop,
                                  flow::go<RejectSynAck>>>,
          flow::node<RejectSynAck, flow::action<&reject_syn_ack>, flow::stop>,
          flow::node<SynReset, flow::noop,
                     flow::branch<flow::condition<&reset>, flow::go<SynResetAck>,
                                  flow::go<SynFlag>>>,
          flow::node<
              SynResetAck, flow::noop,
              flow::branch<flow::condition<&ack>, flow::go<Refused>, flow::stop>>,
          flow::node<Refused, flow::action<&refused>, flow::stop>,
          flow::node<SynFlag, flow::noop,
                     flow::branch<flow::condition<&syn>, flow::go<Mss>, flow::stop>>,
          flow::node<
              Mss, flow::action<&parse_mss>,
              flow::branch<flow::condition<&valid_mss>, flow::go<Peer>, flow::stop>>,
          flow::node<Peer, flow::action<&initialize_peer>,
                     flow::branch<flow::condition<&ack>, flow::go<EstablishActive>,
                                  flow::go<SimultaneousOpen>>>,
          flow::node<SimultaneousOpen, flow::action<&simultaneous_open>, flow::stop>,
          flow::node<EstablishActive, flow::action<&establish_active>,
                     flow::branch<flow::condition<&empty_handshake>,
                                  flow::go<HandshakeAck>, flow::go<TimeWait>>>,
          flow::node<HandshakeAck, flow::action<&emit_ack>, flow::go<Pump>>,
          flow::node<TimeWait, flow::noop,
                     flow::branch<flow::condition<&time_wait>,
                                  flow::go<RefreshTimeWait>, flow::go<RepeatedSyn>>>,
          flow::node<RefreshTimeWait, flow::action<&refresh_time_wait>, flow::stop>,
          flow::node<RepeatedSyn, flow::noop,
                     flow::branch<flow::condition<&repeated_syn>, flow::go<RepeatedAck>,
                                  flow::go<Acceptable>>>,
          flow::node<RepeatedAck, flow::noop,
                     flow::branch<flow::condition<&ack>, flow::go<SynAcknowledged>,
                                  flow::go<Retransmit>>>,
          flow::node<Retransmit, flow::action<&retransmit>, flow::stop>,
          flow::node<SynAcknowledged, flow::noop,
                     flow::branch<flow::condition<&acknowledges_syn>,
                                  flow::go<ConsumeSyn>, flow::stop>>,
          flow::node<ConsumeSyn, flow::action<&consume_syn>, flow::go<Acceptable>>,
          flow::node<Acceptable, flow::noop,
                     flow::branch<flow::condition<&acceptable>, flow::go<Reset>,
                                  flow::go<UnacceptableReset>>>,
          flow::node<
              UnacceptableReset, flow::noop,
              flow::branch<flow::condition<&reset>, flow::stop, flow::go<Challenge>>>,
          flow::node<Reset, flow::noop,
                     flow::branch<flow::condition<&reset>, flow::go<ExactReset>,
                                  flow::go<UnexpectedSyn>>>,
          flow::node<ExactReset, flow::noop,
                     flow::branch<flow::condition<&exact_reset>,
                                  flow::go<ConnectionReset>, flow::go<Challenge>>>,
          flow::node<ConnectionReset, flow::action<&connection_reset>, flow::stop>,
          flow::node<UnexpectedSyn, flow::noop,
                     flow::branch<flow::condition<&syn>, flow::go<Challenge>,
                                  flow::go<AckPresent>>>,
          flow::node<
              AckPresent, flow::noop,
              flow::branch<flow::condition<&ack>, flow::go<FutureAck>, flow::stop>>,
          flow::node<FutureAck, flow::noop,
                     flow::branch<flow::condition<&future_ack>, flow::go<Challenge>,
                                  flow::go<FinalAck>>>,
          flow::node<FinalAck, flow::noop,
                     flow::branch<flow::condition<&valid_final_ack>,
                                  flow::go<Acknowledge>, flow::stop>>,
          flow::node<Challenge, flow::action<&emit_ack>, flow::stop>,
          flow::node<Acknowledge, flow::action<&acknowledge>,
                     flow::branch<flow::condition<&closed>, flow::stop,
                                  flow::go<ActivatePeer>>>,
          flow::node<ActivatePeer, flow::action<&activate_peer>,
                     flow::go<InsertPayload>>,
          flow::node<InsertPayload, flow::action<&insert_payload>,
                     flow::branch<flow::condition<&new_fin>, flow::go<FinState>,
                                  flow::go<Reply>>>,
          flow::node<FinState, flow::action<&apply_fin_state>, flow::go<Reply>>,
          flow::node<Reply, flow::noop,
                     flow::branch<flow::condition<&reply_needed>, flow::go<ReplyAck>,
                                  flow::go<NotifyCheck>>>,
          flow::node<ReplyAck, flow::action<&emit_ack>, flow::go<NotifyCheck>>,
          flow::node<NotifyCheck, flow::noop,
                     flow::branch<flow::condition<&notify_needed>, flow::go<Notify>,
                                  flow::go<Pump>>>,
          flow::node<Notify, flow::action<&notify>, flow::go<Pump>>,
          flow::node<Pump, flow::action<&pump>, flow::stop>>>;
};

} // namespace xv::net
