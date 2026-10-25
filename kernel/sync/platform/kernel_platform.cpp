import xv.process;

import xv.sync;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

extern "C" void push_off(void)
{
  // disable interrupts to prevent an involuntary context
  // switch while using xv::process::current_cpu().
  uint64 flags = rc_sstatus(SSTATUS_SIE);
  int old = !!(flags & SSTATUS_SIE);

  if (xv::process::current_cpu()->noff == 0)
  {
    xv::process::current_cpu()->intena = old;
  }
  xv::process::current_cpu()->noff += 1;
}

extern "C" void pop_off(void)
{
  xv::process::CpuState *c = xv::process::current_cpu();
  if (intr_get())
  {
    panic("pop_off - interruptible");
  }
  if (c->noff < 1)
  {
    panic("pop_off");
  }
  c->noff -= 1;
  if (c->noff == 0 && c->intena)
  {
    intr_on();
  }
}
