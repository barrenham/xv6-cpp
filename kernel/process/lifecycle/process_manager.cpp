module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;

import xv.print;

import xv.runtime;

import xv.fs;

import :manager;
import xv.memory;

namespace xv::process
{
extern "C" char trampoline[];

void ProcessManager::initialize()
{
  pid_lock_.init("next pid");
  wait_lock_.init("process families");
}

int ProcessManager::allocate_pid()
{
  int pid;

  pid_lock_.acquire();
  pid = next_pid_ <= 0x7fffffffU ? static_cast<int>(next_pid_++) : 0;
  pid_lock_.release();
  return pid;
}

// Allocation transfers a stable descriptor and its held process lock.
Process *ProcessManager::allocate()
{
  Process *p = store_.allocate();
  if (!p)
  {
    return nullptr;
  }
  p->pid = allocate_pid();
  if (!p->pid)
  {
    reap(p);
    return nullptr;
  }

  // Allocate a trapframe page.
  if ((p->trapframe = (TrapFrame *)xv::memory::allocator().allocate()) == 0)
  {
    reap(p);
    return 0;
  }

#ifdef LAB_LOCK
  __atomic_store_n(&p->pincpu, static_cast<CpuState *>(nullptr), __ATOMIC_RELEASE);
#endif

#ifdef LAB_PGTBL
  if ((p->usyscall = (struct usyscall *)xv::memory::allocator().allocate()) == 0)
  {
    reap(p);
    return 0;
  }
  xv::runtime::fill(p->usyscall, 0, PGSIZE);
  static_cast<struct usyscall *>(p->usyscall)->pid = p->pid;
#endif

  // An empty user page table.
  if (!p->create_image(p->memory_))
  {
    reap(p);
    return 0;
  }

  // Set up new context to start executing at first_return,
  // which returns to user space.
  xv::runtime::fill(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)first_return;
  p->context.sp = p->kstack + PGSIZE;

  return p;
}

// Release process-owned resources and its descriptor,
// including user pages.
// p->lock must be held.
void ProcessManager::reap(Process *p)
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
  p->parent = nullptr;
  store_.release(p);
}

// Set up first user process.
void ProcessManager::create_initial_process(void)
{
  Process *p;

  p = allocate();
  if (!p)
  {
    xv::print::panic("process: no initial process slot");
  }
  initial_ = p;

  p->directory_.replace(xv::fs::filesystem().lookup("/"));

  scheduler_.make_runnable(*p);

  p->lock.release();
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int ProcessManager::fork_process(void)
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
  scheduler_.make_runnable(*np);

  np->lock.release();

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock_.
void ProcessManager::reparent(Process *p)
{
  class Adopt final : public ProcessVisitor
  {
  public:

    Adopt(Process *parent, Process *initial) : parent_(parent), initial_(initial)
    {
    }

    void visit(Process &child) override
    {
      if (child.state != ProcessState::Unused && child.parent == parent_)
      {
        child.parent = initial_;
        adopted = true;
      }
    }

    bool adopted = false;

  private:

    Process *parent_;
    Process *initial_;
  };

  Adopt adopt(p, initial_);
  store_.visit(adopt);
  if (adopt.adopted)
  {
    wakeup(initial_);
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void ProcessManager::exit_process(int status)
{
  Process *p = current();

  if (p == initial_)
  {
    xv::print::panic("init exiting");
  }

  waits_.cancel(*p);
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
  scheduler_.suspend();
  xv::print::panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int ProcessManager::wait(uint64 addr)
{
  Process *p = current();
  wait_lock_.acquire();
  for (;;)
  {
    class Child final : public ProcessPredicate
    {
    public:

      explicit Child(Process *parent) : parent_(parent)
      {
      }

      bool matches(Process &child) override
      {
        if (child.state != ProcessState::Unused && child.parent == parent_)
        {
          have_children = true;
          return child.state == ProcessState::Zombie;
        }
        return false;
      }

      bool have_children = false;

    private:

      Process *parent_;
    };

    Child child(p);
    if (auto *exited = store_.find(child))
    {
      int pid = exited->pid;
      if (addr != 0 &&
          p->memory_.copy_out(addr, &exited->xstate, sizeof(exited->xstate)) < 0)
      {
        exited->lock.release();
        wait_lock_.release();
        return -1;
      }
      reap(exited); // consumes the child lock and retires the descriptor
      wait_lock_.release();
      return pid;
    }
    if (!child.have_children || p->is_killed())
    {
      wait_lock_.release();
      return -1;
    }
    prepare_sleep(p);
    wait_lock_.release();
    sleep();
    wait_lock_.acquire();
  }
}

// Wake up all processes sleeping on channel chan.
void ProcessManager::wakeup(void *channel)
{
  waits_.wake(channel);
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int ProcessManager::kill(int pid)
{
  if (pid <= 0)
  {
    return -1;
  }

  class Pid final : public ProcessPredicate
  {
  public:

    explicit Pid(int pid) : pid_(pid)
    {
    }

    bool matches(Process &process) override
    {
      return process.state != ProcessState::Unused && process.pid == pid_;
    }

  private:

    int pid_;
  };

  Pid key(pid);
  auto *process = store_.find(key);
  if (!process)
  {
    return -1;
  }
  process->killed = 1;
  // Notify even a prepared-but-not-yet-sleeping process. Membership remains
  // valid until wake/prepare/exit unlinks it under the channel bucket lock.
  process->chan = nullptr;
  if (process->state == ProcessState::Sleeping)
  {
    scheduler_.make_runnable(*process);
  }
  process->lock.release();
  return 0;
}

void ProcessManager::dump()
{
  class Listing final : public ProcessVisitor
  {
  public:

    void visit(Process &process) override
    {
      static const char *states[] = {"unused", "used", "sleep",
                                     "runble", "run",  "zombie"};
      if (process.state == ProcessState::Unused)
      {
        return;
      }
      auto index = static_cast<unsigned>(process.state);
      const char *state =
          index < xv::runtime::array_size(states) ? states[index] : "???";
      xv::print::printk("%d %s %s\n", process.pid, state, process.name);
    }
  };

  xv::print::printk("\n");
  Listing listing;
  store_.visit(listing);
}
} // namespace xv::process
