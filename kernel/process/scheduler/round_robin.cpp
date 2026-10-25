module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

module xv.process;

import xv.trap;

import xv.fs;

import :round_robin;

namespace xv::process
{

namespace
{
constinit CpuState cpus[NCPU]{};
constinit RoundRobinScheduler active_scheduler(process_table);
} // namespace

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int cpu_id()
{
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
CpuState *current_cpu(void)
{
  int id = cpu_id();
  CpuState *c = &cpus[id];
  return c;
}

// Return the current Process *, or zero if none.
Process *current(void)
{
  push_off();
  CpuState *c = current_cpu();
  Process *p = c->current;
  pop_off();
  return p;
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void RoundRobinScheduler::run()
{
  Process *p;
  CpuState *c = current_cpu();

  c->current = 0;
  for (;;)
  {
    // The most recent process to run may have had interrupts
    // turned off; enable them to avoid a deadlock if all
    // processes are waiting. Then turn them back off
    // to avoid a possible race between an interrupt
    // and wfi.
    intr_on();
    intr_off();

    int nproc = 0;
    for (p = table_.entries_; p < &table_.entries_[NPROC]; p++)
    {
      p->lock.acquire();
      if (p->state != ProcessState::Unused)
      {
        nproc++;
      }
#ifdef LAB_LOCK
      if (p->pincpu && p->pincpu != c)
      {
        p->lock.release();
        continue;
      }
#endif
      if (p->state == ProcessState::Runnable)
      {
        // Switch to chosen process.  It is the process's job
        // to release its lock and then reacquire it
        // before jumping back to us.
        p->state = ProcessState::Running;
        c->current = p;

        swtch(&c->context, &p->context);

        // Don't re-enable interrupts on release.
        current_cpu()->intena = 0;

        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->current = 0;
      }
      p->lock.release();
    }
    if (nproc <= 2)
    { // only init and sh exist
      // nothing to run; stop running on this core until an interrupt.
      intr_on();
#ifndef LAB_FS
      asm volatile("wfi");
#endif
    }
  }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed table_.entries_->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be table_.entries_->intena and table_.entries_->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void RoundRobinScheduler::suspend()
{
  int intena;
  Process *p = current();

  if (!p->lock.holding())
  {
    panic("sched p->lock");
  }
  if (current_cpu()->noff != 1)
  {
    panic("sched locks");
  }
  if (p->state == ProcessState::Running)
  {
    panic("sched ProcessState::Running");
  }
  if (intr_get())
  {
    panic("sched interruptible");
  }

  intena = current_cpu()->intena;
  swtch(&p->context, &current_cpu()->context);
  current_cpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void RoundRobinScheduler::yield()
{
  Process *p = current();
  p->lock.acquire();
  p->state = ProcessState::Runnable;
  suspend();
  p->lock.release();
}

// A fork child's very first scheduling by scheduler()
// will swtch to first_return.
void RoundRobinScheduler::first_return()
{
  static int first = 1;
  Process *p = current();

  // Still holding p->lock from scheduler.
  p->lock.release();

  if (__atomic_load_n(&first, __ATOMIC_ACQUIRE))
  {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    xv::fs::filesystem().initialize(ROOTDEV);

    // ensure other cores see first=0.
    __atomic_store_n(&first, 0, __ATOMIC_RELEASE);

    // We can invoke execute() now that file system is initialized.
    // Put the return value (argc) of kexec into a0.
    char *arguments[] = {const_cast<char *>("/init"), nullptr};
    p->trapframe->a0 = execute("/init", arguments);
    if (p->trapframe->a0 == -1)
    {
      panic("exec");
    }
  }

  xv::trap::return_to_user();
}

// Register current process as waiting for wakeups on chan.
void RoundRobinScheduler::prepare_sleep(void *chan)
{
  Process *p = current();

  p->lock.acquire();
  if (chan == 0)
  {
    panic("prepare_sleep: zero chan");
  }
  p->chan = chan;
  p->lock.release();
}

// Put the thread to sleep.  Assumes prepare_sleep() was called before.
// If the channel registered by prepare_sleep() has been woken up in
// the meantime, do not go to sleep, and instead return immediately.
void RoundRobinScheduler::sleep()
{
  Process *p = current();

  p->lock.acquire();
  if (p->chan != 0)
  {
    p->state = ProcessState::Sleeping;
    suspend();
  }
  p->lock.release();
}

CpuState *cpu_at(int index)
{
  return &cpus[index];
}

Scheduler &scheduler()
{
  return active_scheduler;
}

void first_return()
{
  active_scheduler.first_return();
}

void prepare_sleep(void *channel)
{
  active_scheduler.prepare_sleep(channel);
}

void sleep()
{
  active_scheduler.sleep();
}
} // namespace xv::process
