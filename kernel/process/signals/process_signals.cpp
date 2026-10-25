module;
#include "kernel/process/signals/signal.h"

module xv.process;

namespace xv::process
{
bool Process::is_killed() const
{
  xv::sync::ScopedLock guard(lock);
  return killed != 0 || pending_signals_.terminating_signal() != 0 ||
         pending_signals_.stopping_signal() != 0;
}

void Process::request_kill()
{
  xv::sync::ScopedLock guard(lock);
  killed = 1;
}

int Process::termination_status() const
{
  xv::sync::ScopedLock guard(lock);
  int signal = pending_signals_.terminating_signal();
  return signal ? SIGNAL_EXIT_STATUS(signal) : (killed ? -1 : 0);
}
} // namespace xv::process
