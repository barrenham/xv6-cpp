module xv.trap;
import :handlers;
import :platform;

namespace xv::trap
{
bool TimerInterruptHandler::accepts(const TrapEvent &event) const
{
  return event.cause.kind() == CauseKind::TimerInterrupt;
}

TrapAction TimerInterruptHandler::handle(const TrapEvent &event)
{
  const bool periodic = clock_.interrupt();
  if (periodic)
  {
    detail::account_timer(!event.from_supervisor());
  }
  else
  {
    detail::account_deadline_interrupt();
  }
  detail::service_timer_devices(periodic);
  // Deadline wakeups get a dispatch opportunity without consuming a policy tick.
  return (!periodic || detail::scheduler_timer_tick()) ? TrapAction::Yield
                                                       : TrapAction::Resume;
}
} // namespace xv::trap
