module;
#include "kernel/process/signals/signal.h"
#include "kernel/support/flow.h"

module xv.process;
import :manager;

namespace xv::process
{

namespace flow = xv::flow;

struct ProcessManager::WakeSignalFlow
{
  struct Context
  {
    ProcessManager &owner;
    Process &process;
    unsigned bits;
    bool fatal;
  };

  static bool resume(Context &c) noexcept
  {
    return c.process.state == ProcessState::Stopped &&
           (c.process.killed || c.process.exec_stopped() ||
            (c.bits & (1U << SIGKILL)) || c.process.pending_signals_.continuing());
  }

  static void continued(Context &c) noexcept
  {
    c.process.pending_signals_.clear_resume();
    if (auto *resources = c.process.resources_)
    {
      if (!__atomic_fetch_sub(&resources->stopped, 1U, __ATOMIC_ACQ_REL))
      {
        __builtin_trap();
      }
      if (!c.fatal &&
          __atomic_exchange_n(&resources->stop_reported, false, __ATOMIC_ACQ_REL))
      {
        __atomic_store_n(&resources->continue_pending, true, __ATOMIC_RELEASE);
        __atomic_store_n(&c.owner.child_notifications_, true, __ATOMIC_RELEASE);
      }
    }
  }

  static bool interrupt(Context &c) noexcept
  {
    return c.process.interruptible_ && (c.process.killed || c.process.exec_stopped() ||
                                        c.process.pending_signals_.interrupting());
  }

  static void clear_wait(Context &c) noexcept
  {
    c.process.chan = nullptr;
    c.process.interruptible_ = false;
  }

  static bool sleeping(Context &c) noexcept
  {
    return c.process.state == ProcessState::Sleeping;
  }

  static void runnable(Context &c) noexcept
  {
    c.owner.scheduler_.make_runnable(c.process);
  }
  struct Resume;
  struct Continued;
  struct Interrupt;
  struct Clear;
  struct Runnable;
  using Graph = flow::graph<
      Resume,
      flow::types<flow::node<Resume, flow::noop,
                             flow::branch<flow::condition<&resume>, flow::go<Continued>,
                                          flow::go<Interrupt>>>,
                  flow::node<Continued, flow::action<&continued>, flow::go<Runnable>>,
                  flow::node<Interrupt, flow::noop,
                             flow::branch<flow::condition<&interrupt>, flow::go<Clear>,
                                          flow::stop>>,
                  flow::node<Clear, flow::action<&clear_wait>,
                             flow::branch<flow::condition<&sleeping>,
                                          flow::go<Runnable>, flow::stop>>,
                  flow::node<Runnable, flow::action<&runnable>, flow::stop>>>;
};

struct ProcessManager::SendPidSignalFlow
{
  struct Context
  {
    ProcessManager &owner;
    int pid;
    int signal;
    Process *process = nullptr;
    int result = -1;
  };

  static void find(Context &c) noexcept
  {
    c.process = c.owner.store_.find_pid(c.pid);
  }

  static bool found(Context &c) noexcept
  {
    return c.process != nullptr;
  }

  static bool permitted(Context &c) noexcept
  {
    return !c.process->thread_slot_ && (!c.process->kernel_entry_ || c.signal == 0) &&
           (c.process != c.owner.initial_ || c.signal == 0) &&
           Credentials::may_signal(current()->credentials().snapshot(),
                                   c.process->credentials().snapshot());
  }

  static bool postable(Context &c) noexcept
  {
    return c.signal && c.process->state != ProcessState::Zombie;
  }

  static void post(Context &c) noexcept
  {
    c.process->pending_signals_.post_process(c.signal);
    if (c.signal == SIGKILL)
    {
      c.process->request_group_exit(SIGNAL_EXIT_STATUS(SIGKILL), SIGKILL);
    }
    c.owner.wake_for_signal(*c.process);
  }

  static void unlock(Context &c) noexcept
  {
    c.process->lock.release();
  }

  static void notify(Context &c) noexcept
  {
    c.owner.wake_signal_waiters();
    c.result = 0;
  }
  struct Find;
  struct Permission;
  struct Denied;
  struct PostCheck;
  struct Post;
  struct Unlock;
  struct Notify;
  using Graph = flow::graph<
      Find, flow::types<flow::node<Find, flow::action<&find>,
                                   flow::branch<flow::condition<&found>,
                                                flow::go<Permission>, flow::stop>>,
                        flow::node<Permission, flow::noop,
                                   flow::branch<flow::condition<&permitted>,
                                                flow::go<PostCheck>, flow::go<Denied>>>,
                        flow::node<Denied, flow::action<&unlock>, flow::stop>,
                        flow::node<PostCheck, flow::noop,
                                   flow::branch<flow::condition<&postable>,
                                                flow::go<Post>, flow::go<Unlock>>>,
                        flow::node<Post, flow::action<&post>, flow::go<Unlock>>,
                        flow::node<Unlock, flow::action<&unlock>, flow::go<Notify>>,
                        flow::node<Notify, flow::action<&notify>, flow::stop>>>;
};

struct ProcessManager::SendGroupSignalFlow
{
  struct Context
  {
    ProcessManager &owner;
    int group, signal;
    GroupVisitor &post;
    unsigned &matched;
    int result = -1;
  };

  static void visit(Context &c) noexcept
  {
    c.owner.groups_.visit_group(current()->group_leader().membership_, c.group, c.post);
  }

  static bool matched(Context &c) noexcept
  {
    return c.matched != 0;
  }

  static bool signal(Context &c) noexcept
  {
    return c.signal != 0;
  }

  static void wake(Context &c) noexcept
  {
    c.owner.wake_signal_waiters();
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Visit;
  struct Signal;
  struct Wake;
  struct Success;
  using Graph = flow::graph<
      Visit, flow::types<flow::node<Visit, flow::action<&visit>,
                                    flow::branch<flow::condition<&matched>,
                                                 flow::go<Signal>, flow::stop>>,
                         flow::node<Signal, flow::noop,
                                    flow::branch<flow::condition<&signal>,
                                                 flow::go<Wake>, flow::go<Success>>>,
                         flow::node<Wake, flow::action<&wake>, flow::go<Success>>,
                         flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct ProcessManager::NotifyChildFlow
{
  struct Context
  {
    ProcessManager &owner;
  };

  static void collect(Context &c) noexcept
  {
    c.owner.collect_child_notifications();
  }

  static void wake(Context &c) noexcept
  {
    c.owner.wake_signal_waiters(false);
  }

  static bool again(Context &c) noexcept
  {
    return __atomic_exchange_n(&c.owner.child_notifications_, false, __ATOMIC_ACQ_REL);
  }
  struct Collect;
  struct Wake;
  using Graph = flow::graph<
      Collect, flow::types<flow::node<Collect, flow::action<&collect>, flow::go<Wake>>,
                           flow::node<Wake, flow::action<&wake>,
                                      flow::branch<flow::condition<&again>,
                                                   flow::go<Collect>, flow::stop>>>>;
};

struct ProcessManager::DeliverSignalFlow
{
  struct Context
  {
    ProcessManager &owner;
    Process &process;
    int status = 0, signal = 0;
    bool entered = false;
  };

  static void status(Context &c) noexcept
  {
    c.status = c.process.termination_status(&c.signal);
  }

  static bool terminating(Context &c) noexcept
  {
    return c.status != 0 || c.process.group_exit_requested();
  }

  static void terminate(Context &c) noexcept
  {
    c.owner.exit_process(c.status, c.signal ? c.signal : SIGKILL);
  }

  static bool caught(Context &c) noexcept
  {
    return !c.process.stop_requested() && c.process.pending_signals_.caught_signal();
  }

  static void handler(Context &c) noexcept
  {
    c.entered = c.process.enter_signal_handler();
  }

  static bool entered(Context &c) noexcept
  {
    return c.entered;
  }

  static void fault(Context &c) noexcept
  {
    c.owner.exit_process(SIGNAL_EXIT_STATUS(SIGSEGV), SIGSEGV);
  }

  static void defaults(Context &c) noexcept
  {
    if (!(c.process.pending_signals_.handled() & (1U << SIGCHLD)) &&
        !(c.process.pending_signals_.mask() & (1U << SIGCHLD)))
    {
      c.process.pending_signals_.consume(SIGCHLD);
    }
    if (!(c.process.pending_signals_.handled() & (1U << SIGCONT)))
    {
      c.process.pending_signals_.consume_continue();
    }
  }

  static bool stopping(Context &c) noexcept
  {
    return c.process.stop_requested();
  }

  static void prepare(Context &c) noexcept
  {
    c.process.pending_signals_.request_group_stop(
        c.process.pending_signals_.stopping_signal());
    c.owner.wake_signal_waiters(false);
    c.owner.waits_.cancel(c.process);
    c.owner.wait_lock_.acquire();
    c.process.lock.acquire();
    c.signal = c.process.pending_signals_.take_stop();
  }

  static bool valid_stop(Context &c) noexcept
  {
    return c.signal && c.process.pending_signals_.group_stop_signal() &&
           !c.process.killed && !c.process.group_exit_requested() &&
           !c.process.exec_stopped() &&
           !c.process.pending_signals_.terminating_signal();
  }

  static void retry(Context &c) noexcept
  {
    c.process.lock.release();
    c.owner.wait_lock_.release();
  }

  static void publish(Context &c) noexcept
  {
    c.process.stop_signal_ = c.signal;
    c.process.state = ProcessState::Stopped;
    __atomic_add_fetch(&c.process.resources_->stopped, 1U, __ATOMIC_ACQ_REL);
    c.owner.report_group_stop(c.process);
    c.owner.wait_lock_.release();
  }

  static void suspend(Context &c) noexcept
  {
    c.owner.scheduler_.suspend();
    c.process.lock.release();
  }
  struct Status;
  struct Terminate;
  struct Caught;
  struct Handler;
  struct Fault;
  struct Defaults;
  struct Prepare;
  struct Retry;
  struct Publish;
  struct Suspend;
  using Graph = flow::graph<
      Status,
      flow::types<flow::node<Status, flow::action<&status>,
                             flow::branch<flow::condition<&terminating>,
                                          flow::go<Terminate>, flow::go<Caught>>>,
                  flow::node<Terminate, flow::action<&terminate>, flow::stop>,
                  flow::node<Caught, flow::noop,
                             flow::branch<flow::condition<&caught>, flow::go<Handler>,
                                          flow::go<Defaults>>>,
                  flow::node<Handler, flow::action<&handler>,
                             flow::branch<flow::condition<&entered>, flow::stop,
                                          flow::go<Fault>>>,
                  flow::node<Fault, flow::action<&fault>, flow::stop>,
                  flow::node<Defaults, flow::action<&defaults>,
                             flow::branch<flow::condition<&stopping>, flow::go<Prepare>,
                                          flow::stop>>,
                  flow::node<Prepare, flow::action<&prepare>,
                             flow::branch<flow::condition<&valid_stop>,
                                          flow::go<Publish>, flow::go<Retry>>>,
                  flow::node<Retry, flow::action<&retry>, flow::go<Status>>,
                  flow::node<Publish, flow::action<&publish>, flow::go<Suspend>>,
                  flow::node<Suspend, flow::action<&suspend>, flow::go<Status>>>>;
};

void ProcessManager::wake_for_signal(Process &process)
{
  // Selecting a fatal process-pending signal transfers it to this thread.
  // Sample the local bits afterwards, including SIGKILL for stopped tasks.
  bool fatal = process.killed || process.exec_stopped() ||
               process.group_exit_requested() ||
               process.pending_signals_.terminating_signal();
  auto bits = process.pending_signals_.pending();
  WakeSignalFlow::Context context{*this, process, bits, fatal};
  (void)WakeSignalFlow::Graph::run<0>(context);
}

int ProcessManager::send_signal(int target, int signal)
{
  if ((signal != 0 && !supported_signal(signal)) || target == -1 ||
      target < -0x7fffffff)
  {
    return -1;
  }
  if (target > 0)
  {
    SendPidSignalFlow::Context context{*this, target, signal};
    (void)SendPidSignalFlow::Graph::run<0>(context);
    return context.result;
  }

  int group = target == 0 ? 0 : -target;

  class Post final : public GroupVisitor
  {
  public:

    Post(Process *initial, int signal, CredentialState sender)
        : initial_(initial), signal_(signal), sender_(sender)
    {
    }

    void visit(ProcessMembership &member) override
    {
      auto *process = member.owner();
      if (!process || (process->kernel_entry_ && signal_ != 0) ||
          (process == initial_ && signal_ != 0) ||
          !Credentials::may_signal(sender_, process->credentials().snapshot()))
      {
        return;
      }
      ++matched;
      if (signal_)
      {
        process->pending_signals_.post_process(signal_);
        if (signal_ == SIGKILL)
        {
          process->request_group_exit(SIGNAL_EXIT_STATUS(SIGKILL), SIGKILL);
        }
      }
    }

    unsigned matched = 0;

  private:

    Process *initial_;
    int signal_;
    CredentialState sender_;
  } post(initial_, signal, current()->credentials().snapshot());

  // Membership pins owners only during the callback; wake after releasing the group lock.
  SendGroupSignalFlow::Context context{*this, group, signal, post, post.matched};
  (void)SendGroupSignalFlow::Graph::run<0>(context);
  return context.result;
}

void ProcessManager::wake_signal_waiters(bool notify_children)
{
  class Wake final : public ProcessVisitor
  {
  public:

    explicit Wake(ProcessManager &manager) : manager_(manager)
    {
    }

    bool stopping = false;

    void visit(Process &process) override
    {
      if (process.group_exit_requested() && process.state != ProcessState::Zombie &&
          process.state != ProcessState::ThreadZombie)
      {
        process.killed = 1;
      }
      if (process.state != ProcessState::Unused &&
          process.state != ProcessState::Zombie &&
          process.state != ProcessState::ThreadZombie &&
          (process.killed || process.exec_stopped() ||
           process.pending_signals_.pending() ||
           process.pending_signals_.process_pending() ||
           process.pending_signals_.group_stop_signal() ||
           process.pending_signals_.continuing()))
      {
        manager_.wake_for_signal(process);
        stopping = stopping || process.pending_signals_.group_stop_signal();
      }
    }

  private:

    ProcessManager &manager_;
  } wake(*this);

  // No group lock and no borrowed owner pointer survives to this stable visit.
  store_.visit(wake);
  if (wake.stopping)
  {
    store_.visit(wake);
    // The child-notification consumer itself wakes signal waiters with false.
    // Requeueing here would keep that consumer looping while a group is stopped.
    if (notify_children)
    {
      __atomic_store_n(&child_notifications_, true, __ATOMIC_RELEASE);
    }
  }
  if (notify_children)
  {
    notify_child_events();
  }
}

void ProcessManager::post_terminal_interrupt(void *terminal, int signal)
{
  class Post final : public GroupVisitor
  {
  public:

    Post(Process *initial, int signal) : initial_(initial), signal_(signal)
    {
    }

    void visit(ProcessMembership &member) override
    {
      auto *process = member.owner();
      if (process && process != initial_)
      {
        process->pending_signals_.post_process(signal_);
        if (signal_ == SIGKILL)
        {
          process->request_group_exit(SIGNAL_EXIT_STATUS(SIGKILL), SIGKILL);
        }
        if (signal_ == SIGHUP)
        {
          process->pending_signals_.post_process(SIGCONT);
        }
      }
    }

  private:

    Process *initial_;
    int signal_;
  } post(initial_, signal);

  // IRQ path: the group lock pins owners; no process/store locks are acquired.
  groups_.visit_foreground(terminal, post);
  __atomic_store_n(&signal_notifications_, true, __ATOMIC_RELEASE);
}

void ProcessManager::service_signal_notifications()
{
  notify_child_events();
  if (__atomic_exchange_n(&signal_notifications_, false, __ATOMIC_ACQ_REL))
  {
    wake_signal_waiters();
  }
}

int ProcessManager::set_signal_disposition(int signal, int action)
{
  auto *process = current();
  xv::sync::ScopedLock guard(process->lock);
  return process->pending_signals_.disposition(signal, action);
}

void ProcessManager::post_child_signal(Process &child)
{
  // wait_lock_ pins the parent across posting/reparent/reap. The child lock may
  // be held: do not acquire the parent lock or scan the registry here.
  refresh_child_events(child);
  if (child.parent)
  {
    child.parent->pending_signals_.post_process(SIGCHLD);
    // wait/waitpid already receive the family event above. Keep the pending
    // bit, but a default/ignored SIGCHLD needs no full-registry signal wakeup.
    // A caught disposition is shared by the group: wake even when the leader
    // blocks SIGCHLD, since another thread may be eligible to run its handler.
    // A later mask/action change rechecks pending signals on its own path.
    if (child.parent->pending_signals_.handled() & (1U << SIGCHLD))
    {
      __atomic_store_n(&signal_notifications_, true, __ATOMIC_RELEASE);
    }
  }
}

void ProcessManager::notify_child_events()
{
  if (!__atomic_exchange_n(&child_notifications_, false, __ATOMIC_ACQ_REL))
  {
    return;
  }
  NotifyChildFlow::Context context{*this};
  (void)NotifyChildFlow::Graph::run<0>(context);
}

void ProcessManager::deliver_signals()
{
  if (current()->exec_stopped())
  {
    exit_thread(0);
  }
  DeliverSignalFlow::Context context{*this, *current()};
  (void)DeliverSignalFlow::Graph::run<0>(context);
}
} // namespace xv::process
