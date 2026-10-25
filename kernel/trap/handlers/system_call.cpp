module xv.trap;
import :handlers;
import :platform;

namespace xv::trap
{
bool SystemCallHandler::accepts(const TrapEvent &event) const
{
  return !event.from_supervisor() && event.cause.kind() == CauseKind::UserSystemCall;
}

TrapAction SystemCallHandler::handle(const TrapEvent &)
{
  if (detail::killed())
  {
    return TrapAction::Terminate;
  }
  // Save the next user PC before enabling interrupts. exec may replace it later.
  detail::advance_user_pc(4);
  detail::enable_interrupts();
  detail::dispatch_system_call();
  return TrapAction::Resume;
}
} // namespace xv::trap
