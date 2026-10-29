module;
#include "kernel/process/scheduler/policies/scheduling_policy.h"
module xv.process:ready_queue;

import :process;

namespace xv::process
{

class ReadyQueue
{
public:

  virtual const char *name() const = 0;
  virtual void initialize() = 0;
  // enqueue requires the process lock; take must never wait for it while
  // holding its own queue lock. Runnable ownership protects dequeued pointers.
  virtual void enqueue(Process &process) = 0;
  virtual Process *take(CpuState &cpu) = 0;
  virtual const char *scheduler_name() const { return name(); }
  // Start/stop run with the process lock; tick runs in its local interrupt.
  virtual void start(Process &) {}
  virtual void stop(Process &) {}
  virtual bool tick(Process &) { return true; }

protected:

  constexpr ReadyQueue() = default;
  ~ReadyQueue() = default;
  static bool process_locked(const Process &process);
  static bool eligible(const Process &process, const CpuState &cpu);
  static Process *&next(Process &process);
  static bool &queued(Process &process);
  static xv::scheduling::Entity &entity(Process &process);
};
} // namespace xv::process
