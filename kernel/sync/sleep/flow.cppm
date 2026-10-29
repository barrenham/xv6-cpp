module;
#include "kernel/support/flow.h"
module xv.sync:sleep_flow;
import :sleep;
import :platform;

namespace xv::sync
{
namespace flow = xv::flow;

struct SleepLock::AcquireFlow
{
  struct Context
  {
    SleepLock &owner;
    bool interruptible, acquired = false;
  };

  static inline bool interrupted(Context &c) noexcept
  {
    return c.interruptible && detail::interrupted();
  }

  static inline bool owned(Context &c) noexcept
  {
    return c.owner.locked_;
  }

  static inline void wait(Context &c) noexcept
  {
    if (c.owner.pid_ == detail::current_pid())
    {
      detail::fail("sleeplock: recursive acquire");
    }
    if (c.interruptible)
    {
      detail::prepare_interruptible_sleep(&c.owner);
    }
    else
    {
      detail::prepare_sleep(&c.owner);
    }
    c.owner.state_lock_.release();
    detail::sleep_process();
    c.owner.state_lock_.acquire();
  }

  static inline void claim(Context &c) noexcept
  {
    c.owner.locked_ = true;
    c.owner.pid_ = detail::current_pid();
    c.acquired = true;
  }

  static inline void unlock(Context &c) noexcept
  {
    c.owner.state_lock_.release();
  }
  struct Interrupted;
  struct Owned;
  struct Wait;
  struct Claim;
  struct Unlock;
  using Graph = flow::graph<
      Interrupted,
      flow::types<flow::node<Interrupted, flow::noop,
                             flow::branch<flow::condition<&interrupted>,
                                          flow::go<Unlock>, flow::go<Owned>>>,
                  flow::node<Owned, flow::noop,
                             flow::branch<flow::condition<&owned>, flow::go<Wait>,
                                          flow::go<Claim>>>,
                  flow::node<Wait, flow::action<&wait>, flow::go<Interrupted>>,
                  flow::node<Claim, flow::action<&claim>, flow::go<Unlock>>,
                  flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

} // namespace xv::sync
