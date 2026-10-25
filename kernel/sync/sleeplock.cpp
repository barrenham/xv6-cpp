// Sleeping locks

#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/sync/spinlock.h"
#include "kernel/process/proc.h"
#include "kernel/sync/sleeplock.h"



void
SleepLock::init(const char *name)
{
  auto *lk = this;
  lk->lk.init("sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

void
SleepLock::acquire()
{
  auto *lk = this;
  lk->lk.acquire();
  while (lk->locked) {
    sleep_prepare(lk);
    lk->lk.release();
    sleep();
    lk->lk.acquire();
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  lk->lk.release();
}

void
SleepLock::release()
{
  auto *lk = this;
  lk->lk.acquire();
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  lk->lk.release();
}

int
SleepLock::holding()
{
  auto *lk = this;
  int r;

  lk->lk.acquire();
  r = lk->locked && (lk->pid == myproc()->pid);
  lk->lk.release();
  return r;
}
