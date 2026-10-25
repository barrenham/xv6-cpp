module xv.sync;

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
  while (locked_)
  {
    if (pid_ == detail::current_pid())
    {
      detail::fail("sleeplock: recursive acquire");
    }
    // Register while holding the state lock to avoid a lost wakeup.
    detail::prepare_sleep(this);
    state_lock_.release();
    detail::sleep_process();
    state_lock_.acquire();
  }
  locked_ = true;
  pid_ = detail::current_pid();
  state_lock_.release();
}

bool SleepLock::acquire_interruptible()
{
  state_lock_.acquire();
  for (;;)
  {
    if (detail::interrupted())
    {
      state_lock_.release();
      return false;
    }
    if (!locked_)
    {
      break;
    }
    if (pid_ == detail::current_pid())
    {
      detail::fail("sleeplock: recursive acquire");
    }
    detail::prepare_interruptible_sleep(this);
    state_lock_.release();
    detail::sleep_process();
    state_lock_.acquire();
  }
  locked_ = true;
  pid_ = detail::current_pid();
  state_lock_.release();
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
