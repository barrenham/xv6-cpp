module;
#include "kernel/process/signals/signal.h"

module xv.process;

namespace xv::process
{
bool Process::is_killed() const
{
  xv::sync::ScopedLock guard(lock);
  if (pending_signals_.caught_signal()) syscall_interrupted_ = true;
  return killed != 0 || exec_stopped() || group_exit_requested() ||
         pending_signals_.interrupting();
}

void Process::request_kill()
{
  xv::sync::ScopedLock guard(lock);
  killed = 1;
}

int
Process::termination_status(int *selected) const {
  xv::sync::ScopedLock guard(lock);
  auto request =
      resources_ ? __atomic_load_n(&resources_->exit_request, __ATOMIC_ACQUIRE) : 0;
  if (request)
  {
    if (selected)
    {
      *selected = (request >> 1) & 127;
    }
    return static_cast<int>(static_cast<unsigned>(request >> 16));
  }
  int signal = pending_signals_.terminating_signal();
  if (selected)
  {
    *selected = signal;
  }
  return signal ? SIGNAL_EXIT_STATUS(signal) : (killed ? -1 : 0);
}
} // namespace xv::process
