module;
#include "kernel/process/scheduler/policies/scheduling_policy.h"
#include "kernel/process/exec/exec_snapshot.h"
export module xv.process:process;

export import :registers;
export import :resources;
export import :address_space;
import xv.sync;
import :identity;
export import :signals;
export import :credentials;

export namespace xv::process
{
using ExecSnapshot=xv::exec::Snapshot;
struct ProcessInfo
{
  int pid = 0, parent = 0, group = 0, session = 0;
  unsigned uid = 0, effective_uid = 0, gid = 0, effective_gid = 0;
  unsigned long bytes = 0;
  char state = 0;
  char name[16]{};
};

enum class ProcessState
{
  Unused,
  Used,
  Sleeping,
  Runnable,
  Running,
  Stopped,
  Zombie
};

struct CpuState;

class Process final
{
public:

  ~Process() = default;
  Process(const Process &) = delete;
  Process &operator=(const Process &) = delete;

  int id() const
  {
    return pid;
  }

  const char *debug_name() const
  {
    return name;
  }

  void set_name(const char *label);
  bool is_killed() const;
  void request_kill();
  int termination_status() const;
  int signal_action(int signal, unsigned long action, unsigned long old,
                    unsigned long restorer);
  int signal_mask(int how, unsigned long set, unsigned long old);
  int signal_pending(unsigned long set);
  int signal_suspend(unsigned long set);
  unsigned long signal_return();
  void begin_system_call();
  long finish_system_call(int number, unsigned long argument, long result);

  inline bool stop_requested() const
  {
    return pending_signals_.stopping_signal() != 0;
  }

  int pin_cpu(int index);

  unsigned long kernel_stack() const
  {
    return kstack;
  }

  TrapFrame *registers()
  {
    return trapframe;
  }

  AddressSpace &memory()
  {
    return memory_;
  }

  const AddressSpace &memory() const
  {
    return memory_;
  }

  Credentials &credentials()
  {
    return credentials_;
  }

  const Credentials &credentials() const
  {
    return credentials_;
  }

  DescriptorTable &files()
  {
    return descriptors_;
  }

  WorkingDirectory &directory()
  {
    return directory_;
  }

  bool create_image(AddressSpace &image);
  void commit_image(AddressSpace &image, unsigned long entry, unsigned long stack,
                    unsigned long arguments, const char *path,
                    unsigned long environment = 0,ExecSnapshot *snapshot=nullptr);

private:
  ExecSnapshot exec_snapshot_;

  struct CommitFlow;
  struct FirstReturnFlow;

  struct SignalActionFlow;
  struct SignalMaskFlow;
  struct SignalSuspendFlow;
  struct EnterSignalFlow;
  struct ReturnSignalFlow;

  bool enter_signal_handler();
  constexpr Process() = default;
  friend class ProcessManager;
  friend class ProcessStore;
  friend class ReadyQueue;
  friend class WaitChannels;
  friend class PollWait;
  friend class SchedulerContext;
  friend void prepare_sleep(void *channel);
  friend void sleep();
  friend void first_return();

  mutable xv::sync::SpinLock lock;
  ProcessState state = ProcessState::Unused;
  void *chan = nullptr;
  int killed = 0; // legacy kill/fault termination
  PendingSignals pending_signals_;
  unsigned long signal_frames_[16]{};
  unsigned signal_depth_ = 0;
  unsigned suspend_mask_ = 0;
  bool suspended_mask_ = false;
  bool suspend_restarted_ = false;
  mutable bool syscall_interrupted_ = false;
  bool interruptible_ = false;
  int xstate = 0;
  int exit_signal_ = 0;
  int stop_signal_ = 0;
  bool stop_event_ = false;
  bool continued_event_ = false;
  bool continue_notification_ = false; // independent of waitpid event consumption
  int pid = 0;
  void *storage_node_ = nullptr; // immutable registry owner, set before publication
  Process *parent = nullptr; // protected by the manager's family lock
  // Intrusive family links are protected by the same family lock, which pins
  // linked descriptors against reap; process locks protect their wait events.
  Process *first_child_ = nullptr;
  Process *last_child_ = nullptr;
  Process *sibling_previous_ = nullptr;
  Process *sibling_next_ = nullptr;
  // Queue membership is protected by the family lock. Each kind has separate
  // links, while its pending state remains protected by the child process lock.
  struct ChildEventLinks { Process *previous = nullptr, *next = nullptr; };
  ChildEventLinks child_event_links_[3]{};
  unsigned child_event_mask_ = 0;
  Process *child_events_[3]{}, *child_event_tails_[3]{};
  Process *continue_notify_previous_ = nullptr, *continue_notify_next_ = nullptr;
  bool continue_notify_queued_ = false;
  ProcessMembership membership_{this}; // protected by ProcessGroups
  unsigned long kstack = 0;
  AddressSpace memory_;
  TrapFrame *trapframe = nullptr;
  void *usyscall = nullptr;
  Context context{};
  Credentials credentials_;
  DescriptorTable descriptors_;
  WorkingDirectory directory_;
  char name[16]{};
  struct CpuState *pincpu = nullptr;
  void *wait_key_ = nullptr;
  Process *wait_next_ = nullptr;
  Process *ready_next_ = nullptr;
  bool queued_ = false;
  xv::scheduling::Entity scheduling_;
};

// Per-hart architectural state. Interrupt nesting is accessed by the sync
// platform; current/context are managed by the scheduler with interrupts off.
struct CpuState
{
  Process *current = nullptr;
  Context context{};
  int noff = 0;
  int intena = 0;
};

int cpu_id();
CpuState *current_cpu(); // interrupts must be off
Process *current();      // null on an idle hart / before scheduling
} // namespace xv::process
