module;
#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/trap/arch/riscv/entry.h"

module xv.trap;

import xv.arch;

import xv.print;
import :platform;
import xv.process;
import xv.drivers;
import xv.syscall;

namespace xv::trap::detail
{
static_assert(TrapEvent::SupervisorPrevious == SSTATUS_SPP);
static_assert(TrapEvent::InterruptEnable == SSTATUS_SIE);

TrapEvent snapshot()
{
  return {TrapCause(xv::arch::r_scause()), xv::arch::r_sepc(), xv::arch::r_sstatus(),
          xv::arch::r_stval()};
}

void install_kernel_vector()
{
  xv::arch::w_stvec(reinterpret_cast<Word>(kernelvec));
}

bool has_process()
{
  return xv::process::current() != nullptr;
}

bool killed()
{
  return xv::process::current()->is_killed();
}

void request_kill()
{
  xv::process::current()->request_kill();
}

[[noreturn]] void exit_current(int status)
{
  xv::process::exit_process(status);
}

void yield_current()
{
  xv::process::scheduler().yield();
}

void save_user_pc(Word pc)
{
  xv::process::current()->registers()->epc = pc;
}

void advance_user_pc(Word bytes)
{
  xv::process::current()->registers()->epc += bytes;
}

void enable_interrupts()
{
  xv::arch::intr_on();
}

void dispatch_system_call()
{
  xv::syscall::dispatch();
}

void dispatch_external_interrupt()
{
  xv::drivers::handle_external_interrupt(xv::process::cpu_id());
}

bool resolve_page_fault(Word address, bool read)
{
  return xv::process::current()->memory().handle_fault(address, read) != 0;
}

void report_user_fault(const TrapEvent &event)
{
  xv::print::printk("usertrap(): unexpected scause 0x%lx pid=%d\n", event.cause.raw(),
                    xv::process::current()->id());
  xv::print::printk("            sepc=0x%lx stval=0x%lx\n", event.pc, event.value);
}

[[noreturn]] void fatal(const char *message, const TrapEvent &event)
{
  xv::print::printk("scause=0x%lx sepc=0x%lx stval=0x%lx\n", event.cause.raw(),
                    event.pc, event.value);
  xv::print::panic(message);
}

void prepare_user_return()
{
  auto *process = xv::process::current();
  xv::arch::intr_off();
  const auto vector = TRAMPOLINE + (reinterpret_cast<Word>(uservec) -
                                    reinterpret_cast<Word>(trampoline));
  xv::arch::w_stvec(vector);
  auto *frame = process->registers();
  frame->kernel_satp = xv::arch::r_satp();
  frame->kernel_sp = process->kernel_stack() + PGSIZE;
  frame->kernel_trap = reinterpret_cast<Word>(usertrap);
  frame->kernel_hartid = xv::arch::r_tp();
  auto status = xv::arch::r_sstatus();
  status &= ~SSTATUS_SPP;
  status |= SSTATUS_SPIE;
  xv::arch::w_sstatus(status);
  xv::arch::w_sepc(frame->epc);
}

Word user_page_table_token()
{
  return MAKE_SATP(xv::process::current()->memory().page_table());
}

void restore_kernel(const TrapEvent &event)
{
  xv::arch::w_sepc(event.pc);
  xv::arch::w_sstatus(event.status);
}

[[noreturn]] void enter_user(Word token)
{
  const auto address = TRAMPOLINE + (reinterpret_cast<Word>(userret) -
                                     reinterpret_cast<Word>(trampoline));
  reinterpret_cast<void (*)(Word)>(address)(token);
  __builtin_unreachable();
}

int hart_id()
{
  return xv::process::cpu_id();
}

Word time()
{
  return xv::arch::r_time();
}

void arm_timer(Word deadline)
{
  xv::arch::w_stimecmp(deadline);
}

void prepare_sleep(void *channel)
{
  xv::process::prepare_sleep(channel);
}

void sleep()
{
  xv::process::sleep();
}

void wakeup(void *channel)
{
  xv::process::wakeup(channel);
}
} // namespace xv::trap::detail
