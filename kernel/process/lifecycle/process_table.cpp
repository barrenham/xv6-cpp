module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;

import xv.print;

import xv.runtime;

import xv.fs;

import :table;
import xv.memory;

namespace xv::process
{
constinit ProcessTable process_table;

extern "C" char trampoline[];

// Allocate a page for each process's kernel stack.
// Map it high in memory, followed by an invalid
// guard page.
void ProcessTable::map_kernel_stacks(pagetable_t kpgtbl)
{
  Process *p;

  for (p = entries_; p < &entries_[NPROC]; p++)
  {
    char *pa = static_cast<char *>(xv::memory::allocator().allocate());
    if (pa == 0)
    {
      xv::print::panic("kalloc");
    }
    uint64 va = KSTACK((int)(p - entries_));
    xv::memory::map_kernel_range(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  }
}

// initialize the entries_ table.
void ProcessTable::initialize(void)
{
  Process *p;

  pid_lock_.init("next_pid_");
  wait_lock_.init("wait_lock_");
  for (p = entries_; p < &entries_[NPROC]; p++)
  {
    p->lock.init("entries_");
    p->state = ProcessState::Unused;
    p->kstack = KSTACK((int)(p - entries_));
  }
}

int ProcessTable::allocate_pid()
{
  int pid;

  pid_lock_.acquire();
  pid = next_pid_;
  next_pid_ = next_pid_ + 1;
  pid_lock_.release();
  return pid;
}

// Look in the process table for an ProcessState::Unused entries_.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
Process *ProcessTable::allocate(void)
{
  Process *p;

  for (p = entries_; p < &entries_[NPROC]; p++)
  {
    p->lock.acquire();
    if (p->state == ProcessState::Unused)
    {
      goto found;
    }
    else
    {
      p->lock.release();
    }
  }
  return 0;

found:
  p->pid = allocate_pid();
  p->state = ProcessState::Used;

  // Allocate a trapframe page.
  if ((p->trapframe = (TrapFrame *)xv::memory::allocator().allocate()) == 0)
  {
    reap(p);
    p->lock.release();
    return 0;
  }

#ifdef LAB_LOCK
  p->pincpu = 0;
#endif

#ifdef LAB_PGTBL
  if ((p->usyscall = (struct usyscall *)xv::memory::allocator().allocate()) == 0)
  {
    reap(p);
    p->lock.release();
    return 0;
  }
  xv::runtime::fill(p->usyscall, 0, PGSIZE);
  static_cast<struct usyscall *>(p->usyscall)->pid = p->pid;
#endif

  // An empty user page table.
  if (!p->create_image(p->memory_))
  {
    reap(p);
    p->lock.release();
    return 0;
  }

  // Set up new context to start executing at first_return,
  // which returns to user space.
  xv::runtime::fill(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)first_return;
  p->context.sp = p->kstack + PGSIZE;

  return p;
}

// free a entries_ structure and the data hanging from it,
// including user pages.
// p->lock must be held.
void ProcessTable::reap(Process *p)
{
  if (p->trapframe)
  {
    xv::memory::allocator().deallocate((void *)p->trapframe);
  }
  p->trapframe = 0;
  p->memory_.reset();
#ifdef LAB_PGTBL
  if (p->usyscall)
  {
    xv::memory::allocator().deallocate((void *)p->usyscall);
  }
  p->usyscall = 0;
#endif
  p->pid = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = ProcessState::Unused;
}

// Set up first user process.
void ProcessTable::create_initial_process(void)
{
  Process *p;

  p = allocate();
  if (!p)
  {
    xv::print::panic("process: no initial process slot");
  }
  initial_ = p;

  p->directory_.replace(xv::fs::filesystem().lookup("/"));

  p->state = ProcessState::Runnable;

  p->lock.release();
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int ProcessTable::fork_process(void)
{
  int pid;
  Process *np;
  Process *p = current();

  // Allocate process.
  if ((np = allocate()) == 0)
  {
    return -1;
  }

  // Copy user memory from parent to child.
  if (!p->memory_.copy_to(np->memory_))
  {
    reap(np);
    np->lock.release();
    return -1;
  }

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  np->descriptors_.inherit_from(p->descriptors_);
  np->directory_.inherit_from(p->directory_);

  xv::runtime::copy_string(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  np->lock.release();

  wait_lock_.acquire();
  np->parent = p;
  wait_lock_.release();

  np->lock.acquire();
  np->state = ProcessState::Runnable;

  np->lock.release();

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock_.
void ProcessTable::reparent(Process *p)
{
  Process *pp;

  for (pp = entries_; pp < &entries_[NPROC]; pp++)
  {
    if (pp->parent == p)
    {
      pp->parent = initial_;
      wakeup(initial_);
    }
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void ProcessTable::exit_process(int status)
{
  Process *p = current();

  if (p == initial_)
  {
    xv::print::panic("init exiting");
  }

  p->descriptors_.close_all();
  {
    xv::fs::Transaction transaction;
    p->directory_.clear();
  }

  wait_lock_.acquire();

  // Give any children to init.
  reparent(p);

  // Parent might be sleeping in wait().
  wakeup(p->parent);

  p->lock.acquire();

  p->xstate = status;
  p->state = ProcessState::Zombie;

  wait_lock_.release();

  // Jump into the scheduler, never to return.
  scheduler().suspend();
  xv::print::panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int ProcessTable::wait(uint64 addr)
{
  Process *pp;
  int havekids, pid;
  Process *p = current();

  wait_lock_.acquire();

  for (;;)
  {
    // Scan through table looking for exited children.
    havekids = 0;
    for (pp = entries_; pp < &entries_[NPROC]; pp++)
    {
      if (pp->parent == p)
      {
        // make sure the child isn't still in exit() or swtch().
        pp->lock.acquire();

        havekids = 1;
        if (pp->state == ProcessState::Zombie)
        {
          // Found one.
          pid = pp->pid;
          if (addr != 0 &&
              p->memory_.copy_out(addr, &pp->xstate, sizeof(pp->xstate)) < 0)
          {
            pp->lock.release();
            wait_lock_.release();
            return -1;
          }
          pp->parent = 0;
          reap(pp);
          pp->lock.release();
          wait_lock_.release();
          return pid;
        }
        pp->lock.release();
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || p->is_killed())
    {
      wait_lock_.release();
      return -1;
    }

    // Wait for a child to exit.
    prepare_sleep(p); //DOC: wait-sleep
    wait_lock_.release();
    sleep();
    wait_lock_.acquire();
  }
}

// Wake up all processes sleeping on channel chan.
void ProcessTable::wakeup(void *chan)
{
  Process *p;

  for (p = entries_; p < &entries_[NPROC]; p++)
  {
    p->lock.acquire();
    if (p->chan == chan)
    {
      // If the process is waiting for wakeups on this channel,
      // signal that the ProcessTable::wakeup happened by clearing p->chan.
      p->chan = 0;

      // If this waiting process has gotten so far as to actually
      // go to sleep, also set it back to ProcessState::Running.
      if (p->state == ProcessState::Sleeping)
      {
        p->state = ProcessState::Runnable;
      }
    }
    p->lock.release();
  }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int ProcessTable::kill(int pid)
{
  Process *p;

  for (p = entries_; p < &entries_[NPROC]; p++)
  {
    p->lock.acquire();
    if (p->pid == pid)
    {
      p->killed = 1;
      if (p->state == ProcessState::Sleeping)
      {
        // Wake process from sleep().
        p->state = ProcessState::Runnable;
      }
      p->lock.release();
      return 0;
    }
    p->lock.release();
  }
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void ProcessTable::dump(void)
{
  static char *states[] = {
      // clang-format off
    "unused",
    "used",
    "sleep ",
    "runble",
    "run   ",
    "zombie"
      // clang-format on
  };
  Process *p;
  char *state;

  xv::print::printk("\n");
  for (p = entries_; p < &entries_[NPROC]; p++)
  {
    if (p->state == ProcessState::Unused)
    {
      continue;
    }
    if (static_cast<unsigned>(p->state) < xv::runtime::array_size(states))
    {
      state = states[static_cast<unsigned>(p->state)];
    }
    else
    {
      state = "???";
    }
    xv::print::printk("%d %s %s", p->pid, state, p->name);
    xv::print::printk("\n");
  }
}

} // namespace xv::process
