module xv.trap;
import :handlers;
import :platform;

namespace xv::trap
{
bool ExternalInterruptHandler::accepts(const TrapEvent &event) const
{
  return event.cause.kind() == CauseKind::ExternalInterrupt;
}

TrapAction ExternalInterruptHandler::handle(const TrapEvent &)
{
  // Device identification, claim and completion belong to the drivers router.
  detail::account_external_interrupt();
  detail::dispatch_external_interrupt();
  return TrapAction::Resume;
}
} // namespace xv::trap
