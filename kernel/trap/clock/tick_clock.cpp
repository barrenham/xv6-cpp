module xv.trap;
import :clock_flow;
import :platform;

namespace xv::trap
{
void TickClock::initialize()
{
  lock_.init("ticks");
}

void TickClock::interrupt()
{
  if (detail::hart_id() == 0)
  {
    xv::sync::ScopedLock guard(lock_);
    ++ticks_;
    detail::wakeup(&ticks_);
  }
  detail::arm_timer(detail::time() + TimerInterval);
}

unsigned TickClock::uptime() const
{
  xv::sync::ScopedLock guard(lock_);
  return ticks_;
}

int TickClock::pause(int count)
{
  const auto interval = static_cast<unsigned>(count < 0 ? 0 : count);
  lock_.acquire();
  PauseFlow::Context ctx{*this, interval, ticks_};
  (void)PauseFlow::Graph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::trap
