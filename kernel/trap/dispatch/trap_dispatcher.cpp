module xv.trap;
import :platform;

namespace xv::trap
{
TrapAction TrapDispatcher::dispatch(const TrapEvent &event)
{
  if (count_ && !handlers_)
  {
    detail::fatal("trap: missing handler table", event);
  }
  for (unsigned i = 0; i < count_; ++i)
  {
    if (!handlers_[i])
    {
      detail::fatal("trap: null handler", event);
    }
    if (handlers_[i]->accepts(event))
    {
      return handlers_[i]->handle(event);
    }
  }
  return TrapAction::Unhandled;
}
} // namespace xv::trap
