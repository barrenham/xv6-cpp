module;
#include "kernel/support/flow.h"
#include "kernel/fs/io_status.h"
module xv.fs:pipe_flow;
import :pipe;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct Pipe::WriteFlow
{
  struct Context
  {
    Pipe &owner;
    unsigned long address;
    int count;
    bool nonblocking;
    int done = 0, result = -1;
    bool aborted = false, wait_needed = false;
  };

  static bool atomic_wait(Context &c) noexcept
  {
    return static_cast<unsigned>(c.count) <= Pipe::Capacity &&
           Pipe::Capacity - (c.owner.written_ - c.owner.read_) <
               static_cast<unsigned>(c.count);
  }

  static bool unavailable(Context &c) noexcept
  {
    return !c.owner.reader_open_ || detail::killed();
  }

  static bool nonblocking(Context &c) noexcept
  {
    return c.nonblocking;
  }

  static void again(Context &c) noexcept
  {
    c.result = -EAGAIN;
  }

  static void wait(Context &c) noexcept
  {
    detail::prepare_interruptible_sleep(&c.owner.written_);
    c.owner.lock_.release();
    detail::sleep();
    c.owner.lock_.acquire();
  }

  // Keep byte copies in a direct loop; dispatch only at wait/finish boundaries.
  static void transfer(Context &c) noexcept
  {
    c.wait_needed = false;
    while (c.done < c.count)
    {
      if (detail::killed())
      {
        c.result = c.done ? c.done : -1;
        c.aborted = true;
        return;
      }
      if (!c.owner.reader_open_)
      {
        c.result = c.done ? c.done : -1;
        c.aborted = true;
        return;
      }
      if (c.owner.written_ == c.owner.read_ + Pipe::Capacity)
      {
        detail::wakeup(&c.owner.read_);
        if (c.nonblocking)
        {
          c.result = c.done ? c.done : -EAGAIN;
          c.aborted = true;
        }
        else
        {
          c.wait_needed = true;
        }
        return;
      }
      char ch;
      if (detail::copy_in(&ch, true, c.address + c.done, 1) == -1)
      {
        c.result = c.done ? c.done : -1;
        return;
      }
      c.owner.data_[c.owner.written_++ % Pipe::Capacity] = ch;
      ++c.done;
    }
    c.result = c.done;
  }

  static bool aborted(Context &c) noexcept
  {
    return c.aborted;
  }

  static bool needs_wait(Context &c) noexcept
  {
    return c.wait_needed;
  }

  static void notify_reader(Context &c) noexcept
  {
    detail::wakeup(&c.owner.read_);
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.lock_.release();
  }
  struct AtomicSpace;
  struct Availability;
  struct Nonblocking;
  struct Again;
  struct AtomicSleep;
  struct Transfer;
  struct WaitNeeded;
  struct TransferSleep;
  struct Notify;
  struct Unlock;
  using Graph = flow::graph<
      AtomicSpace,
      flow::types<flow::node<AtomicSpace, flow::noop,
                             flow::branch<flow::condition<&atomic_wait>,
                                          flow::go<Availability>, flow::go<Transfer>>>,
                  flow::node<Availability, flow::noop,
                             flow::branch<flow::condition<&unavailable>,
                                          flow::go<Unlock>, flow::go<Nonblocking>>>,
                  flow::node<Nonblocking, flow::noop,
                             flow::branch<flow::condition<&nonblocking>,
                                          flow::go<Again>, flow::go<AtomicSleep>>>,
                  flow::node<Again, flow::action<&again>, flow::go<Unlock>>,
                  flow::node<AtomicSleep, flow::action<&wait>, flow::go<AtomicSpace>>,
                  flow::node<Transfer, flow::action<&transfer>,
                             flow::branch<flow::condition<&aborted>, flow::go<Unlock>,
                                          flow::go<WaitNeeded>>>,
                  flow::node<WaitNeeded, flow::noop,
                             flow::branch<flow::condition<&needs_wait>,
                                          flow::go<TransferSleep>, flow::go<Notify>>>,
                  flow::node<TransferSleep, flow::action<&wait>, flow::go<Transfer>>,
                  flow::node<Notify, flow::action<&notify_reader>, flow::go<Unlock>>,
                  flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

struct Pipe::ReadFlow
{
  struct Context
  {
    Pipe &owner;
    unsigned long address;
    int count;
    bool nonblocking;
    int result = -1;
  };

  static bool waiting(Context &c) noexcept
  {
    return c.owner.read_ == c.owner.written_ && c.owner.writer_open_;
  }

  static bool interrupted(Context &) noexcept
  {
    return detail::killed();
  }

  static bool nonblocking(Context &c) noexcept
  {
    return c.nonblocking;
  }

  static void again(Context &c) noexcept
  {
    c.result = -EAGAIN;
  }

  static void wait(Context &c) noexcept
  {
    detail::prepare_interruptible_sleep(&c.owner.read_);
    c.owner.lock_.release();
    detail::sleep();
    c.owner.lock_.acquire();
  }

  static void transfer(Context &c) noexcept
  {
    int i;
    for (i = 0; i < c.count; ++i)
    {
      if (c.owner.read_ == c.owner.written_)
      {
        break;
      }
      char ch = c.owner.data_[c.owner.read_ % Pipe::Capacity];
      if (detail::copy_out(true, c.address + i, &ch, 1) == -1)
      {
        if (i == 0)
        {
          i = -1;
        }
        break;
      }
      ++c.owner.read_;
    }
    c.result = i;
  }

  static void notify_writer(Context &c) noexcept
  {
    detail::wakeup(&c.owner.written_);
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.lock_.release();
  }
  struct Available;
  struct Interrupted;
  struct Nonblocking;
  struct Again;
  struct Wait;
  struct Transfer;
  struct Notify;
  struct Unlock;
  using Graph = flow::graph<
      Available,
      flow::types<flow::node<Available, flow::noop,
                             flow::branch<flow::condition<&waiting>,
                                          flow::go<Interrupted>, flow::go<Transfer>>>,
                  flow::node<Interrupted, flow::noop,
                             flow::branch<flow::condition<&interrupted>,
                                          flow::go<Unlock>, flow::go<Nonblocking>>>,
                  flow::node<Nonblocking, flow::noop,
                             flow::branch<flow::condition<&nonblocking>,
                                          flow::go<Again>, flow::go<Wait>>>,
                  flow::node<Again, flow::action<&again>, flow::go<Unlock>>,
                  flow::node<Wait, flow::action<&wait>, flow::go<Available>>,
                  flow::node<Transfer, flow::action<&transfer>, flow::go<Notify>>,
                  flow::node<Notify, flow::action<&notify_writer>, flow::go<Unlock>>,
                  flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

} // namespace xv::fs
