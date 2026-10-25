module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/arch/riscv/register_constants.h"

// push_off/pop_off are like xv::arch::intr_off()/xv::arch::intr_on() except that they are matched:
// it takes two xv::sync::pop_interrupts_off()s to undo two xv::sync::push_interrupts_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

module xv.sync;

import xv.arch;

import xv.print;

import xv.process;

namespace xv::sync
{
void push_interrupts_off()
{
  // disable interrupts to prevent an involuntary context
  // switch while using xv::process::current_cpu().
  uint64 flags = xv::arch::rc_sstatus(SSTATUS_SIE);
  int old = !!(flags & SSTATUS_SIE);

  if (xv::process::current_cpu()->noff == 0)
  {
    xv::process::current_cpu()->intena = old;
  }
  xv::process::current_cpu()->noff += 1;
}

void pop_interrupts_off()
{
  xv::process::CpuState *c = xv::process::current_cpu();
  if (xv::arch::intr_get())
  {
    xv::print::panic("pop_off - interruptible");
  }
  if (c->noff < 1)
  {
    xv::print::panic("pop_off");
  }
  c->noff -= 1;
  if (c->noff == 0 && c->intena)
  {
    xv::arch::intr_on();
  }
}

} // namespace xv::sync
