module;
#include "kernel/process/signals/signal_abi.h"
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
  if (address >= memory.size() || (address & 1)) return false;
  auto *entry = xv::memory::walk_entry(memory.page_table(), address, 0);
  return entry && (*entry & (PTE_V | PTE_U | PTE_X)) == (PTE_V | PTE_U | PTE_X);
}
}

int Process::signal_action(int signal, unsigned long address, unsigned long old,
                           unsigned long restorer)
{
  if (!supported_signal(signal) || signal == SIGKILL || signal == SIGSTOP) return -1;
  struct sigaction value{};
  if (address)
  {
    if (memory_.copy_in(&value, address, sizeof(value)) < 0 ||
        (value.sa_mask & ~PendingSignals::SupportedMask) ||
        (value.sa_flags & ~(SA_NODEFER | SA_RESETHAND | SA_RESTART))) return -1;
    auto handler = reinterpret_cast<unsigned long>(value.sa_handler);
    if (handler > SIGNAL_IGNORE &&
        (!executable(memory_, handler) || !executable(memory_, restorer))) return -1;
  }
  auto previous = pending_signals_.action(signal);
  struct sigaction result{reinterpret_cast<sighandler_t>(previous.handler),
                          previous.mask, previous.flags};
  // Copy both inputs before the output; aliased act/oldact is supported.
  if (old && memory_.copy_out(old, &result, sizeof(result)) < 0) return -1;
  if (address)
  {
    xv::sync::ScopedLock guard(lock);
    pending_signals_.set_action(signal,
        {reinterpret_cast<unsigned long>(value.sa_handler), value.sa_mask,
         value.sa_flags, restorer});
  }
  return 0;
}

int Process::signal_mask(int how, unsigned long address, unsigned long old)
{
  unsigned value = 0;
  if (address && (memory_.copy_in(&value, address, sizeof(value)) < 0 ||
      (value & ~PendingSignals::SupportedMask) ||
      (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK))) return -1;
  auto previous = pending_signals_.mask();
  if (old && memory_.copy_out(old, &previous, sizeof(previous)) < 0) return -1;
  if (address)
  {
    xv::sync::ScopedLock guard(lock);
    pending_signals_.set_mask(how == SIG_BLOCK ? previous | value :
                             how == SIG_UNBLOCK ? previous & ~value : value);
  }
  return 0;
}

int Process::signal_pending(unsigned long address)
{
  auto value = pending_signals_.pending() & pending_signals_.mask();
  return memory_.copy_out(address, &value, sizeof(value));
}

int Process::signal_suspend(unsigned long address)
{
  unsigned value;
  if (memory_.copy_in(&value, address, sizeof(value)) < 0 ||
      (value & ~PendingSignals::SupportedMask)) return -1;
  {
    xv::sync::ScopedLock guard(lock);
    if (!suspended_mask_) suspend_mask_ = pending_signals_.mask();
    suspended_mask_ = true;
    suspend_restarted_ = false;
    pending_signals_.set_mask(value);
  }
  for (;;)
  {
    if (is_killed())
      return pending_signals_.caught_signal() ? -EINTR : -1;
    // The same process lock guards signal wakeups and the final sleep check.
    prepare_interruptible_sleep(&suspend_mask_);
    sleep();
  }
}

bool Process::enter_signal_handler()
{
  const int signal = pending_signals_.caught_signal();
  if (!signal) return true;
  const auto action = pending_signals_.action(signal);
  if (signal_depth_ == 16 || !executable(memory_, action.handler) ||
      !executable(memory_, action.restorer) || trapframe->sp > memory_.size() ||
      trapframe->sp < sizeof(SignalFrame)) return false;
  const auto stack = (trapframe->sp - sizeof(SignalFrame)) & ~15UL;
  SignalFrame frame{};
  frame.magic = FrameMagic;
  frame.mask = suspended_mask_ ? suspend_mask_ : pending_signals_.mask();
  frame.pc = trapframe->epc;
  xv::runtime::move(frame.registers, &trapframe->ra, sizeof(frame.registers));
  if (suspended_mask_ && suspend_restarted_)
  {
    // A caught SIGCONT after a default stop completes sigsuspend. Its saved
    // context must return EINTR rather than replaying the stopped ecall.
    frame.pc += 4;
    frame.registers[9] = -EINTR; // a0
  }
  // May fault in lazy user pages. Never copy user memory while holding a spinlock.
  if (memory_.copy_out(stack, &frame, sizeof(frame)) < 0) return false;
  {
    xv::sync::ScopedLock guard(lock);
    pending_signals_.consume(signal);
    if (action.flags & SA_RESETHAND) pending_signals_.set_action(signal, {});
    unsigned mask = pending_signals_.mask() | action.mask;
    if (!(action.flags & SA_NODEFER)) mask |= 1U << signal;
    pending_signals_.set_mask(mask);
    suspended_mask_ = false;
    suspend_restarted_ = false;
    signal_frames_[signal_depth_++] = stack;
  }
  trapframe->sp = stack;
  trapframe->epc = action.handler;
  trapframe->ra = action.restorer;
  trapframe->a0 = signal;
  return true;
}

unsigned long Process::signal_return()
{
  if (!signal_depth_) return -1;
  SignalFrame frame{};
  const auto stack = signal_frames_[signal_depth_ - 1];
  if (trapframe->sp != stack || memory_.copy_in(&frame, stack, sizeof(frame)) < 0 ||
      frame.magic != FrameMagic || !executable(memory_, frame.pc) ||
      (frame.registers[1] & 15) || frame.registers[1] > memory_.size() ||
      (frame.mask & ~static_cast<unsigned long>(PendingSignals::SupportedMask))) return -1;
  {
    xv::sync::ScopedLock guard(lock);
    --signal_depth_;
    pending_signals_.set_mask(frame.mask);
  }
  trapframe->epc = frame.pc;
  xv::runtime::move(&trapframe->ra, frame.registers, sizeof(frame.registers));
  // dispatch() writes this back into a0; preserve the interrupted register value.
  return trapframe->a0;
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
  if (result != -1 || !syscall_interrupted_ || !signal) return result;
  bool restartable = number == SYS_read || number == SYS_write ||
                     number == SYS_wait || number == SYS_waitpid || number == SYS_recv;
  if (restartable && (pending_signals_.action(signal).flags & SA_RESTART))
  {
    trapframe->epc -= 4;
    return argument;
  }
  return -EINTR;
}
} // namespace xv::process
