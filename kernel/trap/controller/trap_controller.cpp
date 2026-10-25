module xv.trap;
import :platform;

namespace xv::trap
{
void TrapController::initialize_hart()
{
  detail::install_kernel_vector();
}

Word TrapController::handle_user()
{
  const auto event = detail::snapshot();
  if (event.from_supervisor())
  {
    detail::fatal("usertrap: not from user mode", event);
  }
  detail::install_kernel_vector();
  if (!detail::has_process())
  {
    detail::fatal("usertrap: no current process", event);
  }
  detail::save_user_pc(event.pc);

  const auto action = dispatcher_.dispatch(event);
  if (action == TrapAction::Unhandled)
  {
    detail::report_user_fault(event);
    detail::request_kill();
    detail::exit_current(-1);
  }
  detail::deliver_signals();
  if (action == TrapAction::Terminate || detail::killed())
  {
    detail::exit_current(-1);
  }
  if (action == TrapAction::Yield)
  {
    detail::yield_current();
  }
  prepare_return();
  return detail::user_page_table_token();
}

void TrapController::handle_kernel()
{
  const auto event = detail::snapshot();
  if (!event.from_supervisor())
  {
    detail::fatal("kerneltrap: not from supervisor mode", event);
  }
  if (event.interrupts_enabled())
  {
    detail::fatal("kerneltrap: interrupts enabled", event);
  }
  const auto action = dispatcher_.dispatch(event);
  if (action != TrapAction::Resume && action != TrapAction::Yield)
  {
    detail::fatal("kerneltrap", event);
  }
  if (action == TrapAction::Yield && detail::has_process())
  {
    detail::yield_current();
  }
  // A yield can run other traps on this stack/hart before returning.
  detail::restore_kernel(event);
}

void TrapController::prepare_return()
{
  detail::deliver_signals();
  detail::prepare_user_return();
}

[[noreturn]] void TrapController::return_to_user()
{
  prepare_return();
  detail::enter_user(detail::user_page_table_token());
}
} // namespace xv::trap
