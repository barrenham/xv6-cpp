module;
#include "kernel/process/signals/signal.h"

module xv.process;
import :manager;

namespace xv::process
{
int ProcessManager::get_process_group()
{
  return groups_.get_group(current()->group_leader().membership_);
}

int ProcessManager::set_process_group(int pid, int group)
{
  return groups_.set_group(current()->group_leader().membership_, pid, group);
}

int ProcessManager::get_session(int pid)
{
  return groups_.get_session(current()->group_leader().membership_, pid);
}

int ProcessManager::create_session()
{
  return groups_.create_session(current()->group_leader().membership_);
}

void ProcessManager::did_exec(Process &process)
{
  groups_.did_exec(process.group_leader().membership_);
}

void ProcessManager::detach_terminal(void *terminal)
{
  groups_.detach_terminal(terminal);
}

void *
ProcessManager::pin_controlling_terminal(void *(*pin)(void *)) {
  return groups_.pin_terminal(current()->group_leader().membership_, pin);
}

int ProcessManager::claim_terminal(void *terminal)
{
  return groups_.claim_terminal(current()->group_leader().membership_, terminal);
}

int ProcessManager::terminal_foreground(void *terminal)
{
  return groups_.foreground(current()->group_leader().membership_, terminal);
}

int ProcessManager::set_terminal_foreground(void *terminal, int group)
{
  return groups_.set_foreground(current()->group_leader().membership_, terminal, group);
}

bool ProcessManager::may_read_terminal(void *terminal)
{
  auto *process = current();
  if (groups_.may_read_terminal(process->group_leader().membership_, terminal))
  {
    return true;
  }
  process->pending_signals_.post_process(SIGTTIN);
  return false;
}
} // namespace xv::process
