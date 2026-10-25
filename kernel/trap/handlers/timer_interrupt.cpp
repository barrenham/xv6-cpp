module xv.trap;
import :handlers;

namespace xv::trap
{
bool TimerInterruptHandler::accepts(const TrapEvent &event) const
{
  return event.cause.kind() == CauseKind::TimerInterrupt;
}

TrapAction TimerInterruptHandler::handle(const TrapEvent &)
{
  clock_.interrupt();
  return TrapAction::Yield;
}
} // namespace xv::trap
