module xv.process;
import :wait_channels;
import :storage_platform;
import :poll;

namespace xv::process
{
unsigned WaitChannels::index(void *channel)
{
  auto value = reinterpret_cast<unsigned long>(channel);
  return ((value >> 3) ^ (value >> 11)) % BucketCount;
}

void WaitChannels::initialize()
{
  timer_lock_.init("poll timers");
  for (auto &bucket : buckets_)
  {
    bucket.lock.init("wait channel");
  }
}

void WaitChannels::unlink(Bucket &bucket, Process &process)
{
  Process **link = &bucket.head;
  while (*link && *link != &process)
  {
    link = &(*link)->wait_next_;
  }
  if (!*link)
  {
    detail::storage_fail("wait channel: missing registration");
  }
  *link = process.wait_next_;
  process.wait_next_ = nullptr;
  __atomic_store_n(&process.wait_key_, static_cast<void *>(nullptr), __ATOMIC_RELEASE);
  process.chan = nullptr;
  process.interruptible_ = false;
}

void WaitChannels::cancel(Process &process)
{
  for (;;)
  {
    auto *key = __atomic_load_n(&process.wait_key_, __ATOMIC_ACQUIRE);
    if (!key)
    {
      return;
    }
    auto &bucket = buckets_[index(key)];
    xv::sync::ScopedLock bucket_guard(bucket.lock);
    xv::sync::ScopedLock process_guard(process.lock);
    if (__atomic_load_n(&process.wait_key_, __ATOMIC_RELAXED) == key)
    {
      unlink(bucket, process);
      return;
    }
  }
}

void WaitChannels::prepare(Process &process, void *channel, bool interruptible)
{
  if (!channel)
  {
    detail::storage_fail("prepare_sleep: zero channel");
  }
  // A kill can notify the old registration without unlinking it while holding
  // the process lock. Retire that membership before registering the next wait.
  cancel(process);
  auto &bucket = buckets_[index(channel)];
  xv::sync::ScopedLock bucket_guard(bucket.lock);
  xv::sync::ScopedLock process_guard(process.lock);
  if (interruptible &&
      (process.killed || process.pending_signals_.interrupting()))
  {
    if (process.pending_signals_.caught_signal()) process.syscall_interrupted_ = true;
    process.chan = nullptr;
    process.interruptible_ = false;
    return;
  }
  process.interruptible_ = interruptible;
  process.chan = channel;
  process.wait_next_ = bucket.head;
  bucket.head = &process;
  __atomic_store_n(&process.wait_key_, channel, __ATOMIC_RELEASE);
}

void WaitChannels::wake(void *channel)
{
  if (!channel)
  {
    return;
  }
  auto &bucket = buckets_[index(channel)];
  xv::sync::ScopedLock bucket_guard(bucket.lock);
  Process *process = bucket.head;
  while (process)
  {
    Process *next = process->wait_next_;
    if (__atomic_load_n(&process->wait_key_, __ATOMIC_RELAXED) == channel)
    {
      xv::sync::ScopedLock process_guard(process->lock);
      unlink(bucket, *process);
      if (process->state == ProcessState::Sleeping)
      {
        scheduler_.make_runnable(*process);
      }
    }
    process = next;
  }
  for (auto *entry = bucket.pollers; entry; entry = entry->next)
    if (entry->channel == channel) entry->owner->notify();
}
} // namespace xv::process
