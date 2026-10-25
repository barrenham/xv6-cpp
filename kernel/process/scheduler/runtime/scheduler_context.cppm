module xv.process:scheduler_context;

import :process;
import :scheduler;
import :ready_queue;

namespace xv::process
{
// Execution/lock handoff mechanics shared by scheduling policies.
class SchedulerContext final
{
public:

  static CpuState &prepare_cpu();
  static void begin_round();
  static void idle();
  static void make_runnable(Process &process, ReadyQueue &ready);
  static void dispatch(Process &process, CpuState &cpu);
  static void suspend();
  static void yield(Scheduler &scheduler);
};
} // namespace xv::process
