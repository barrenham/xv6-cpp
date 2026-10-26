module xv.process;

import :round_robin;
import :scheduler_context;

namespace xv::process
{
void RoundRobinScheduler::run()
{
  auto &cpu = SchedulerContext::prepare_cpu();
  for (;;)
  {
    SchedulerContext::begin_round();
    if (auto *process = ready_.take(cpu))
    {
      SchedulerContext::dispatch(*process, cpu);
      // exit/stop publish SIGCHLD while holding the outgoing process lock.
      // Service only after dispatch releases it; no process pointer is reused.
      service_signal_notifications();
    }
    else
    {
      SchedulerContext::idle();
    }
  }
}

void RoundRobinScheduler::suspend()
{
  SchedulerContext::suspend();
}

void RoundRobinScheduler::yield()
{
  SchedulerContext::yield(*this);
}

void RoundRobinScheduler::make_runnable(Process &process)
{
  SchedulerContext::make_runnable(process, ready_);
}
} // namespace xv::process
