module;
#include "kernel/support/flow.h"
#include "kernel/fs/io_status.h"
module xv.drivers:uart_flow;
import :uart;
import :platform;

namespace xv::drivers
{
namespace flow = xv::flow;

struct Uart16550::TransmitFlow
{
  struct Context
  {
    Uart16550 &owner;
    const char *data;
    int count;
    bool nonblocking;
    int result = -1;
  };

  static bool acquire_writer(Context &c) noexcept
  {
    if (c.nonblocking)
    {
      if (!c.owner.tx_lock_.try_acquire())
      {
        c.result = -EAGAIN;
        return false;
      }
      return true;
    }
    return c.owner.tx_lock_.acquire_interruptible();
  }

  static void start(Context &c) noexcept
  {
    c.owner.tx_state_.acquire();
    c.owner.pump();
  }

  static bool waiting(Context &c) noexcept
  {
    return Uart16550::TxCapacity - (c.owner.tx_written_ - c.owner.tx_read_) <
           static_cast<unsigned>(c.count);
  }

  static bool killed(Context &) noexcept
  {
    return detail::process_killed();
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
    detail::prepare_interruptible_sleep(&c.owner.tx_channel_);
    c.owner.tx_state_.release();
    detail::sleep_process();
    c.owner.tx_state_.acquire();
    c.owner.pump();
  }

  static void enqueue(Context &c) noexcept
  {
    for (int i = 0; i < c.count; ++i)
    {
      c.owner.tx_data_[c.owner.tx_written_++ % Uart16550::TxCapacity] = c.data[i];
    }
    c.owner.pump();
    c.result = c.count;
  }

  static void unlock_state(Context &c) noexcept
  {
    c.owner.tx_state_.release();
  }

  static void unlock_writer(Context &c) noexcept
  {
    c.owner.tx_lock_.release();
  }
  struct AcquireWriter;
  struct Start;
  struct Space;
  struct WaitKilled;
  struct Nonblocking;
  struct Again;
  struct Wait;
  struct ReadyKilled;
  struct Enqueue;
  struct UnlockState;
  struct UnlockWriter;
  using Graph = flow::graph<
      AcquireWriter,
      flow::types<
          flow::node<AcquireWriter, flow::noop,
                     flow::branch<flow::condition<&acquire_writer>, flow::go<Start>,
                                  flow::stop>>,
          flow::node<Start, flow::action<&start>, flow::go<Space>>,
          flow::node<Space, flow::noop,
                     flow::branch<flow::condition<&waiting>, flow::go<WaitKilled>,
                                  flow::go<ReadyKilled>>>,
          flow::node<WaitKilled, flow::noop,
                     flow::branch<flow::condition<&killed>, flow::go<UnlockState>,
                                  flow::go<Nonblocking>>>,
          flow::node<Nonblocking, flow::noop,
                     flow::branch<flow::condition<&nonblocking>, flow::go<Again>,
                                  flow::go<Wait>>>,
          flow::node<Again, flow::action<&again>, flow::go<UnlockState>>,
          flow::node<Wait, flow::action<&wait>, flow::go<Space>>,
          flow::node<ReadyKilled, flow::noop,
                     flow::branch<flow::condition<&killed>, flow::go<UnlockState>,
                                  flow::go<Enqueue>>>,
          flow::node<Enqueue, flow::action<&enqueue>, flow::go<UnlockState>>,
          flow::node<UnlockState, flow::action<&unlock_state>, flow::go<UnlockWriter>>,
          flow::node<UnlockWriter, flow::action<&unlock_writer>, flow::stop>>>;
};

struct Uart16550::DrainFlow
{
  struct Context
  {
    Uart16550 &owner;
  };

  static bool waiting(Context &c) noexcept
  {
    return c.owner.tx_read_ != c.owner.tx_written_ && !detail::process_killed();
  }

  static void pump(Context &c) noexcept
  {
    c.owner.pump();
  }

  static bool empty(Context &c) noexcept
  {
    return c.owner.tx_read_ == c.owner.tx_written_;
  }

  static void wait(Context &c) noexcept
  {
    detail::prepare_interruptible_sleep(&c.owner.tx_channel_);
    c.owner.tx_state_.release();
    detail::sleep_process();
    c.owner.tx_state_.acquire();
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.tx_state_.release();
  }

  // Hardware polling stays a tight direct loop after releasing the state lock.
  static void hardware_drain(Context &c) noexcept
  {
    while (!(c.owner.registers_.read8(5) & (1U << 6)) && !detail::process_killed())
    {
    }
  }
  struct Queued;
  struct Pump;
  struct Wait;
  struct Unlock;
  struct Hardware;
  using Graph = flow::graph<
      Queued,
      flow::types<flow::node<Queued, flow::noop,
                             flow::branch<flow::condition<&waiting>, flow::go<Pump>,
                                          flow::go<Unlock>>>,
                  flow::node<Pump, flow::action<&pump>,
                             flow::branch<flow::condition<&empty>, flow::go<Unlock>,
                                          flow::go<Wait>>>,
                  flow::node<Wait, flow::action<&wait>, flow::go<Queued>>,
                  flow::node<Unlock, flow::action<&unlock>, flow::go<Hardware>>,
                  flow::node<Hardware, flow::action<&hardware_drain>, flow::stop>>>;
};

} // namespace xv::drivers
