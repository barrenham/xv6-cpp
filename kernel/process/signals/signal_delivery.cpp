module;
#include "kernel/process/signals/signal.h"

module xv.process;
import :manager;

namespace xv::process
{
void ProcessManager::wake_for_signal(Process &process)
{
  auto bits = process.pending_signals_.pending();
  bool fatal = process.killed || process.pending_signals_.terminating_signal();
  if (process.state == ProcessState::Stopped &&
      (process.killed || (bits & ((1U << SIGKILL) | (1U << SIGCONT)))))
  {
    process.stop_event_ = false;
    process.continued_event_ = !fatal;
    __atomic_store_n(&child_notifications_, true, __ATOMIC_RELEASE);
    scheduler_.make_runnable(process);
    return;
  }
  if (!process.interruptible_ ||
      (!fatal && !process.pending_signals_.stopping_signal()))
  {
    return;
  }
  process.chan = nullptr;
  process.interruptible_ = false;
  if (process.state == ProcessState::Sleeping)
  {
    scheduler_.make_runnable(process);
  }
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

    auto *process = store_.find(key);
    if (!process)
    {
      return -1;
    }
    if (process == initial_ && signal != 0)
    {
      process->lock.release();
      return -1;
    }
    if (signal && process->state != ProcessState::Zombie)
    {
      process->pending_signals_.post(signal);
      wake_for_signal(*process);
    }
    process->lock.release();
    notify_child_events();
    return 0;
  }

  int group = target == 0 ? 0 : -target;

  class Post final : public GroupVisitor
  {
  public:

    Post(Process *initial, int signal) : initial_(initial), signal_(signal)
    {
    }

    void visit(ProcessMembership &member) override
    {
      auto *process = member.owner();
      if (!process || (process == initial_ && signal_ != 0))
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
  } post(initial_, signal);

  // Membership pins embedded owners only during this callback. Post atomically
  // under the group lock; never invert the lifetime path's process -> group order.
  groups_.visit_group(current()->membership_, group, post);
  if (!post.matched)
  {
    return -1;
  }
  if (signal)
  {
    wake_signal_waiters();
  }
  return 0;
}

void ProcessManager::wake_signal_waiters()
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
          process.state != ProcessState::Zombie && process.pending_signals_.pending())
      {
        manager_.wake_for_signal(process);
      }
    }

  private:

    ProcessManager &manager_;
  } wake(*this);

  // No group lock and no borrowed owner pointer survives to this stable visit.
  store_.visit(wake);
  notify_child_events();
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
        if (signal_ == SIGHUP) process->pending_signals_.post(SIGCONT);
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

void ProcessManager::notify_child_events()
{
  if (!__atomic_exchange_n(&child_notifications_, false, __ATOMIC_ACQ_REL))
  {
    return;
  }
  xv::sync::ScopedLock family(wait_lock_);
  wakeup(&child_events_);
}

void ProcessManager::deliver_signals()
{
  auto *process = current();
  for (;;)
  {
    int status = process->termination_status();
    if (status != 0)
    {
      int signal = process->pending_signals_.terminating_signal();
      exit_process(status, signal ? signal : SIGKILL);
    }
    process->pending_signals_.consume_continue();
    if (!process->stop_requested())
    {
      return;
    }
    waits_.cancel(*process);
    wait_lock_.acquire();
    // Notify before acquiring the child lock; the family lock closes the
    // parent's check-to-sleep window, as on exit.
    wakeup(&child_events_);
    process->lock.acquire();
    int signal = process->pending_signals_.take_stop();
    if (!signal || process->killed || process->pending_signals_.terminating_signal())
    {
      process->lock.release();
      wait_lock_.release();
      continue;
    }
    process->stop_signal_ = signal;
    process->stop_event_ = true;
    process->continued_event_ = false;
    process->state = ProcessState::Stopped;
    wait_lock_.release();
    scheduler_.suspend();
    process->lock.release();
  }
}
} // namespace xv::process
