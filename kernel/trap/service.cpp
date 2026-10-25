module xv.trap;
import :handlers;

namespace xv::trap
{
namespace
{
constinit TickClock system_clock;
constinit SystemCallHandler system_calls;
constinit ExternalInterruptHandler external_interrupts;
constinit TimerInterruptHandler timer_interrupts(system_clock);
constinit PageFaultHandler page_faults;
constinit TrapHandler *handlers[] = {&system_calls, &external_interrupts,
                                     &timer_interrupts, &page_faults};
constinit TrapDispatcher system_dispatcher(handlers,
                                           sizeof(handlers) / sizeof(handlers[0]));
constinit TrapController system_controller(system_dispatcher);
} // namespace

TickClock &clock()
{
  return system_clock;
}

TrapController &controller()
{
  return system_controller;
}

void initialize()
{
  system_clock.initialize();
}

void initialize_hart()
{
  system_controller.initialize_hart();
}

void prepare_return()
{
  system_controller.prepare_return();
}

[[noreturn]] void return_to_user()
{
  system_controller.return_to_user();
}
} // namespace xv::trap
