module xv.trap;
import :dispatch_flow;
import :platform;

namespace xv::trap
{
TrapAction TrapDispatcher::dispatch(const TrapEvent &event)
{
  DispatchFlow::Context ctx{*this, event};
  (void)DispatchFlow::Graph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::trap
