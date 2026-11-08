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
using ExecSnapshot = xv::exec::Snapshot;

struct ProcessInfo
{
  int pid = 0, parent = 0, group = 0, session = 0;
  unsigned uid = 0, effective_uid = 0, gid = 0, effective_gid = 0;
  unsigned long bytes = 0;
  char state = 0;
  char name[16]{};
};

struct ThreadInfo
{
  int tid = 0, pid = 0, parent = 0;
  unsigned long ticks = 0, bytes = 0;
  char state = 0, name[16]{};
};

enum class ProcessState
{
  Unused,
  Used,
  Sleeping,
  Runnable,
  Running,
  Stopped,
  Zombie,
  ThreadZombie
};

struct CpuState;
class Process;

// One owner for resources shared by all execution contexts of a process.
// Lifecycle code allocates this separately from each schedulable descriptor.
struct ProcessResources
{
  AddressSpace memory;
  Credentials credentials;
  DescriptorTable descriptors;
  WorkingDirectory directory;
  unsigned references = 1;
  int group_pid = 0;
  Process *leader = nullptr;
  Process *threads[32]{};
  unsigned slots = 1, live = 1, creating = 0;
  unsigned stopped = 0;
  bool stop_reported = false, continue_pending = false;
  int exec_owner = 0;
  bool threaded = false;
  unsigned long exit_request = 0;
  void *user_info = nullptr;
};

class Process final
{
public:

  ~Process() = default;
  Process(const Process &) = delete;
  Process &operator=(const Process &) = delete;

  int
  id() const {
    return resources_ ? resources_->group_pid : pid;
  }

  Process &
  group_leader() {
    return resources_ && resources_->leader ? *resources_->leader : *this;
  }

  bool
  threaded_group() const {
    return resources_ && __atomic_load_n(&resources_->threaded, __ATOMIC_ACQUIRE);
  }

  bool
  group_exit_requested() const {
    return resources_ && __atomic_load_n(&resources_->exit_request, __ATOMIC_ACQUIRE);
  }

  bool
  exec_stopped() const {
    auto owner =
        resources_ ? __atomic_load_n(&resources_->exec_owner, __ATOMIC_ACQUIRE) : 0;
    return owner && owner != pid;
  }

  bool prepare_exec();
  void finish_exec();
  void request_group_exit(int status, int signal);

  int
  thread_id() const {
    return pid;
  }

  const char *debug_name() const
  {
    return name;
  }

  void set_name(const char *label);

  void
  mark_filesystem_worker() {
    filesystem_worker_ = true;
  }

  bool
  filesystem_worker() const {
    return filesystem_worker_;
  }
  bool is_killed() const;
  void request_kill();
  int termination_status(int *signal = nullptr) const;
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

  unsigned long trap_frame_address() const;

  TrapFrame *registers()
  {
    return trapframe;
  }

  AddressSpace &memory()
  {
    return resources_->memory;
  }

  const AddressSpace &memory() const
  {
    return resources_->memory;
  }

  Credentials &credentials()
  {
    return resources_->credentials;
  }

  const Credentials &credentials() const
  {
    return resources_->credentials;
  }

  DescriptorTable &files()
  {
    return resources_->descriptors;
  }

  // Borrow for this system call; pins survive another thread closing the FD.
  xv::fs::File *pin_file(int descriptor);
  void clear_file_pins();

  WorkingDirectory &directory()
  {
    return resources_->directory;
  }

  bool create_image(AddressSpace &image, bool describe = true);
  void commit_image(AddressSpace &image, unsigned long entry, unsigned long stack,
                    unsigned long arguments, const char *path,
                    unsigned long environment = 0, ExecSnapshot *snapshot = nullptr);

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
  bool filesystem_worker_ = false; // inherited; must not recursively reclaim via FS RPC
  int pid = 0;
  void (*kernel_entry_)() = nullptr;
  void *storage_node_ = nullptr; // immutable registry owner, set before publication
  Process *parent = nullptr;     // protected by the manager's family lock
  // Intrusive family links are protected by the same family lock, which pins
  // linked descriptors against reap; process locks protect their wait events.
  Process *first_child_ = nullptr;
  Process *last_child_ = nullptr;
  Process *sibling_previous_ = nullptr;
  Process *sibling_next_ = nullptr;

  // Queue membership is protected by the family lock. Each kind has separate
  // links, while its pending state remains protected by the child process lock.
  struct ChildEventLinks
  {
    Process *previous = nullptr, *next = nullptr;
  };

  ChildEventLinks child_event_links_[3]{};
  unsigned child_event_mask_ = 0;
  Process *child_events_[3]{}, *child_event_tails_[3]{};
  Process *continue_notify_previous_ = nullptr, *continue_notify_next_ = nullptr;
  bool continue_notify_queued_ = false;
  ProcessMembership membership_{this}; // protected by ProcessGroups
  unsigned long kstack = 0;
  ProcessResources *resources_ = nullptr;
  struct FilePins;
  xv::fs::File *inline_file_pins_[8]{};
  unsigned inline_file_pin_count_ = 0;
  FilePins *extra_file_pins_ = nullptr;
  unsigned thread_slot_ = 0;
  TrapFrame *trapframe = nullptr;
  unsigned long thread_result_ = 0;
  int thread_joiner_ = 0, joining_tid_ = 0;
  int wait_claimant_ = 0;
  Process *word_wait_next_ = nullptr;
  unsigned long word_wait_address_ = 0;
  bool word_woken_ = false, word_waiting_ = false;
  bool thread_joined_ = false;
  Context context{};
  char name[16]{};
  struct CpuState *pincpu = nullptr;
  void *wait_key_ = nullptr;
  Process *wait_next_ = nullptr;
  Process *ready_next_ = nullptr;
  bool queued_ = false;
  unsigned long cpu_ticks_ = 0, cpu_started_ = 0;
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
