module;
#include "kernel/process/wait/wait.h"

module xv.process;
import :manager;

namespace xv::process
{
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
  for (;;)
  {
    class Child final : public ProcessPredicate
    {
    public:

      Child(Process *parent, int pid, int group, int options, ProcessGroups &groups)
          : parent_(parent), pid_(pid), group_(group), options_(options),
            groups_(groups)
      {
      }

      bool matches(Process &child) override
      {
        if (child.state == ProcessState::Unused || child.parent != parent_ ||
            (pid_ > 0 && child.pid != pid_) ||
            (group_ && groups_.get_group(child.membership_) != group_))
        {
          return false;
        }
        have_children = true;
        return child.state == ProcessState::Zombie ||
               ((options_ & WUNTRACED) && child.stop_event_) ||
               ((options_ & WCONTINUED) && child.continued_event_);
      }

      bool have_children = false;

    private:

      Process *parent_;
      int pid_, group_, options_;
      ProcessGroups &groups_;
    } match(parent, selector, group, options, groups_);

    if (auto *child = store_.find(match))
    {
      int pid = child->pid;
      bool exited = child->state == ProcessState::Zombie;
      int status = exited
                       ? (encoded ? (child->exit_signal_ ? child->exit_signal_
                                                         : (child->xstate & 0xff) << 8)
                                  : child->xstate)
                   : child->stop_event_ ? (child->stop_signal_ << 8) | 0x7f
                                        : 0xffff;
      if (address && parent->memory_.copy_out(address, &status, sizeof(status)) < 0)
      {
        child->lock.release();
        wait_lock_.release();
        return -1;
      }
      if (exited)
      {
        reap(child);
      }
      else
      {
        if (WIFSTOPPED(status))
        {
          child->stop_event_ = false;
        }
        else
        {
          child->continued_event_ = false;
        }
        child->lock.release();
      }
      wait_lock_.release();
      return pid;
    }
    if (!match.have_children || parent->is_killed())
    {
      wait_lock_.release();
      return -1;
    }
    if (options & WNOHANG)
    {
      wait_lock_.release();
      return 0;
    }
    prepare_interruptible_sleep(&child_events_);
    wait_lock_.release();
    sleep();
    wait_lock_.acquire();
  }
}
} // namespace xv::process
