module;
#include "kernel/support/flow.h"

module xv.process;

import :scheduler_context;
import xv.arch;
import xv.print;

namespace xv::process
{

namespace flow = xv::flow;

struct SchedulerContext::DispatchFlow
{
  struct Context
  {
    Process &process;
    CpuState &cpu;
    ReadyQueue &ready;
  };

  static void enter(Context &c) noexcept
  {
    c.process.lock.acquire();
    if (c.process.state != ProcessState::Runnable)
    {
      xv::print::panic("scheduler: queued process is not runnable");
    }
    c.ready.start(c.process);
    c.process.state = ProcessState::Running;
    c.cpu.current = &c.process;
  }

  static void switch_context(Context &c) noexcept
  {
    // Keep both TLB fences adjacent to the switch across recyclable stack leaves.
    xv::arch::sfence_vma();
    swtch(&c.cpu.context, &c.process.context);
    xv::arch::sfence_vma();
  }

  static void leave(Context &c) noexcept
  {
    current_cpu()->intena = 0;
    c.cpu.current = nullptr;
    c.process.lock.release();
  }
  struct Enter;
  struct Switch;
  struct Leave;
  using Graph = flow::graph<
      Enter,
      flow::types<flow::node<Enter, flow::action<&enter>, flow::go<Switch>>,
                  flow::node<Switch, flow::action<&switch_context>, flow::go<Leave>>,
                  flow::node<Leave, flow::action<&leave>, flow::stop>>>;
};

struct SchedulerContext::SuspendFlow
{
  struct Context
  {
    Process &process;
    int intena = 0;
  };

  static void prepare(Context &c) noexcept
  {
    if (!c.process.lock.holding())
    {
      xv::print::panic("sched p->lock");
    }
    if (current_cpu()->noff != 1)
    {
      xv::print::panic("sched locks");
    }
    if (c.process.state == ProcessState::Running)
    {
      xv::print::panic("sched ProcessState::Running");
    }
    if (xv::arch::intr_get())
    {
      xv::print::panic("sched interruptible");
    }
    c.intena = current_cpu()->intena;
  }

  static void switch_context(Context &c) noexcept
  {
    swtch(&c.process.context, &current_cpu()->context);
    // The resumed process may run on another hart. Never cache CpuState across swtch.
    current_cpu()->intena = c.intena;
  }
  struct Prepare;
  struct Switch;
  using Graph = flow::graph<
      Prepare,
      flow::types<flow::node<Prepare, flow::action<&prepare>, flow::go<Switch>>,
                  flow::node<Switch, flow::action<&switch_context>, flow::stop>>>;
};

struct SchedulerContext::YieldFlow
{
  struct Context
  {
    Scheduler &scheduler;
    Process &process;
  };

  static void lock(Context &c) noexcept
  {
    c.process.lock.acquire();
  }

  static void publish(Context &c) noexcept
  {
    c.scheduler.make_runnable(c.process);
  }

  static void suspend(Context &c) noexcept
  {
    c.scheduler.suspend();
    c.process.lock.release();
  }
  struct Lock;
  struct Publish;
  struct Suspend;
  using Graph = flow::graph<
      Lock, flow::types<flow::node<Lock, flow::action<&lock>, flow::go<Publish>>,
                        flow::node<Publish, flow::action<&publish>, flow::go<Suspend>>,
                        flow::node<Suspend, flow::action<&suspend>, flow::stop>>>;
};

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

void SchedulerContext::dispatch(Process &process, CpuState &cpu, ReadyQueue &ready)
{
  DispatchFlow::Context context{process, cpu, ready};
  (void)DispatchFlow::Graph::run<0>(context);
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
  SuspendFlow::Context context{*current()};
  (void)SuspendFlow::Graph::run<0>(context);
}

void SchedulerContext::yield(Scheduler &scheduler)
{
  YieldFlow::Context context{scheduler, *current()};
  (void)YieldFlow::Graph::run<0>(context);
}

} // namespace xv::process
