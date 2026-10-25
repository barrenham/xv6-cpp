module xv.trap;
import :handlers;
import :platform;

namespace xv::trap
{
bool TimerInterruptHandler::accepts(const TrapEvent &event) const
{
  return event.cause.kind() == CauseKind::TimerInterrupt;
}

TrapAction TimerInterruptHandler::handle(const TrapEvent &)
{
  clock_.interrupt();
  detail::service_timer_devices();
  return TrapAction::Yield;
}
} // namespace xv::trap
