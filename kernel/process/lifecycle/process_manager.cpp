module;

#include "kernel/support/types.h"
#include "kernel/support/flow.h"
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

namespace flow = xv::flow;

struct ProcessManager::AllocateFlow
{
  struct Context
  {
    ProcessManager &owner;
    Process *process = nullptr, *result = nullptr;
    bool ready = false;
  };

  static void descriptor(Context &c) noexcept
  {
    c.process = c.owner.store_.allocate();
  }

  static bool allocated(Context &c) noexcept
  {
    return c.process != nullptr;
  }

  static void identity(Context &c) noexcept
  {
    c.process->pid = c.owner.allocate_pid();
  }

  static bool identified(Context &c) noexcept
  {
    return c.process->pid != 0;
  }

  static void trapframe(Context &c) noexcept
  {
    c.process->trapframe = static_cast<TrapFrame *>(xv::memory::allocator().allocate());
#ifdef LAB_LOCK
    if (c.process->trapframe)
    {
      __atomic_store_n(&c.process->pincpu, static_cast<CpuState *>(nullptr),
                       __ATOMIC_RELEASE);
    }
#endif
  }

  static bool has_trapframe(Context &c) noexcept
  {
    return c.process->trapframe != nullptr;
  }
#ifdef LAB_PGTBL
  static void user_syscall(Context &c) noexcept
  {
    c.process->usyscall = xv::memory::allocator().allocate();
    if (c.process->usyscall)
    {
      xv::runtime::fill(c.process->usyscall, 0, PGSIZE);
      static_cast<struct usyscall *>(c.process->usyscall)->pid = c.process->pid;
    }
  }

  static bool has_user_syscall(Context &c) noexcept
  {
    return c.process->usyscall != nullptr;
  }
#endif
  static void image(Context &c) noexcept
  {
    c.ready = c.process->create_image(c.process->memory_);
  }

  static bool ready(Context &c) noexcept
  {
    return c.ready;
  }

  static void prepare_return(Context &c) noexcept
  {
    xv::runtime::fill(&c.process->context, 0, sizeof(c.process->context));
    c.process->context.ra = reinterpret_cast<uint64>(first_return);
    c.process->context.sp = c.process->kstack + KSTACK_PAGES * PGSIZE;
    c.result = c.process;
  }

  static void rollback(Context &c) noexcept
  {
    c.owner.reap(c.process);
  }
  struct Descriptor;
  struct Identity;
  struct Trapframe;
  struct UserSyscall;
  struct Image;
  struct Prepare;
  struct Rollback;
#ifdef LAB_PGTBL
  using AfterTrapframe = flow::go<UserSyscall>;
#else
  using AfterTrapframe = flow::go<Image>;
#endif
  using Graph = flow::graph<
      Descriptor,
      flow::types<flow::node<Descriptor, flow::action<&descriptor>,
                             flow::branch<flow::condition<&allocated>,
                                          flow::go<Identity>, flow::stop>>,
                  flow::node<Identity, flow::action<&identity>,
                             flow::branch<flow::condition<&identified>,
                                          flow::go<Trapframe>, flow::go<Rollback>>>,
                  flow::node<Trapframe, flow::action<&trapframe>,
                             flow::branch<flow::condition<&has_trapframe>,
                                          AfterTrapframe, flow::go<Rollback>>>,
#ifdef LAB_PGTBL
                  flow::node<UserSyscall, flow::action<&user_syscall>,
                             flow::branch<flow::condition<&has_user_syscall>,
                                          flow::go<Image>, flow::go<Rollback>>>,
#endif
                  flow::node<Image, flow::action<&image>,
                             flow::branch<flow::condition<&ready>, flow::go<Prepare>,
                                          flow::go<Rollback>>>,
                  flow::node<Prepare, flow::action<&prepare_return>, flow::stop>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>>>;
};

struct ProcessManager::ReapFlow
{
  struct Context
  {
    ProcessManager &owner;
    Process *process;
  };

  static void group(Context &c) noexcept
  {
    c.owner.groups_.release(c.process->membership_);
  }

  static bool has_trapframe(Context &c) noexcept
  {
    return c.process->trapframe != nullptr;
  }

  static void free_trapframe(Context &c) noexcept
  {
    xv::memory::allocator().deallocate(c.process->trapframe);
  }

  static void image(Context &c) noexcept
  {
    c.process->trapframe = nullptr;
    c.process->memory_.reset();
  }
#ifdef LAB_PGTBL
  static bool has_user_syscall(Context &c) noexcept
  {
    return c.process->usyscall != nullptr;
  }

  static void free_user_syscall(Context &c) noexcept
  {
    xv::memory::allocator().deallocate(c.process->usyscall);
  }
#endif
  static void reset_identity(Context &c) noexcept
  {
#ifdef LAB_PGTBL
    c.process->usyscall = nullptr;
#endif
    c.process->pid = 0;
    c.process->name[0] = 0;
    c.process->chan = nullptr;
    c.process->killed = 0;
    c.process->pending_signals_.reset();
    c.process->signal_depth_ = 0;
    c.process->suspended_mask_ = c.process->suspend_restarted_ =
        c.process->syscall_interrupted_ = false;
    c.process->interruptible_ = false;
    c.process->xstate = 0;
    c.process->exit_signal_ = c.process->stop_signal_ = 0;
    c.process->stop_event_ = c.process->continued_event_ =
        c.process->continue_notification_ = false;
  }

  static void detach(Context &c) noexcept
  {
    detach_child(*c.process);
  }

  static void release(Context &c) noexcept
  {
    c.owner.store_.release(c.process);
  }
  struct Group;
  struct Trapframe;
  struct Image;
  struct UserSyscall;
  struct Reset;
  struct Detach;
  struct Release;
#ifdef LAB_PGTBL
  using AfterImage = flow::branch<flow::condition<&has_user_syscall>,
                                  flow::go<UserSyscall>, flow::go<Reset>>;
#else
  using AfterImage = flow::go<Reset>;
#endif
  using Graph = flow::graph<
      Group,
      flow::types<
          flow::node<Group, flow::action<&group>,
                     flow::branch<flow::condition<&has_trapframe>, flow::go<Trapframe>,
                                  flow::go<Image>>>,
          flow::node<Trapframe, flow::action<&free_trapframe>, flow::go<Image>>,
          flow::node<Image, flow::action<&image>, AfterImage>,
#ifdef LAB_PGTBL
          flow::node<UserSyscall, flow::action<&free_user_syscall>, flow::go<Reset>>,
#endif
          flow::node<Reset, flow::action<&reset_identity>, flow::go<Detach>>,
          flow::node<Detach, flow::action<&detach>, flow::go<Release>>,
          flow::node<Release, flow::action<&release>, flow::stop>>>;
};

struct ProcessManager::InitialFlow
{
  struct Context
  {
    ProcessManager &owner;
    Process *process = nullptr;
    bool grouped = false;
  };

  static void allocate(Context &c) noexcept
  {
    c.process = c.owner.allocate();
  }

  static bool allocated(Context &c) noexcept
  {
    return c.process != nullptr;
  }

  static void no_slot(Context &) noexcept
  {
    xv::print::panic("process: no initial process slot");
  }

  static void group(Context &c) noexcept
  {
    c.owner.initial_ = c.process;
    c.grouped = c.owner.groups_.create_initial(c.process->membership_, c.process->pid);
  }

  static bool grouped(Context &c) noexcept
  {
    return c.grouped;
  }

  static void no_session(Context &) noexcept
  {
    xv::print::panic("process: no initial session memory");
  }

  static void directory(Context &c) noexcept
  {
    c.process->directory_.replace(xv::fs::filesystem().root());
  }

  static void publish(Context &c) noexcept
  {
    c.owner.scheduler_.make_runnable(*c.process);
    c.process->lock.release();
  }
  struct Allocate;
  struct NoSlot;
  struct Group;
  struct NoSession;
  struct Directory;
  struct Publish;
  using Graph = flow::graph<
      Allocate,
      flow::types<flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&allocated>, flow::go<Group>,
                                          flow::go<NoSlot>>>,
                  flow::node<NoSlot, flow::action<&no_slot>, flow::stop>,
                  flow::node<Group, flow::action<&group>,
                             flow::branch<flow::condition<&grouped>,
                                          flow::go<Directory>, flow::go<NoSession>>>,
                  flow::node<NoSession, flow::action<&no_session>, flow::stop>,
                  flow::node<Directory, flow::action<&directory>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct ProcessManager::ForkFlow
{
  struct Context
  {
    ProcessManager &owner;
    Process &parent;
    Process *child = nullptr;
    bool copied = false;
    int result = -1;
  };

  static void allocate(Context &c) noexcept
  {
    c.child = c.owner.allocate();
  }

  static bool allocated(Context &c) noexcept
  {
    return c.child != nullptr;
  }

  static void image(Context &c) noexcept
  {
    c.copied = c.parent.memory_.copy_to(c.child->memory_);
  }

  static bool copied(Context &c) noexcept
  {
    return c.copied;
  }

  static void rollback(Context &c) noexcept
  {
    c.owner.reap(c.child);
  }

  static void signal_state(Context &c) noexcept
  {
    c.child->pending_signals_.inherit_dispositions(c.parent.pending_signals_);
    c.child->signal_depth_ = c.parent.signal_depth_;
    for (unsigned i = 0; i < c.parent.signal_depth_; ++i)
    {
      c.child->signal_frames_[i] = c.parent.signal_frames_[i];
    }
  }

  static void registers(Context &c) noexcept
  {
    *c.child->trapframe = *c.parent.trapframe;
    c.child->trapframe->a0 = 0;
  }

  static void resources(Context &c) noexcept
  {
    c.child->descriptors_.inherit_from(c.parent.descriptors_);
    c.child->directory_.inherit_from(c.parent.directory_);
    c.child->credentials_.inherit(c.parent.credentials_);
    xv::runtime::copy_string(c.child->name, c.parent.name, sizeof(c.child->name));
    c.result = c.child->pid;
    c.child->lock.release();
  }

  static void family(Context &c) noexcept
  {
    c.owner.wait_lock_.acquire();
    c.owner.groups_.inherit(c.child->membership_, c.child->pid, c.parent.membership_);
    attach_child(c.parent, *c.child);
    c.owner.wait_lock_.release();
  }

  static void publish(Context &c) noexcept
  {
    c.child->lock.acquire();
    c.owner.scheduler_.make_runnable(*c.child);
    c.child->lock.release();
  }
  struct Allocate;
  struct Image;
  struct Rollback;
  struct Signals;
  struct Registers;
  struct Resources;
  struct Family;
  struct Publish;
  using Graph = flow::graph<
      Allocate,
      flow::types<flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&allocated>, flow::go<Image>,
                                          flow::stop>>,
                  flow::node<Image, flow::action<&image>,
                             flow::branch<flow::condition<&copied>, flow::go<Signals>,
                                          flow::go<Rollback>>>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>,
                  flow::node<Signals, flow::action<&signal_state>, flow::go<Registers>>,
                  flow::node<Registers, flow::action<&registers>, flow::go<Resources>>,
                  flow::node<Resources, flow::action<&resources>, flow::go<Family>>,
                  flow::node<Family, flow::action<&family>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct ProcessManager::ReparentFlow
{
  struct Context
  {
    ProcessManager &owner;
    Process &parent;
    Process *child;
  };

  static void adopt(Context &c) noexcept
  {
    c.child->lock.acquire();
    c.owner.groups_.reparent(c.child->membership_, c.owner.initial_->pid);
    detach_child(*c.child);
    attach_child(*c.owner.initial_, *c.child);
  }

  static bool zombie(Context &c) noexcept
  {
    return c.child->state == ProcessState::Zombie;
  }

  static void notify(Context &c) noexcept
  {
    c.owner.post_child_signal(*c.child);
  }

  static void advance(Context &c) noexcept
  {
    c.child->lock.release();
    c.child = c.parent.first_child_;
  }

  static bool more(Context &c) noexcept
  {
    return c.child != nullptr;
  }

  static void wake(Context &c) noexcept
  {
    c.owner.wakeup(&c.owner.child_events_);
  }
  struct Adopt;
  struct Notify;
  struct Advance;
  struct Wake;
  using Graph = flow::graph<
      Adopt, flow::types<flow::node<Adopt, flow::action<&adopt>,
                                    flow::branch<flow::condition<&zombie>,
                                                 flow::go<Notify>, flow::go<Advance>>>,
                         flow::node<Notify, flow::action<&notify>, flow::go<Advance>>,
                         flow::node<Advance, flow::action<&advance>,
                                    flow::branch<flow::condition<&more>,
                                                 flow::go<Adopt>, flow::go<Wake>>>,
                         flow::node<Wake, flow::action<&wake>, flow::stop>>>;
};

struct ProcessManager::ExitFlow
{
  struct Context
  {
    ProcessManager &owner;
    Process &process;
    int status, signal;
  };

  static void terminal(Context &c) noexcept
  {
    c.owner.groups_.exit_terminal(c.process.membership_);
  }

  static void cancel_wait(Context &c) noexcept
  {
    c.owner.waits_.cancel(c.process);
  }

  static void close_files(Context &c) noexcept
  {
    c.process.descriptors_.close_all();
  }

  static void directory(Context &c) noexcept
  {
    xv::fs::Transaction transaction;
    c.process.directory_.clear();
  }

  static void family(Context &c) noexcept
  {
    c.owner.wait_lock_.acquire();
    c.owner.reparent(&c.process);
    c.owner.wakeup(&c.owner.child_events_);
  }

  static void publish(Context &c) noexcept
  {
    c.process.lock.acquire();
    c.process.xstate = c.status;
    c.process.exit_signal_ = c.signal;
    c.process.state = ProcessState::Zombie;
    c.owner.post_child_signal(c.process);
    c.owner.wait_lock_.release();
  }

  static void suspend(Context &c) noexcept
  {
    c.owner.scheduler_.suspend();
    xv::print::panic("zombie exit");
  }
  struct Terminal;
  struct CancelWait;
  struct CloseFiles;
  struct Directory;
  struct Family;
  struct Publish;
  struct Suspend;
  using Graph = flow::graph<
      Terminal,
      flow::types<
          flow::node<Terminal, flow::action<&terminal>, flow::go<CancelWait>>,
          flow::node<CancelWait, flow::action<&cancel_wait>, flow::go<CloseFiles>>,
          flow::node<CloseFiles, flow::action<&close_files>, flow::go<Directory>>,
          flow::node<Directory, flow::action<&directory>, flow::go<Family>>,
          flow::node<Family, flow::action<&family>, flow::go<Publish>>,
          flow::node<Publish, flow::action<&publish>, flow::go<Suspend>>,
          flow::node<Suspend, flow::action<&suspend>, flow::stop>>>;
};

struct ProcessManager::KillFlow
{
  struct Context
  {
    ProcessManager &owner;
    ProcessPredicate &key;
    Process *process = nullptr;
    int result = -1;
  };

  static void find(Context &c) noexcept
  {
    c.process = c.owner.store_.find(c.key);
  }

  static bool found(Context &c) noexcept
  {
    return c.process != nullptr;
  }

  static bool permitted(Context &c) noexcept
  {
    return c.process != c.owner.initial_ &&
           Credentials::may_signal(current()->credentials().snapshot(),
                                   c.process->credentials().snapshot());
  }

  static void mark(Context &c) noexcept
  {
    c.process->killed = 1;
    c.owner.wake_for_signal(*c.process);
  }

  static void unlock(Context &c) noexcept
  {
    c.process->lock.release();
  }

  static void notify(Context &c) noexcept
  {
    c.owner.notify_child_events();
    c.result = 0;
  }
  struct Find;
  struct Permission;
  struct Mark;
  struct Unlock;
  struct Notify;
  struct Reject;
  using Graph = flow::graph<
      Find, flow::types<flow::node<Find, flow::action<&find>,
                                   flow::branch<flow::condition<&found>,
                                                flow::go<Permission>, flow::stop>>,
                        flow::node<Permission, flow::noop,
                                   flow::branch<flow::condition<&permitted>,
                                                flow::go<Mark>, flow::go<Reject>>>,
                        flow::node<Mark, flow::action<&mark>, flow::go<Unlock>>,
                        flow::node<Unlock, flow::action<&unlock>, flow::go<Notify>>,
                        flow::node<Notify, flow::action<&notify>, flow::stop>,
                        flow::node<Reject, flow::action<&unlock>, flow::stop>>>;
};

struct ProcessManager::InspectFlow
{
  struct Context
  {
    ProcessManager &owner;
    ProcessPredicate &key;
    xv::fs::FileInfo *entries;
    unsigned capacity;
    Process *process = nullptr;
    bool permitted = false;
    int result = -1;
  };

  static void find(Context &c) noexcept
  {
    c.process = c.owner.store_.find(c.key);
  }

  static bool found(Context &c) noexcept
  {
    return c.process != nullptr;
  }

  static void credentials(Context &c) noexcept
  {
    auto caller = current()->credentials().snapshot(),
         target = c.process->credentials().snapshot();
    c.permitted = caller.effective_uid == 0 || (caller.effective_uid == target.uid &&
                                                target.uid == target.effective_uid);
  }

  static bool permitted(Context &c) noexcept
  {
    return c.permitted;
  }

  static void inspect(Context &c) noexcept
  {
    c.result = c.process->files().inspect(c.entries, c.capacity);
  }

  static void unlock(Context &c) noexcept
  {
    c.process->lock.release();
  }
  struct Find;
  struct Credentials;
  struct Inspect;
  struct Unlock;
  using Graph = flow::graph<
      Find, flow::types<flow::node<Find, flow::action<&find>,
                                   flow::branch<flow::condition<&found>,
                                                flow::go<Credentials>, flow::stop>>,
                        flow::node<Credentials, flow::action<&credentials>,
                                   flow::branch<flow::condition<&permitted>,
                                                flow::go<Inspect>, flow::go<Unlock>>>,
                        flow::node<Inspect, flow::action<&inspect>, flow::go<Unlock>>,
                        flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

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
  AllocateFlow::Context context{*this};
  (void)AllocateFlow::Graph::run<0>(context);
  return context.result;
}

// Release process-owned resources and its descriptor,
// including user pages.
// p->lock must be held.
void ProcessManager::reap(Process *p)
{
  ReapFlow::Context context{*this, p};
  (void)ReapFlow::Graph::run<0>(context);
}

// Set up first user process.
void ProcessManager::create_initial_process(void)
{
  InitialFlow::Context context{*this};
  (void)InitialFlow::Graph::run<0>(context);
}

// Family links are only accessed with wait_lock_ held. Unpublished rollback
// objects have no parent, so detach also accepts a never-attached descriptor.
void ProcessManager::attach_child(Process &parent, Process &child)
{
  child.parent = &parent;
  child.sibling_previous_ = parent.last_child_;
  child.sibling_next_ = nullptr;
  if (parent.last_child_)
  {
    parent.last_child_->sibling_next_ = &child;
  }
  else
  {
    parent.first_child_ = &child;
  }
  parent.last_child_ = &child;
}

void ProcessManager::detach_child(Process &child)
{
  auto *parent = child.parent;
  if (!parent)
  {
    return;
  }
  if (child.sibling_previous_)
  {
    child.sibling_previous_->sibling_next_ = child.sibling_next_;
  }
  else
  {
    parent->first_child_ = child.sibling_next_;
  }
  if (child.sibling_next_)
  {
    child.sibling_next_->sibling_previous_ = child.sibling_previous_;
  }
  else
  {
    parent->last_child_ = child.sibling_previous_;
  }
  child.parent = nullptr;
  child.sibling_previous_ = child.sibling_next_ = nullptr;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int ProcessManager::fork_process(void)
{
  ForkFlow::Context context{*this, *current()};
  (void)ForkFlow::Graph::run<0>(context);
  return context.result;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock_.
void ProcessManager::reparent(Process *p)
{
  if (!p->first_child_)
  {
    return;
  }
  ReparentFlow::Context context{*this, *p, p->first_child_};
  (void)ReparentFlow::Graph::run<0>(context);
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
  ExitFlow::Context context{*this, *p, status, signal};
  (void)ExitFlow::Graph::run<0>(context);
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
  KillFlow::Context context{*this, key};
  (void)KillFlow::Graph::run<0>(context);
  return context.result;
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
        : after_(after), result_(result), groups_(groups)
    {
    }

    void visit(Process &process) override
    {
      if (process.state == ProcessState::Unused || process.pid <= after_ ||
          (found && process.pid >= result_.pid))
      {
        return;
      }
      result_ = {};
      result_.pid = process.pid;
      result_.parent = process.parent ? process.parent->pid : 0;
      result_.group = groups_.get_group(process.membership_);
      result_.session = groups_.get_session(process.membership_, 0);
      auto credentials = process.credentials_.snapshot();
      result_.uid = credentials.uid;
      result_.effective_uid = credentials.effective_uid;
      result_.gid = credentials.gid;
      result_.effective_gid = credentials.effective_gid;
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
  if (pid < 1 || capacity < DescriptorTable::Capacity)
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
      return process.id() == pid_ && process.state != ProcessState::Unused;
    }

  private:

    int pid_;
  } predicate(pid);

  InspectFlow::Context context{*this, predicate, entries, capacity};
  (void)InspectFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::process
