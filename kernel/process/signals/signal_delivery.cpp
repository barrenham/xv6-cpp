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
           (c.process.killed || (c.bits & (1U << SIGKILL)) ||
            c.process.pending_signals_.continuing());
  }

  static void continued(Context &c) noexcept
  {
    c.process.stop_event_ = false;
    c.process.continued_event_ = !c.fatal;
    c.process.continue_notification_ = c.process.continue_notification_ || !c.fatal;
    __atomic_store_n(&c.owner.child_notifications_, true, __ATOMIC_RELEASE);
  }

  static bool interrupt(Context &c) noexcept
  {
    return c.process.interruptible_ &&
           (c.process.killed || c.process.pending_signals_.interrupting());
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
    ProcessPredicate &key;
    int signal;
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
    return (c.process != c.owner.initial_ || c.signal == 0) &&
           Credentials::may_signal(current()->credentials().snapshot(),
                                   c.process->credentials().snapshot());
  }

  static bool postable(Context &c) noexcept
  {
    return c.signal && c.process->state != ProcessState::Zombie;
  }

  static void post(Context &c) noexcept
  {
    c.process->pending_signals_.post(c.signal);
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
    c.owner.groups_.visit_group(current()->membership_, c.group, c.post);
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
    xv::sync::ScopedLock family(c.owner.wait_lock_);

    class Continued final : public ProcessVisitor
    {
    public:

      explicit Continued(ProcessManager &manager) : manager_(manager)
      {
      }

      void visit(Process &child) override
      {
        if (!child.continue_notification_)
        {
          return;
        }
        child.continue_notification_ = false;
        manager_.post_child_signal(child);
      }

    private:

      ProcessManager &manager_;
    } continued(c.owner);

    // The family lock pins parent pointers through posting, without a parent lock.
    c.owner.store_.visit(continued);
    c.owner.wakeup(&c.owner.child_events_);
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
    c.status = c.process.termination_status();
  }

  static bool terminating(Context &c) noexcept
  {
    return c.status != 0;
  }

  static void terminate(Context &c) noexcept
  {
    int signal = c.process.pending_signals_.terminating_signal();
    c.owner.exit_process(c.status, signal ? signal : SIGKILL);
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
    c.owner.waits_.cancel(c.process);
    c.owner.wait_lock_.acquire();
    // Notify before taking the child lock, closing the parent's check-to-sleep window.
    c.owner.wakeup(&c.owner.child_events_);
    c.process.lock.acquire();
    c.signal = c.process.pending_signals_.take_stop();
  }

  static bool valid_stop(Context &c) noexcept
  {
    return c.signal && !c.process.killed &&
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
    c.process.stop_event_ = true;
    c.process.continued_event_ = false;
    c.process.state = ProcessState::Stopped;
    c.owner.post_child_signal(c.process);
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
  auto bits = process.pending_signals_.pending();
  bool fatal = process.killed || process.pending_signals_.terminating_signal();
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
    class Pid final : public ProcessPredicate
    {
    public:

      explicit Pid(int id) : id_(id)
      {
      }

      bool matches(Process &process) override
      {
        return process.state != ProcessState::Unused && process.pid == id_;
      }

    private:

      int id_;
    } key(target);

    SendPidSignalFlow::Context context{*this, key, signal};
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
      if (!process || (process == initial_ && signal_ != 0) ||
          !Credentials::may_signal(sender_, process->credentials().snapshot()))
      {
        return;
      }
      ++matched;
      if (signal_)
      {
        process->pending_signals_.post(signal_);
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

    void visit(Process &process) override
    {
      if (process.state != ProcessState::Unused &&
          process.state != ProcessState::Zombie &&
          (process.pending_signals_.pending() || process.pending_signals_.continuing()))
      {
        manager_.wake_for_signal(process);
      }
    }

  private:

    ProcessManager &manager_;
  } wake(*this);

  // No group lock and no borrowed owner pointer survives to this stable visit.
  store_.visit(wake);
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
        process->pending_signals_.post(signal_);
        if (signal_ == SIGHUP)
        {
          process->pending_signals_.post(SIGCONT);
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
  if (child.parent)
  {
    child.parent->pending_signals_.post(SIGCHLD);
    // Default/ignored/blocked SIGCHLD does not interrupt a sleep. wait/waitpid
    // already receive the family event above. Keep the pending bit, but avoid
    // scanning and locking every process unless a signal can interrupt this
    // parent. Its mask/disposition can only change while it is running; the
    // next interruptible sleep rechecks pending signals under its process lock.
    if (child.parent->pending_signals_.interrupting())
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
  DeliverSignalFlow::Context context{*this, *current()};
  (void)DeliverSignalFlow::Graph::run<0>(context);
}
} // namespace xv::process
