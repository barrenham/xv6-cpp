module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:transport_algorithm_flows;
import :congestion;
import :retransmission_timer;
import :wire;

namespace xv::net
{
namespace flow = xv::flow;

struct TcpCongestion::DuplicateFlow
{
  struct Context
  {
    TcpCongestion &owner;
    unsigned acknowledgement, flight, next;
    bool result = false;
  };

  static inline bool eligible(Context &c) noexcept
  {
    return c.flight &&
           (!c.owner.barrier_ || !wire::before(c.acknowledgement, c.owner.recover_));
  }

  static inline void count(Context &c) noexcept
  {
    ++c.owner.duplicates_;
  }

  static inline bool third(Context &c) noexcept
  {
    return c.owner.duplicates_ == 3;
  }

  static inline void recover(Context &c) noexcept
  {
    auto &s = c.owner;
    s.threshold_ = c.flight / 2 > 2 * s.mss_ ? c.flight / 2 : 2 * s.mss_;
    s.window_ = s.threshold_ + 3 * s.mss_;
    s.recover_ = c.next;
    s.recovery_ = true;
    c.result = true;
  }

  static inline bool can_inflate(Context &c) noexcept
  {
    auto &s = c.owner;
    return s.recovery_ && s.duplicates_ > 3 && s.window_ < 65535 - s.mss_;
  }

  static inline void inflate(Context &c) noexcept
  {
    c.owner.window_ += c.owner.mss_;
  }
  struct Eligible;
  struct Count;
  struct Recover;
  struct Inflation;
  struct Inflate;
  using Graph = flow::graph<
      Eligible,
      flow::types<flow::node<Eligible, flow::noop,
                             flow::branch<flow::condition<&eligible>, flow::go<Count>,
                                          flow::stop>>,
                  flow::node<Count, flow::action<&count>,
                             flow::branch<flow::condition<&third>, flow::go<Recover>,
                                          flow::go<Inflation>>>,
                  flow::node<Recover, flow::action<&recover>, flow::stop>,
                  flow::node<Inflation, flow::noop,
                             flow::branch<flow::condition<&can_inflate>,
                                          flow::go<Inflate>, flow::stop>>,
                  flow::node<Inflate, flow::action<&inflate>, flow::stop>>>;
};

struct TcpCongestion::AcknowledgeFlow
{
  struct Context
  {
    TcpCongestion &owner;
    unsigned bytes, acknowledgement;
    bool result = false;
  };

  static inline bool bytes(Context &c) noexcept
  {
    return c.bytes != 0;
  }

  static inline bool recovery(Context &c) noexcept
  {
    return c.owner.recovery_;
  }

  static inline bool partial(Context &c) noexcept
  {
    return wire::before(c.acknowledgement, c.owner.recover_);
  }

  static inline void repair(Context &c) noexcept
  {
    auto &s = c.owner;
    s.window_ = s.window_ > c.bytes ? s.window_ - c.bytes : s.mss_;
    if (c.bytes >= s.mss_)
    {
      s.window_ += s.mss_;
    }
    c.result = true;
  }

  static inline void leave_recovery(Context &c) noexcept
  {
    auto &s = c.owner;
    s.recovery_ = false;
    s.barrier_ = false;
    s.window_ = s.threshold_;
    s.credit_ = s.duplicates_ = 0;
  }

  static inline void acknowledge_barrier(Context &c) noexcept
  {
    c.owner.duplicates_ = 0;
    if (c.owner.barrier_ && !wire::before(c.acknowledgement, c.owner.recover_))
    {
      c.owner.barrier_ = false;
    }
  }

  static inline bool slow_start(Context &c) noexcept
  {
    return c.owner.window_ < c.owner.threshold_;
  }

  static inline void grow(Context &c) noexcept
  {
    c.owner.window_ += c.bytes < c.owner.mss_ ? c.bytes : c.owner.mss_;
  }

  static inline void accumulate(Context &c) noexcept
  {
    auto &s = c.owner;
    s.credit_ += c.bytes;
    if (s.credit_ >= s.window_)
    {
      s.credit_ -= s.window_;
      s.window_ += s.mss_;
    }
  }

  static inline void clamp(Context &c) noexcept
  {
    if (c.owner.window_ > 65535)
    {
      c.owner.window_ = 65535;
    }
  }
  struct Bytes;
  struct Recovery;
  struct Partial;
  struct Repair;
  struct LeaveRecovery;
  struct Barrier;
  struct Grow;
  struct Credit;
  struct Clamp;
  using Graph = flow::graph<
      Bytes,
      flow::types<flow::node<Bytes, flow::noop,
                             flow::branch<flow::condition<&bytes>, flow::go<Recovery>,
                                          flow::stop>>,
                  flow::node<Recovery, flow::noop,
                             flow::branch<flow::condition<&recovery>, flow::go<Partial>,
                                          flow::go<Barrier>>>,
                  flow::node<Partial, flow::noop,
                             flow::branch<flow::condition<&partial>, flow::go<Repair>,
                                          flow::go<LeaveRecovery>>>,
                  flow::node<Repair, flow::action<&repair>, flow::stop>,
                  flow::node<LeaveRecovery, flow::action<&leave_recovery>, flow::stop>,
                  flow::node<Barrier, flow::action<&acknowledge_barrier>,
                             flow::branch<flow::condition<&slow_start>, flow::go<Grow>,
                                          flow::go<Credit>>>,
                  flow::node<Grow, flow::action<&grow>, flow::go<Clamp>>,
                  flow::node<Credit, flow::action<&accumulate>, flow::go<Clamp>>,
                  flow::node<Clamp, flow::action<&clamp>, flow::stop>>>;
};

struct RetransmissionTimer::AcknowledgeFlow
{
  struct Context
  {
    RetransmissionTimer &owner;
    unsigned acknowledgement;
    unsigned long now;
    bool outstanding;
    unsigned sample = 0;
  };

  static inline bool sampled(Context &c) noexcept
  {
    return c.owner.sampling_ && !wire::before(c.acknowledgement, c.owner.sample_end_);
  }

  static inline void measure(Context &c) noexcept
  {
    c.sample = c.now - c.owner.started_;
    if (!c.sample)
    {
      c.sample = 1;
    }
  }

  static inline bool first_sample(Context &c) noexcept
  {
    return c.owner.smoothed_ == 0;
  }

  static inline void initialize(Context &c) noexcept
  {
    c.owner.smoothed_ = c.sample;
    c.owner.variation_ = c.sample / 2;
  }

  static inline void smooth(Context &c) noexcept
  {
    auto &s = c.owner;
    unsigned difference =
        c.sample > s.smoothed_ ? c.sample - s.smoothed_ : s.smoothed_ - c.sample;
    s.variation_ = (3 * s.variation_ + difference) / 4;
    s.smoothed_ = (7 * s.smoothed_ + c.sample) / 8;
  }

  static inline void interval(Context &c) noexcept
  {
    auto &s = c.owner;
    unsigned deviation = 4 * s.variation_;
    s.interval_ = s.smoothed_ + (deviation > 100 ? deviation : 100);
    if (s.interval_ < 1000)
    {
      s.interval_ = 1000;
    }
    if (s.interval_ > 60000)
    {
      s.interval_ = 60000;
    }
    s.sampling_ = false;
  }

  static inline bool all_acknowledged(Context &c) noexcept
  {
    return !c.outstanding;
  }

  static inline void clear_ambiguity(Context &c) noexcept
  {
    c.owner.ambiguous_ = false;
  }

  static inline void start(Context &c) noexcept
  {
    c.owner.start(c.now);
  }
  struct Sample;
  struct Measure;
  struct Initialize;
  struct Smooth;
  struct Interval;
  struct Outstanding;
  struct ClearAmbiguity;
  struct Start;
  using Graph = flow::graph<
      Sample,
      flow::types<
          flow::node<Sample, flow::noop,
                     flow::branch<flow::condition<&sampled>, flow::go<Measure>,
                                  flow::go<Outstanding>>>,
          flow::node<Measure, flow::action<&measure>,
                     flow::branch<flow::condition<&first_sample>, flow::go<Initialize>,
                                  flow::go<Smooth>>>,
          flow::node<Initialize, flow::action<&initialize>, flow::go<Interval>>,
          flow::node<Smooth, flow::action<&smooth>, flow::go<Interval>>,
          flow::node<Interval, flow::action<&interval>, flow::go<Outstanding>>,
          flow::node<Outstanding, flow::noop,
                     flow::branch<flow::condition<&all_acknowledged>,
                                  flow::go<ClearAmbiguity>, flow::go<Start>>>,
          flow::node<ClearAmbiguity, flow::action<&clear_ambiguity>, flow::go<Start>>,
          flow::node<Start, flow::action<&start>, flow::stop>>>;
};

} // namespace xv::net
