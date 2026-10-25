export module xv.process:process;

export import :registers;
export import :resources;
export import :address_space;
import xv.sync;

export namespace xv::process
{
enum class ProcessState
{
  Unused,
  Used,
  Sleeping,
  Runnable,
  Running,
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
                    unsigned long arguments, const char *path);

private:

  constexpr Process() = default;
  friend class ProcessManager;
  friend class ProcessStore;
  friend class ReadyQueue;
  friend class WaitChannels;
  friend class SchedulerContext;
  friend void prepare_sleep(void *channel);
  friend void sleep();
  friend void first_return();

  mutable xv::sync::SpinLock lock;
  ProcessState state = ProcessState::Unused;
  void *chan = nullptr;
  int killed = 0;
  int xstate = 0;
  int pid = 0;
  Process *parent = nullptr; // protected by the manager's family lock
  unsigned long kstack = 0;
  AddressSpace memory_;
  TrapFrame *trapframe = nullptr;
  void *usyscall = nullptr;
  Context context{};
  DescriptorTable descriptors_;
  WorkingDirectory directory_;
  char name[16]{};
  struct CpuState *pincpu = nullptr;
  void *wait_key_ = nullptr;
  Process *wait_next_ = nullptr;
  Process *ready_next_ = nullptr;
  bool queued_ = false;
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
