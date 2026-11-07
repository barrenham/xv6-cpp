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
  single_waiter_ = false;
  pid_ = 0;
}

void SleepLock::init_single_waiter(const char *label)
{
  init(label);
  single_waiter_ = true;
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
  if (single_waiter_) detail::wake_one_process(this);
  else detail::wake_processes(this);
}

bool SleepLock::holding() const
{
  ScopedLock guard(state_lock_);
  return locked_ && pid_ == detail::current_pid();
}

void
SleepRwLock::init(const char *label) {
  state_lock_.init("rw lock state");
  name_ = label;
  readers_ = waiting_writers_ = 0;
  writer_ = 0;
  writing_ = false;
}

void
SleepRwLock::wait() {
  // Register while holding the state lock, before allowing a release/wakeup.
  detail::prepare_sleep(this);
  state_lock_.release();
  detail::sleep_process();
  state_lock_.acquire();
}

void
SleepRwLock::acquire() {
  ScopedLock guard(state_lock_);
  if (writing_ && writer_ == detail::current_pid())
  {
    detail::fail("rwlock: recursive acquire");
  }
  ++waiting_writers_;
  while (writing_ || readers_)
  {
    wait();
  }
  --waiting_writers_;
  writer_ = detail::current_pid();
  writing_ = true;
}

void
SleepRwLock::release() {
  ScopedLock guard(state_lock_);
  if (!writing_ || writer_ != detail::current_pid())
  {
    detail::fail("rwlock: release by non-owner");
  }
  writer_ = 0;
  writing_ = false;
  detail::wake_processes(this);
}

bool
SleepRwLock::holding() const {
  ScopedLock guard(state_lock_);
  return writing_ && writer_ == detail::current_pid();
}

void
SleepRwLock::acquire_shared() {
  ScopedLock guard(state_lock_);
  if (writing_ && writer_ == detail::current_pid())
  {
    detail::fail("rwlock: recursive shared acquire");
  }
  while (writing_ || waiting_writers_)
  {
    wait();
  }
  ++readers_;
}

bool
SleepRwLock::try_acquire_shared() {
  ScopedLock guard(state_lock_);
  if (writing_ || waiting_writers_)
  {
    return false;
  }
  ++readers_;
  return true;
}

void
SleepRwLock::release_shared() {
  ScopedLock guard(state_lock_);
  if (!readers_)
  {
    detail::fail("rwlock: shared release without reader");
  }
  if (!--readers_)
  {
    detail::wake_processes(this);
  }
}
} // namespace xv::sync
