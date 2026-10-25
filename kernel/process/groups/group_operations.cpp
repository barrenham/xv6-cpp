module;
#include "kernel/process/signals/signal.h"

module xv.process;
import :manager;

namespace xv::process
{
int ProcessManager::get_process_group()
{
  return groups_.get_group(current()->membership_);
}

int ProcessManager::set_process_group(int pid, int group)
{
  return groups_.set_group(current()->membership_, pid, group);
}

int ProcessManager::get_session(int pid)
{
  return groups_.get_session(current()->membership_, pid);
}

int ProcessManager::create_session()
{
  return groups_.create_session(current()->membership_);
}

void ProcessManager::did_exec(Process &process)
{
  groups_.did_exec(process.membership_);
}

int ProcessManager::claim_terminal(void *terminal)
{
  return groups_.claim_terminal(current()->membership_, terminal);
}

int ProcessManager::terminal_foreground(void *terminal)
{
  return groups_.foreground(current()->membership_, terminal);
}

int ProcessManager::set_terminal_foreground(void *terminal, int group)
{
  return groups_.set_foreground(current()->membership_, terminal, group);
}

bool ProcessManager::may_read_terminal(void *terminal)
{
  auto *process = current();
  if (groups_.may_read_terminal(process->membership_, terminal))
  {
    return true;
  }
  process->pending_signals_.post(SIGTTIN);
  return false;
}
} // namespace xv::process
