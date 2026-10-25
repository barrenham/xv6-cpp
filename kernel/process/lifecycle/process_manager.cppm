module xv.process:manager;

import :process;
import :store;
import :scheduler;
import :wait_channels;
import xv.sync;

namespace xv::process
{
class ProcessManager final
{
public:

  constexpr ProcessManager(ProcessStore &store, Scheduler &scheduler,
                           WaitChannels &waits)
      : store_(store), scheduler_(scheduler), waits_(waits)
  {
  }

  void initialize();
  void create_initial_process();
  int fork_process();
  [[noreturn]] void exit_process(int status);
  int wait(unsigned long status_address);
  int kill(int pid);
  void wakeup(void *channel);
  void dump();

private:

  Process *allocate();
  void reap(Process *process);
  int allocate_pid();
  void reparent(Process *parent);
  ProcessStore &store_;
  Scheduler &scheduler_;
  WaitChannels &waits_;
  Process *initial_ = nullptr;
  unsigned next_pid_ = 1;
  xv::sync::SpinLock pid_lock_;
  xv::sync::SpinLock wait_lock_;
};

extern ProcessManager process_manager;
void first_return();
} // namespace xv::process
