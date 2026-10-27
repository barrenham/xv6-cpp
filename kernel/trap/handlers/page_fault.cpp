module xv.trap;
import :handlers;
import :platform;

namespace xv::trap
{
bool PageFaultHandler::accepts(const TrapEvent &event) const
{
  return !event.from_supervisor() && (event.cause.kind() == CauseKind::InstructionPageFault ||
                                      event.cause.kind() == CauseKind::LoadPageFault ||
                                      event.cause.kind() == CauseKind::StorePageFault);
}

TrapAction PageFaultHandler::handle(const TrapEvent &event)
{
  auto access = event.cause.kind() == CauseKind::LoadPageFault ? MemoryAccess::Read :
                event.cause.kind() == CauseKind::StorePageFault ? MemoryAccess::Write : MemoryAccess::Execute;
  return detail::resolve_page_fault(event.value, access) ? TrapAction::Resume
                                                       : TrapAction::Unhandled;
}
} // namespace xv::trap
