module xv.process:manager;

import :process;
import :store;
import :scheduler;
import :wait_channels;
import :groups;
import xv.sync;

namespace xv::process
{
class ProcessManager final
{
public:

  constexpr ProcessManager(ProcessStore &store, Scheduler &scheduler,
                           WaitChannels &waits, ProcessGroups &groups)
      : store_(store), scheduler_(scheduler), waits_(waits), groups_(groups)
  {
  }

  void initialize();
  void create_initial_process();
  int fork_process();
  [[noreturn]] void exit_process(int status, int signal = 0);
  int wait(unsigned long status_address);
  int waitpid(int pid, unsigned long address, int options);
  int kill(int pid);
  int send_signal(int target, int signal);
  void deliver_signals();
  int set_signal_disposition(int signal, int action);
  void post_terminal_interrupt(void *terminal, int signal);
  void service_signal_notifications();
  int claim_terminal(void *terminal);
  int terminal_foreground(void *terminal);
  int set_terminal_foreground(void *terminal, int group);
  bool may_read_terminal(void *terminal);
  void wakeup(void *channel);
  void dump();
  int get_process_group();
  int set_process_group(int pid, int group);
  int get_session(int pid);
  int create_session();
  void did_exec(Process &process);

private:

  void wake_for_signal(Process &process);
  void wake_signal_waiters();
  void notify_child_events();
  int wait_child(int pid, unsigned long address, int options, bool encoded);
  int child_events_ = 0;
  bool child_notifications_ = false;
  bool signal_notifications_ = false;
  Process *allocate();
  void reap(Process *process);
  int allocate_pid();
  void reparent(Process *parent);
  ProcessStore &store_;
  Scheduler &scheduler_;
  WaitChannels &waits_;
  ProcessGroups &groups_;
  Process *initial_ = nullptr;
  unsigned next_pid_ = 1;
  xv::sync::SpinLock pid_lock_;
  xv::sync::SpinLock wait_lock_;
};

extern ProcessManager process_manager;
void first_return();
} // namespace xv::process
