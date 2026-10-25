module xv.trap;
import :handlers;
import :platform;

namespace xv::trap
{
bool PageFaultHandler::accepts(const TrapEvent &event) const
{
  return !event.from_supervisor() && (event.cause.kind() == CauseKind::LoadPageFault ||
                                      event.cause.kind() == CauseKind::StorePageFault);
}

TrapAction PageFaultHandler::handle(const TrapEvent &event)
{
  const bool read = event.cause.kind() == CauseKind::LoadPageFault;
  return detail::resolve_page_fault(event.value, read) ? TrapAction::Resume
                                                       : TrapAction::Unhandled;
}
} // namespace xv::trap
