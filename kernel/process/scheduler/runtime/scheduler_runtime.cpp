module;
#include "kernel/support/flow.h"

module xv.process;

import :scheduler_runtime;
import :scheduler_context;

namespace xv::process
{

namespace flow = xv::flow;

struct SchedulerRuntime::RunFlow
{
  struct Context
  {
    SchedulerRuntime &owner;
    CpuState &cpu;
    Process *process = nullptr;
  };

  static void select(Context &c) noexcept
  {
    SchedulerContext::begin_round();
    c.process = c.owner.ready_->take(c.cpu);
  }

  static bool selected(Context &c) noexcept
  {
    return c.process != nullptr;
  }

  static void dispatch(Context &c) noexcept
  {
    SchedulerContext::dispatch(*c.process, c.cpu, *c.owner.ready_);
  }

  static void notify(Context &) noexcept
  {
    // Dispatch has released the outgoing process lock. Its pointer is not reused.
    service_signal_notifications();
  }

  static void idle(Context &) noexcept
  {
    SchedulerContext::idle();
  }
  struct Select;
  struct Dispatch;
  struct Notify;
  struct Idle;
  using Graph = flow::graph<
      Select,
      flow::types<flow::node<Select, flow::action<&select>,
                             flow::branch<flow::condition<&selected>,
                                          flow::go<Dispatch>, flow::go<Idle>>>,
                  flow::node<Dispatch, flow::action<&dispatch>, flow::go<Notify>>,
                  flow::node<Notify, flow::action<&notify>, flow::go<Select>>,
                  flow::node<Idle, flow::action<&idle>, flow::go<Select>>>>;
};

void SchedulerRuntime::run()
{
  auto &cpu = SchedulerContext::prepare_cpu();
  RunFlow::Context context{*this, cpu};
  (void)RunFlow::Graph::run<0>(context);
  __builtin_unreachable();
}

void SchedulerRuntime::suspend()
{
  ready_->stop(*current());
  SchedulerContext::suspend();
}

void SchedulerRuntime::yield()
{
  SchedulerContext::yield(*this);
}

void SchedulerRuntime::make_runnable(Process &process)
{
  SchedulerContext::make_runnable(process, *ready_);
}
bool SchedulerRuntime::timer_tick()
{
  auto *process = current();
  return process && ready_->tick(*process);
}

} // namespace xv::process
