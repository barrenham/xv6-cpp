module;
#include "kernel/process/signals/signal_abi.h"
#include "kernel/support/flow.h"
#include "kernel/syscall/syscall.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;
import xv.memory;
import xv.runtime;

namespace xv::process
{
namespace
{
// Only user registers cross the ABI. No kernel stack, SATP, hart ID or status CSR.
struct alignas(16) SignalFrame
{
  unsigned long magic;
  unsigned long mask;
  unsigned long pc;
  unsigned long registers[31]; // TrapFrame::ra through t6, in trampoline order
};

constexpr unsigned long FrameMagic = 0x5856534947465231UL;
static_assert(sizeof(SignalFrame) == 272);
static_assert(sizeof(TrapFrame) - __builtin_offsetof(TrapFrame, ra) == 31 * 8);

bool executable(const AddressSpace &memory, unsigned long address)
{
  if (address >= memory.size() || (address & 1))
  {
    return false;
  }
  auto *entry = xv::memory::walk_entry(memory.page_table(), address, 0);
  return entry && (*entry & (PTE_V | PTE_U | PTE_X)) == (PTE_V | PTE_U | PTE_X);
}
} // namespace

namespace flow = xv::flow;

struct Process::SignalActionFlow
{
  struct Context
  {
    Process &owner;
    int signal;
    unsigned long address, old, restorer;
    sigaction &value;
    PendingSignals::Action previous{};
    bool valid = false;
    int result = -1;
  };

  static bool input(Context &c) noexcept
  {
    return c.address != 0;
  }

  static void read(Context &c) noexcept
  {
    c.valid = c.owner.memory_.copy_in(&c.value, c.address, sizeof(c.value)) >= 0 &&
              !(c.value.sa_mask & ~PendingSignals::SupportedMask) &&
              !(c.value.sa_flags & ~(SA_NODEFER | SA_RESETHAND | SA_RESTART));
  }

  static bool valid(Context &c) noexcept
  {
    return c.valid;
  }

  static bool executable_handler(Context &c) noexcept
  {
    auto handler = reinterpret_cast<unsigned long>(c.value.sa_handler);
    return handler <= SIGNAL_IGNORE || (executable(c.owner.memory_, handler) &&
                                        executable(c.owner.memory_, c.restorer));
  }

  static void previous(Context &c) noexcept
  {
    c.previous = c.owner.pending_signals_.action(c.signal);
  }

  static bool copy_previous(Context &c) noexcept
  {
    sigaction result{reinterpret_cast<sighandler_t>(c.previous.handler),
                     c.previous.mask, c.previous.flags};
    return !c.old || c.owner.memory_.copy_out(c.old, &result, sizeof(result)) >= 0;
  }

  static void install(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock);
    c.owner.pending_signals_.set_action(
        c.signal, {reinterpret_cast<unsigned long>(c.value.sa_handler), c.value.sa_mask,
                   c.value.sa_flags, c.restorer});
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Input;
  struct Read;
  struct Executable;
  struct Previous;
  struct InstallCheck;
  struct Install;
  struct Success;
  using Graph = flow::graph<
      Input, flow::types<flow::node<Input, flow::noop,
                                    flow::branch<flow::condition<&input>,
                                                 flow::go<Read>, flow::go<Previous>>>,
                         flow::node<Read, flow::action<&read>,
                                    flow::branch<flow::condition<&valid>,
                                                 flow::go<Executable>, flow::stop>>,
                         flow::node<Executable, flow::noop,
                                    flow::branch<flow::condition<&executable_handler>,
                                                 flow::go<Previous>, flow::stop>>,
                         flow::node<Previous, flow::action<&previous>,
                                    flow::branch<flow::condition<&copy_previous>,
                                                 flow::go<InstallCheck>, flow::stop>>,
                         flow::node<InstallCheck, flow::noop,
                                    flow::branch<flow::condition<&input>,
                                                 flow::go<Install>, flow::go<Success>>>,
                         flow::node<Install, flow::action<&install>, flow::go<Success>>,
                         flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct Process::SignalMaskFlow
{
  struct Context
  {
    Process &owner;
    int how;
    unsigned long address, old;
    unsigned value = 0, previous = 0;
    bool valid = false;
    int result = -1;
  };

  static bool input(Context &c) noexcept
  {
    return c.address != 0;
  }

  static void read(Context &c) noexcept
  {
    c.valid = c.owner.memory_.copy_in(&c.value, c.address, sizeof(c.value)) >= 0 &&
              !(c.value & ~PendingSignals::SupportedMask) &&
              (c.how == SIG_BLOCK || c.how == SIG_UNBLOCK || c.how == SIG_SETMASK);
  }

  static bool valid(Context &c) noexcept
  {
    return c.valid;
  }

  static void previous(Context &c) noexcept
  {
    c.previous = c.owner.pending_signals_.mask();
  }

  static bool copy_previous(Context &c) noexcept
  {
    return !c.old ||
           c.owner.memory_.copy_out(c.old, &c.previous, sizeof(c.previous)) >= 0;
  }

  static void install(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock);
    c.owner.pending_signals_.set_mask(c.how == SIG_BLOCK     ? c.previous | c.value
                                      : c.how == SIG_UNBLOCK ? c.previous & ~c.value
                                                             : c.value);
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Input;
  struct Read;
  struct Previous;
  struct InstallCheck;
  struct Install;
  struct Success;
  using Graph = flow::graph<
      Input, flow::types<flow::node<Input, flow::noop,
                                    flow::branch<flow::condition<&input>,
                                                 flow::go<Read>, flow::go<Previous>>>,
                         flow::node<Read, flow::action<&read>,
                                    flow::branch<flow::condition<&valid>,
                                                 flow::go<Previous>, flow::stop>>,
                         flow::node<Previous, flow::action<&previous>,
                                    flow::branch<flow::condition<&copy_previous>,
                                                 flow::go<InstallCheck>, flow::stop>>,
                         flow::node<InstallCheck, flow::noop,
                                    flow::branch<flow::condition<&input>,
                                                 flow::go<Install>, flow::go<Success>>>,
                         flow::node<Install, flow::action<&install>, flow::go<Success>>,
                         flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct Process::SignalSuspendFlow
{
  struct Context
  {
    Process &owner;
    unsigned long address;
    unsigned value = 0;
    bool valid = false;
    int result = -1;
  };

  static void read(Context &c) noexcept
  {
    c.valid = c.owner.memory_.copy_in(&c.value, c.address, sizeof(c.value)) >= 0 &&
              !(c.value & ~PendingSignals::SupportedMask);
  }

  static bool valid(Context &c) noexcept
  {
    return c.valid;
  }

  static void install(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock);
    if (!c.owner.suspended_mask_)
    {
      c.owner.suspend_mask_ = c.owner.pending_signals_.mask();
    }
    c.owner.suspended_mask_ = true;
    c.owner.suspend_restarted_ = false;
    c.owner.pending_signals_.set_mask(c.value);
  }

  static bool killed(Context &c) noexcept
  {
    return c.owner.is_killed();
  }

  static void interrupted(Context &c) noexcept
  {
    c.result = c.owner.pending_signals_.caught_signal() ? -EINTR : -1;
  }

  static void wait(Context &c) noexcept
  {
    prepare_interruptible_sleep(&c.owner.suspend_mask_);
    sleep();
  }
  struct Read;
  struct Install;
  struct Wait;
  struct Interrupted;
  using Graph = flow::graph<
      Read,
      flow::types<flow::node<Read, flow::action<&read>,
                             flow::branch<flow::condition<&valid>, flow::go<Install>,
                                          flow::stop>>,
                  flow::node<Install, flow::action<&install>,
                             flow::branch<flow::condition<&killed>,
                                          flow::go<Interrupted>, flow::go<Wait>>>,
                  flow::node<Wait, flow::action<&wait>,
                             flow::branch<flow::condition<&killed>,
                                          flow::go<Interrupted>, flow::go<Wait>>>,
                  flow::node<Interrupted, flow::action<&interrupted>, flow::stop>>>;
};

struct Process::EnterSignalFlow
{
  struct Context
  {
    Process &owner;
    int signal;
    const PendingSignals::Action &action;
    SignalFrame &frame;
    unsigned long stack = 0;
    bool copied = false, result = false;
  };

  static bool valid(Context &c) noexcept
  {
    return c.owner.signal_depth_ != 16 &&
           executable(c.owner.memory_, c.action.handler) &&
           executable(c.owner.memory_, c.action.restorer) &&
           c.owner.trapframe->sp <= c.owner.memory_.size() &&
           c.owner.trapframe->sp >= sizeof(SignalFrame);
  }

  static void frame(Context &c) noexcept
  {
    c.stack = (c.owner.trapframe->sp - sizeof(SignalFrame)) & ~15UL;
    c.frame = {};
    c.frame.magic = FrameMagic;
    c.frame.mask = c.owner.suspended_mask_ ? c.owner.suspend_mask_
                                           : c.owner.pending_signals_.mask();
    c.frame.pc = c.owner.trapframe->epc;
    xv::runtime::move(c.frame.registers, &c.owner.trapframe->ra,
                      sizeof(c.frame.registers));
    if (c.owner.suspended_mask_ && c.owner.suspend_restarted_)
    {
      c.frame.pc += 4;
      c.frame.registers[9] = -EINTR;
    }
  }

  static void copy(Context &c) noexcept
  {
    c.copied = c.owner.memory_.copy_out(c.stack, &c.frame, sizeof(c.frame)) >= 0;
  }

  static bool copied(Context &c) noexcept
  {
    return c.copied;
  }

  static void consume(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock);
    c.owner.pending_signals_.consume(c.signal);
    if (c.action.flags & SA_RESETHAND)
    {
      c.owner.pending_signals_.set_action(c.signal, {});
    }
    unsigned mask = c.owner.pending_signals_.mask() | c.action.mask;
    if (!(c.action.flags & SA_NODEFER))
    {
      mask |= 1U << c.signal;
    }
    c.owner.pending_signals_.set_mask(mask);
    c.owner.suspended_mask_ = false;
    c.owner.suspend_restarted_ = false;
    c.owner.signal_frames_[c.owner.signal_depth_++] = c.stack;
  }

  static void publish(Context &c) noexcept
  {
    c.owner.trapframe->sp = c.stack;
    c.owner.trapframe->epc = c.action.handler;
    c.owner.trapframe->ra = c.action.restorer;
    c.owner.trapframe->a0 = c.signal;
    c.result = true;
  }
  struct Valid;
  struct Frame;
  struct Copy;
  struct Consume;
  struct Publish;
  using Graph = flow::graph<
      Valid, flow::types<flow::node<Valid, flow::noop,
                                    flow::branch<flow::condition<&valid>,
                                                 flow::go<Frame>, flow::stop>>,
                         flow::node<Frame, flow::action<&frame>, flow::go<Copy>>,
                         flow::node<Copy, flow::action<&copy>,
                                    flow::branch<flow::condition<&copied>,
                                                 flow::go<Consume>, flow::stop>>,
                         flow::node<Consume, flow::action<&consume>, flow::go<Publish>>,
                         flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct Process::ReturnSignalFlow
{
  struct Context
  {
    Process &owner;
    SignalFrame &frame;
    unsigned long stack, result = ~0UL;
  };

  static bool valid(Context &c) noexcept
  {
    return c.owner.trapframe->sp == c.stack &&
           c.owner.memory_.copy_in(&c.frame, c.stack, sizeof(c.frame)) >= 0 &&
           c.frame.magic == FrameMagic && executable(c.owner.memory_, c.frame.pc) &&
           !(c.frame.registers[1] & 15) &&
           c.frame.registers[1] <= c.owner.memory_.size() &&
           !(c.frame.mask & ~static_cast<unsigned long>(PendingSignals::SupportedMask));
  }

  static void restore_mask(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock);
    --c.owner.signal_depth_;
    c.owner.pending_signals_.set_mask(c.frame.mask);
  }

  static void restore_registers(Context &c) noexcept
  {
    c.owner.trapframe->epc = c.frame.pc;
    xv::runtime::move(&c.owner.trapframe->ra, c.frame.registers,
                      sizeof(c.frame.registers));
    c.result = c.owner.trapframe->a0;
  }
  struct Valid;
  struct Mask;
  struct Registers;
  using Graph = flow::graph<
      Valid,
      flow::types<
          flow::node<Valid, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<Mask>, flow::stop>>,
          flow::node<Mask, flow::action<&restore_mask>, flow::go<Registers>>,
          flow::node<Registers, flow::action<&restore_registers>, flow::stop>>>;
};

int Process::signal_action(int signal, unsigned long address, unsigned long old,
                           unsigned long restorer)
{
  if (!supported_signal(signal) || signal == SIGKILL || signal == SIGSTOP)
  {
    return -1;
  }
  sigaction value{};
  SignalActionFlow::Context context{*this, signal, address, old, restorer, value};
  (void)SignalActionFlow::Graph::run<0>(context);
  return context.result;
}

int Process::signal_mask(int how, unsigned long address, unsigned long old)
{
  SignalMaskFlow::Context context{*this, how, address, old};
  (void)SignalMaskFlow::Graph::run<0>(context);
  return context.result;
}

int Process::signal_pending(unsigned long address)
{
  auto value = pending_signals_.pending() & pending_signals_.mask();
  return memory_.copy_out(address, &value, sizeof(value));
}

int Process::signal_suspend(unsigned long address)
{
  SignalSuspendFlow::Context context{*this, address};
  (void)SignalSuspendFlow::Graph::run<0>(context);
  return context.result;
}

bool Process::enter_signal_handler()
{
  const int signal = pending_signals_.caught_signal();
  if (!signal)
  {
    return true;
  }
  const auto action = pending_signals_.action(signal);
  SignalFrame frame;
  EnterSignalFlow::Context context{*this, signal, action, frame};
  (void)EnterSignalFlow::Graph::run<0>(context);
  return context.result;
}

unsigned long Process::signal_return()
{
  if (!signal_depth_)
  {
    return -1;
  }
  SignalFrame frame{};
  const auto stack = signal_frames_[signal_depth_ - 1];
  ReturnSignalFlow::Context context{*this, frame, stack};
  (void)ReturnSignalFlow::Graph::run<0>(context);
  return context.result;
}

void Process::begin_system_call()
{
  syscall_interrupted_ = false;
}

long Process::finish_system_call(int number, unsigned long argument, long result)
{
  if (number == SYS_sigsuspend && result == -1 && stop_requested())
  {
    suspend_restarted_ = true;
    return result;
  }
  int signal = pending_signals_.caught_signal();
  if (result != -1 || !syscall_interrupted_ || !signal)
  {
    return result;
  }
  bool restartable = number == SYS_read || number == SYS_write || number == SYS_wait ||
                     number == SYS_waitpid || number == SYS_accept ||
                     number == SYS_recvfrom || number == SYS_sendto;
  if (restartable && (pending_signals_.action(signal).flags & SA_RESTART))
  {
    trapframe->epc -= 4;
    return argument;
  }
  return -EINTR;
}
} // namespace xv::process
