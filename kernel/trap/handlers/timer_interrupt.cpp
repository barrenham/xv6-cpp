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
  detail::account_timer(!event.from_supervisor());
  clock_.interrupt();
  detail::service_timer_devices();
  return detail::scheduler_timer_tick() ? TrapAction::Yield : TrapAction::Resume;
}
} // namespace xv::trap
