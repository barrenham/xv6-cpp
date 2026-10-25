module xv.trap;
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
  if (count < 0)
  {
    count = 0;
  }
  const auto interval = static_cast<unsigned>(count);
  lock_.acquire();
  const unsigned start = ticks_;
  while (ticks_ - start < interval)
  {
    if (detail::interrupted())
    {
      lock_.release();
      return -1;
    }
    detail::prepare_interruptible_sleep(&ticks_);
    lock_.release();
    detail::sleep();
    lock_.acquire();
  }
  lock_.release();
  return 0;
}
} // namespace xv::trap
