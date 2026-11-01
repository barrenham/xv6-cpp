module xv.trap;
import :clock_flow;
import :platform;

namespace xv::trap
{
void TickClock::initialize()
{
  lock_.init("ticks");
}

bool TickClock::interrupt()
{
  const auto now = detail::time();
  const auto hart = detail::hart_id();
  bool periodic = false;
  {
    xv::sync::ScopedLock guard(lock_);
    auto &next = next_tick_[hart];
    if (!next)
    {
      next = TimerInterval;
    }
    // Deadline interrupts do not advance uptime or the scheduler's quantum.
    if (now >= next)
    {
      const auto elapsed = (now - next) / TimerInterval + 1;
      next += elapsed * TimerInterval;
      periodic = true;
      if (hart == 0)
      {
        ticks_ += elapsed;
        detail::wakeup(&ticks_);
      }
    }
    detail::arm_timer(next);
  }
  return periodic;
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
