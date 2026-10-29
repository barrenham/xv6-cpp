module;
#include "kernel/support/flow.h"
module xv.trap:clock_flow;
import :clock;
import :platform;

namespace xv::trap
{
namespace flow = xv::flow;

struct TickClock::PauseFlow
{
  struct Context
  {
    TickClock &owner;
    unsigned interval, start;
    int result = 0;
  };

  static inline bool waiting(Context &c) noexcept
  {
    return c.owner.ticks_ - c.start < c.interval;
  }

  static inline bool interrupted(Context &) noexcept
  {
    return detail::interrupted();
  }

  static inline void interrupted_result(Context &c) noexcept
  {
    c.result = -1;
  }

  static inline void wait(Context &c) noexcept
  {
    detail::prepare_interruptible_sleep(&c.owner.ticks_);
    c.owner.lock_.release();
    detail::sleep();
    c.owner.lock_.acquire();
  }

  static inline void unlock(Context &c) noexcept
  {
    c.owner.lock_.release();
  }
  struct Elapsed;
  struct Interrupted;
  struct Cancelled;
  struct Wait;
  struct Unlock;
  using Graph = flow::graph<
      Elapsed,
      flow::types<
          flow::node<Elapsed, flow::noop,
                     flow::branch<flow::condition<&waiting>, flow::go<Interrupted>,
                                  flow::go<Unlock>>>,
          flow::node<Interrupted, flow::noop,
                     flow::branch<flow::condition<&interrupted>, flow::go<Cancelled>,
                                  flow::go<Wait>>>,
          flow::node<Cancelled, flow::action<&interrupted_result>, flow::go<Unlock>>,
          flow::node<Wait, flow::action<&wait>, flow::go<Elapsed>>,
          flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

} // namespace xv::trap
