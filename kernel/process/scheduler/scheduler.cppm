module;
#include "kernel/process/scheduler/policies/scheduling_policy.h"
export module xv.process:scheduler;

import :process;

export namespace xv::process
{
using SchedulerKind = xv::scheduling::Kind;
class Scheduler
{
public:

  Scheduler(const Scheduler &) = delete;
  Scheduler &operator=(const Scheduler &) = delete;
  virtual const char *name() const = 0;
  [[noreturn]] virtual void run() = 0;
  // Requires current process lock held, interrupts disabled, no other locks.
  virtual void suspend() = 0;
  virtual void yield() = 0;
  // Requires the process lock held; publish exactly one runnable queue entry.
  virtual void make_runnable(Process &process) = 0;
  // Local timer interrupt, interrupts off; false suppresses an unnecessary
  // context switch. Existing scheduler implementations retain tick preemption.
  virtual bool timer_tick() { return true; }

protected:

  constexpr Scheduler() = default;
  ~Scheduler() = default;
};

Scheduler &scheduler();
SchedulerKind configured_scheduler_kind();
} // namespace xv::process
