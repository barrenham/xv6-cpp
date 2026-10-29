module xv.sync;
import :sleep_flow;

import :platform;

namespace xv::sync
{
void SleepLock::init(const char *label)
{
  state_lock_.init("sleep lock state");
  name_ = label;
  locked_ = false;
  pid_ = 0;
}

void SleepLock::acquire()
{
  state_lock_.acquire();
  AcquireFlow::Context ctx{*this, false};
  (void)AcquireFlow::Graph::run<0>(ctx);
}

bool SleepLock::acquire_interruptible()
{
  state_lock_.acquire();
  AcquireFlow::Context ctx{*this, true};
  (void)AcquireFlow::Graph::run<0>(ctx);
  return ctx.acquired;
}

bool SleepLock::available(PollTable *table) const
{
  ScopedLock guard(state_lock_);
  if (table)
  {
    table->watch(const_cast<SleepLock *>(this));
  }
  return !locked_;
}

bool SleepLock::try_acquire()
{
  ScopedLock guard(state_lock_);
  if (locked_)
  {
    return false;
  }
  locked_ = true;
  pid_ = detail::current_pid();
  return true;
}

void SleepLock::release()
{
  ScopedLock guard(state_lock_);
  if (!locked_ || pid_ != detail::current_pid())
  {
    detail::fail("sleeplock: release by non-owner");
  }
  locked_ = false;
  pid_ = 0;
  detail::wake_processes(this);
}

bool SleepLock::holding() const
{
  ScopedLock guard(state_lock_);
  return locked_ && pid_ == detail::current_pid();
}
} // namespace xv::sync
