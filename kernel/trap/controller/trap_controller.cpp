module xv.trap;
import :entry_flow;
import :platform;

namespace xv::trap
{
void TrapController::initialize_hart()
{
  detail::install_kernel_vector();
}

Word TrapController::handle_user()
{
  EntryFlow::Context ctx{*this, detail::snapshot()};
  (void)EntryFlow::UserGraph::run<0>(ctx);
  return ctx.result;
}

void TrapController::handle_kernel()
{
  EntryFlow::Context ctx{*this, detail::snapshot()};
  (void)EntryFlow::KernelGraph::run<0>(ctx);
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
