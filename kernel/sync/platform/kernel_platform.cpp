import xv.sync;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/process/proc.h"
#include "kernel/support/defs.h"

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

extern "C" void push_off(void)
{
  // disable interrupts to prevent an involuntary context
  // switch while using mycpu().
  uint64 flags = rc_sstatus(SSTATUS_SIE);
  int old = !!(flags & SSTATUS_SIE);

  if (mycpu()->noff == 0)
  {
    mycpu()->intena = old;
  }
  mycpu()->noff += 1;
}

extern "C" void pop_off(void)
{
  struct cpu *c = mycpu();
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

// C bridge keeps legacy process headers out of the named module's declarations.
extern "C" void *sync_current_cpu()
{
  return mycpu();
}

extern "C" int sync_current_pid()
{
  return myproc()->pid;
}
