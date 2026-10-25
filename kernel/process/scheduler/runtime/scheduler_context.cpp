module xv.process;

import :scheduler_context;
import xv.arch;
import xv.print;

namespace xv::process
{
CpuState &SchedulerContext::prepare_cpu()
{
  auto *cpu = current_cpu();
  cpu->current = nullptr;
  return *cpu;
}

void SchedulerContext::begin_round()
{
  // Permit pending interrupts, then close the check-to-idle interrupt window.
  xv::arch::intr_on();
  xv::arch::intr_off();
}

void SchedulerContext::idle()
{
  // A periodic timer bounds idle latency if another hart publishes work here.
  xv::arch::intr_on();
#ifndef LAB_FS
  asm volatile("wfi");
#endif
}

void SchedulerContext::dispatch(Process &process, CpuState &cpu)
{
  process.lock.acquire();
  if (process.state != ProcessState::Runnable)
  {
    xv::print::panic("scheduler: queued process is not runnable");
  }
  process.state = ProcessState::Running;
  cpu.current = &process;
  // Stack leaves can be recycled while other harts retain stale TLB entries.
  // Fence before entering and after leaving every process's kernel stack.
  xv::arch::sfence_vma();
  swtch(&cpu.context, &process.context);
  xv::arch::sfence_vma();
  current_cpu()->intena = 0;
  cpu.current = nullptr;
  process.lock.release();
}

void SchedulerContext::make_runnable(Process &process, ReadyQueue &ready)
{
  if (!process.lock.holding() || process.state == ProcessState::Unused ||
      process.state == ProcessState::Zombie || process.state == ProcessState::Runnable)
  {
    xv::print::panic("scheduler: invalid runnable transition");
  }
  process.state = ProcessState::Runnable;
  ready.enqueue(process);
}

void SchedulerContext::suspend()
{
  int intena;
  Process *p = current();

  if (!p->lock.holding())
  {
    xv::print::panic("sched p->lock");
  }
  if (current_cpu()->noff != 1)
  {
    xv::print::panic("sched locks");
  }
  if (p->state == ProcessState::Running)
  {
    xv::print::panic("sched ProcessState::Running");
  }
  if (xv::arch::intr_get())
  {
    xv::print::panic("sched interruptible");
  }

  intena = current_cpu()->intena;
  swtch(&p->context, &current_cpu()->context);
  current_cpu()->intena = intena;
}

void SchedulerContext::yield(Scheduler &scheduler)
{
  Process *p = current();
  p->lock.acquire();
  scheduler.make_runnable(*p);
  scheduler.suspend();
  p->lock.release();
}

} // namespace xv::process
