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
  groups_.release(p->membership_);
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
  p->pending_signals_.reset();
  p->signal_depth_ = 0;
  p->suspended_mask_ = p->suspend_restarted_ = p->syscall_interrupted_ = false;
  p->interruptible_ = false;
  p->xstate = 0;
  p->exit_signal_ = p->stop_signal_ = 0;
  p->stop_event_ = p->continued_event_ = p->continue_notification_ = false;
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
  if (!groups_.create_initial(p->membership_, p->pid))
  {
    xv::print::panic("process: no initial session memory");
  }

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

  np->pending_signals_.inherit_dispositions(p->pending_signals_);
  np->signal_depth_ = p->signal_depth_;
  for (unsigned i = 0; i < p->signal_depth_; ++i)
    np->signal_frames_[i] = p->signal_frames_[i];

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  np->descriptors_.inherit_from(p->descriptors_);
  np->directory_.inherit_from(p->directory_);
  np->credentials_.inherit(p->credentials_);

  xv::runtime::copy_string(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  np->lock.release();

  wait_lock_.acquire();
  groups_.inherit(np->membership_, np->pid, p->membership_);
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

    Adopt(Process *parent, Process *initial, ProcessGroups &groups, ProcessManager &manager)
        : parent_(parent), initial_(initial), groups_(groups), manager_(manager)
    {
    }

    void visit(Process &child) override
    {
      if (child.state != ProcessState::Unused && child.parent == parent_)
      {
        groups_.reparent(child.membership_, initial_->pid);
        child.parent = initial_;
        if (child.state == ProcessState::Zombie) manager_.post_child_signal(child);
        adopted = true;
      }
    }

    bool adopted = false;

  private:

    Process *parent_;
    Process *initial_;
    ProcessGroups &groups_;
    ProcessManager &manager_;
  };

  Adopt adopt(p, initial_, groups_, *this);
  store_.visit(adopt);
  if (adopt.adopted)
  {
    wakeup(&child_events_);
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void ProcessManager::exit_process(int status, int signal)
{
  Process *p = current();

  if (p == initial_)
  {
    xv::print::panic("init exiting");
  }

  groups_.exit_terminal(p->membership_);
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
  wakeup(&child_events_);

  p->lock.acquire();

  p->xstate = status;
  p->exit_signal_ = signal;
  p->state = ProcessState::Zombie;
  post_child_signal(*p);

  wait_lock_.release();

  // Jump into the scheduler, never to return.
  scheduler_.suspend();
  xv::print::panic("zombie exit");
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
  if (process == initial_ || !Credentials::may_signal(current()->credentials().snapshot(), process->credentials().snapshot()))
  {
    process->lock.release();
    return -1;
  }
  process->killed = 1;
  wake_for_signal(*process);
  process->lock.release();
  notify_child_events();
  return 0;
}

void ProcessManager::dump()
{
  class Listing final : public ProcessVisitor
  {
  public:

    void visit(Process &process) override
    {
      static const char *states[] = {"unused", "used",    "sleep", "runble",
                                     "run",    "stopped", "zombie"};
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
bool ProcessManager::snapshot_after(int after, ProcessInfo &result)
{
  class Snapshot final : public ProcessVisitor
  {
  public:
    Snapshot(int after, ProcessInfo &result, ProcessGroups &groups)
        : after_(after), result_(result), groups_(groups) {}
    void visit(Process &process) override
    {
      if (process.state == ProcessState::Unused || process.pid <= after_ ||
          (found && process.pid >= result_.pid)) return;
      result_ = {};
      result_.pid = process.pid;
      result_.parent = process.parent ? process.parent->pid : 0;
      result_.group = groups_.get_group(process.membership_);
      result_.session = groups_.get_session(process.membership_, 0);
      auto credentials = process.credentials_.snapshot();
      result_.uid = credentials.uid; result_.effective_uid = credentials.effective_uid;
      result_.gid = credentials.gid; result_.effective_gid = credentials.effective_gid;
      result_.bytes = process.memory_.observed_size();
      const char states[] = "?USRRTZ";
      result_.state = states[static_cast<unsigned>(process.state)];
      xv::runtime::move(result_.name, process.name, sizeof(result_.name));
      found = true;
    }
    bool found = false;
  private:
    int after_;
    ProcessInfo &result_;
    ProcessGroups &groups_;
  } snapshot(after, result, groups_);
  xv::sync::ScopedLock family(wait_lock_);
  store_.visit(snapshot);
  return snapshot.found;
}
int ProcessManager::inspect_files(int pid, xv::fs::FileInfo *entries, unsigned capacity)
{
  if (pid < 1 || capacity < DescriptorTable::Capacity) return -1;
  class Pid final : public ProcessPredicate
  {
  public:
    explicit Pid(int pid) : pid_(pid) {}
    bool matches(Process &process) override
    { return process.id() == pid_ && process.state != ProcessState::Unused; }
  private:
    int pid_;
  } predicate(pid);
  auto *process = store_.find(predicate);
  if (!process) return -1;
  auto caller = current()->credentials().snapshot(), target = process->credentials().snapshot();
  if (caller.effective_uid != 0 && (caller.effective_uid != target.uid || target.uid != target.effective_uid))
  { process->lock.release(); return -1; }
  int count = process->files().inspect(entries, capacity);
  process->lock.release();
  return count;
}
} // namespace xv::process
