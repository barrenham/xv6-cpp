module;
#include "kernel/process/wait/wait.h"
#include "kernel/support/flow.h"

module xv.process;
import :manager;

namespace xv::process
{

namespace flow = xv::flow;

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
    int pid = -1, status = 0, result = -1;
  };

  static void select_event(Context &c) noexcept
  {
    c.have_children = false;
    c.child = nullptr;
    // Family lock pins only this parent's direct children throughout the scan.
    for (auto *candidate = c.parent.first_child_; candidate;
         candidate = candidate->sibling_next_)
    {
      candidate->lock.acquire();
      if (candidate->state != ProcessState::Unused &&
          (c.selector <= 0 || candidate->pid == c.selector) &&
          (!c.group || c.owner.groups_.get_group(candidate->membership_) == c.group))
      {
        c.have_children = true;
        if (candidate->state == ProcessState::Zombie ||
            ((c.options & WUNTRACED) && candidate->stop_event_) ||
            ((c.options & WCONTINUED) && candidate->continued_event_))
        {
          c.child = candidate;
          break;
        }
      }
      candidate->lock.release();
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
          : c.child->stop_event_ ? (c.child->stop_signal_ << 8) | 0x7f
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
    prepare_interruptible_sleep(&c.owner.child_events_);
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
