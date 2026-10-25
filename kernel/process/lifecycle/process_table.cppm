module;

#include "kernel/support/param.h"

module xv.process:table;

import :process;
import xv.sync;

namespace xv::process
{
class ProcessTable final
{
public:

  constexpr ProcessTable() = default;
  void initialize();
  void map_kernel_stacks(PageTable table);
  void create_initial_process();
  int fork_process();
  [[noreturn]] void exit_process(int status);
  int wait(unsigned long status_address);
  int kill(int pid);
  void wakeup(void *channel);
  void dump();

private:

  friend class RoundRobinScheduler;
  Process *allocate();
  void reap(Process *process);
  int allocate_pid();
  void reparent(Process *parent);
  Process entries_[NPROC]{};
  Process *initial_ = nullptr;
  int next_pid_ = 1;
  xv::sync::SpinLock pid_lock_;
  xv::sync::SpinLock wait_lock_;
};

extern ProcessTable process_table;
void first_return();
} // namespace xv::process
