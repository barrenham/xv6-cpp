module;
#include "kernel/process/wait/wait.h"
#include "kernel/support/flow.h"

module xv.process;
import :manager;

namespace xv::process
{

namespace flow = xv::flow;

// Family lock protects every queue link; callers also hold the child's lock
// when deriving queue membership from its state. No allocations on event paths.
void ProcessManager::update_child_event(Process &child, unsigned kind, bool pending)
{
  auto *parent = child.parent;
  if (!parent) return;
  const auto bit = 1U << kind;
  const bool linked = child.child_event_mask_ & bit;
  if (linked == pending) return;
  auto &links = child.child_event_links_[kind];
  if (pending)
  {
    links.previous = parent->child_event_tails_[kind];
    links.next = nullptr;
    if (links.previous) links.previous->child_event_links_[kind].next = &child;
    else parent->child_events_[kind] = &child;
    parent->child_event_tails_[kind] = &child;
    child.child_event_mask_ |= bit;
  }
  else
  {
    if (links.previous) links.previous->child_event_links_[kind].next = links.next;
    else parent->child_events_[kind] = links.next;
    if (links.next) links.next->child_event_links_[kind].previous = links.previous;
    else parent->child_event_tails_[kind] = links.previous;
    links = {};
    child.child_event_mask_ &= ~bit;
  }
}

void ProcessManager::refresh_child_events(Process &child)
{
  const bool zombie = child.state == ProcessState::Zombie;
  update_child_event(child, 0, zombie);
  update_child_event(child, 1, !zombie && child.stop_event_);
  update_child_event(child, 2, !zombie && child.continued_event_);
}

void ProcessManager::remove_child_events(Process &child)
{
  for (unsigned kind = 0; kind < 3; ++kind) update_child_event(child, kind, false);
}

void ProcessManager::queue_child_notification(Process &child)
{
  // Producers hold the child lock. Never acquire a process/family lock while
  // holding this queue lock; the consumer pins lifetime with the family lock.
  xv::sync::ScopedLock guard(child_notification_lock_);
  if (!child.continue_notify_queued_)
  {
    child.continue_notify_queued_ = true;
    child.continue_notify_previous_ = nullptr;
    child.continue_notify_next_ = child_notification_head_;
    if (child_notification_head_) child_notification_head_->continue_notify_previous_ = &child;
    child_notification_head_ = &child;
  }
  __atomic_store_n(&child_notifications_, true, __ATOMIC_RELEASE);
}

void ProcessManager::cancel_child_notification(Process &child)
{
  xv::sync::ScopedLock guard(child_notification_lock_);
  if (!child.continue_notify_queued_) return;
  if (child.continue_notify_previous_)
    child.continue_notify_previous_->continue_notify_next_ = child.continue_notify_next_;
  else child_notification_head_ = child.continue_notify_next_;
  if (child.continue_notify_next_)
    child.continue_notify_next_->continue_notify_previous_ = child.continue_notify_previous_;
  child.continue_notify_previous_ = child.continue_notify_next_ = nullptr;
  child.continue_notify_queued_ = false;
}

void ProcessManager::collect_child_notifications()
{
  xv::sync::ScopedLock family(wait_lock_);
  for (;;)
  {
    child_notification_lock_.acquire();
    auto *child = child_notification_head_;
    if (child)
    {
      child_notification_head_ = child->continue_notify_next_;
      if (child_notification_head_) child_notification_head_->continue_notify_previous_ = nullptr;
      child->continue_notify_previous_ = child->continue_notify_next_ = nullptr;
      child->continue_notify_queued_ = false;
    }
    child_notification_lock_.release();
    if (!child) break;
    child->lock.acquire();
    const bool notify = child->continue_notification_;
    child->continue_notification_ = false;
    if (notify) post_child_signal(*child);
    auto *parent = child->parent;
    child->lock.release();
    // Family lock pins the saved parent and excludes wait's check/register gap.
    // Take the wait bucket only after releasing the child lock.
    if (notify && parent) wakeup(&parent->child_events_);
  }
}

struct ProcessManager::WaitFlow
{
  struct Context
  {
    ProcessManager &owner;
    Process &parent;
    int selector;
    unsigned long address;
    int options;
    bool encoded;
    int group;
    bool have_children = false, exited = false;
    Process *child = nullptr;
    unsigned event_kind = 0;
    int pid = -1, status = 0, result = -1;
  };

  static void select_event(Context &c) noexcept
  {
    c.have_children = false;
    c.child = nullptr;
    if (c.selector > 0)
    {
      auto *candidate = c.owner.store_.find_pid(c.selector);
      if (candidate)
      {
        if (candidate->parent == &c.parent)
        {
          c.have_children = true;
          refresh_child_events(*candidate);
          for (unsigned kind = 0; kind < 3; ++kind)
            if ((candidate->child_event_mask_ & (1U << kind)) &&
                (!kind || (c.options & (kind == 1 ? WUNTRACED : WCONTINUED))))
            { c.child = candidate; c.event_kind = kind; break; }
        }
        if (!c.child) candidate->lock.release();
      }
    }
    else
    {
      // Examine pending events only. Exit-only wait never walks live children.
      for (unsigned kind = 0; kind < 3 && !c.child; ++kind)
      {
        if (kind && !(c.options & (kind == 1 ? WUNTRACED : WCONTINUED))) continue;
        for (auto *candidate = c.parent.child_events_[kind]; candidate;)
        {
          auto *next = candidate->child_event_links_[kind].next;
          candidate->lock.acquire();
          // SIGCONT publishes state before its deferred family notification.
          // Reconcile an old stop entry before exposing it to the caller.
          refresh_child_events(*candidate);
          if ((candidate->child_event_mask_ & (1U << kind)) &&
              (!c.group || c.owner.groups_.get_group(candidate->membership_) == c.group))
          { c.child = candidate; c.event_kind = kind; break; }
          candidate->lock.release();
          candidate = next;
        }
      }
      c.have_children = c.child || (!c.group && c.parent.first_child_);
      // Group-filtered ECHILD still needs to distinguish live group members
      // from an empty group; group membership changes independently of events.
      if (!c.have_children && c.group)
        for (auto *candidate = c.parent.first_child_; candidate; candidate = candidate->sibling_next_)
        {
          candidate->lock.acquire();
          c.have_children = c.owner.groups_.get_group(candidate->membership_) == c.group;
          candidate->lock.release();
          if (c.have_children) break;
        }
    }
    if (c.child)
    {
      c.pid = c.child->pid;
      c.exited = c.child->state == ProcessState::Zombie;
      c.status =
          c.exited
              ? (c.encoded ? (c.child->exit_signal_ ? c.child->exit_signal_
                                                    : (c.child->xstate & 0xff) << 8)
                           : c.child->xstate)
          : c.event_kind == 1 ? (c.child->stop_signal_ << 8) | 0x7f
                                 : 0xffff;
    }
  }

  static bool found(Context &c) noexcept
  {
    return c.child != nullptr;
  }

  static bool copy_status(Context &c) noexcept
  {
    return !c.address ||
           c.parent.memory_.copy_out(c.address, &c.status, sizeof(c.status)) >= 0;
  }

  static bool exited(Context &c) noexcept
  {
    return c.exited;
  }

  static void reap(Context &c) noexcept
  {
    c.owner.reap(c.child);
  }

  static void consume(Context &c) noexcept
  {
    if (WIFSTOPPED(c.status))
    {
      c.child->stop_event_ = false;
    }
    else
    {
      c.child->continued_event_ = false;
    }
    refresh_child_events(*c.child);
    c.child->lock.release();
  }

  static void success(Context &c) noexcept
  {
    c.owner.wait_lock_.release();
    c.result = c.pid;
  }

  static void unlock_child(Context &c) noexcept
  {
    c.child->lock.release();
  }

  static bool eligible(Context &c) noexcept
  {
    return c.have_children && !c.parent.is_killed();
  }

  static bool nonblocking(Context &c) noexcept
  {
    return c.options & WNOHANG;
  }

  static void nohang(Context &c) noexcept
  {
    c.owner.wait_lock_.release();
    c.result = 0;
  }

  static void wait(Context &c) noexcept
  {
    prepare_interruptible_sleep(&c.parent.child_events_);
    c.owner.wait_lock_.release();
    sleep();
    c.owner.wait_lock_.acquire();
  }

  static void failure(Context &c) noexcept
  {
    c.owner.wait_lock_.release();
  }
  struct Scan;
  struct Copy;
  struct EventKind;
  struct Reap;
  struct Consume;
  struct Success;
  struct UnlockChild;
  struct Eligible;
  struct Nonblocking;
  struct NoHang;
  struct Wait;
  struct Failure;
  using Graph = flow::graph<
      Scan, flow::types<
                flow::node<Scan, flow::action<&select_event>,
                           flow::branch<flow::condition<&found>, flow::go<Copy>,
                                        flow::go<Eligible>>>,
                flow::node<Copy, flow::noop,
                           flow::branch<flow::condition<&copy_status>,
                                        flow::go<EventKind>, flow::go<UnlockChild>>>,
                flow::node<EventKind, flow::noop,
                           flow::branch<flow::condition<&exited>, flow::go<Reap>,
                                        flow::go<Consume>>>,
                flow::node<Reap, flow::action<&reap>, flow::go<Success>>,
                flow::node<Consume, flow::action<&consume>, flow::go<Success>>,
                flow::node<Success, flow::action<&success>, flow::stop>,
                flow::node<UnlockChild, flow::action<&unlock_child>, flow::go<Failure>>,
                flow::node<Eligible, flow::noop,
                           flow::branch<flow::condition<&eligible>,
                                        flow::go<Nonblocking>, flow::go<Failure>>>,
                flow::node<Nonblocking, flow::noop,
                           flow::branch<flow::condition<&nonblocking>, flow::go<NoHang>,
                                        flow::go<Wait>>>,
                flow::node<NoHang, flow::action<&nohang>, flow::stop>,
                flow::node<Wait, flow::action<&wait>, flow::go<Scan>>,
                flow::node<Failure, flow::action<&failure>, flow::stop>>>;
};

int ProcessManager::wait(unsigned long address)
{
  return wait_child(-1, address, 0, false);
}

int ProcessManager::waitpid(int pid, unsigned long address, int options)
{
  if ((options & ~(WNOHANG | WUNTRACED | WCONTINUED)) || pid < -0x7fffffff)
  {
    return -1;
  }
  return wait_child(pid, address, options, true);
}

int ProcessManager::wait_child(int selector, unsigned long address, int options,
                               bool encoded)
{
  Process *parent = current();
  int group = selector == 0   ? groups_.get_group(parent->membership_)
              : selector < -1 ? -selector
                              : 0;
  wait_lock_.acquire();
  WaitFlow::Context context{*this, *parent, selector, address, options, encoded, group};
  (void)WaitFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::process
