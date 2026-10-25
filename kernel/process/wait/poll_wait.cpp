module;
#include "kernel/support/placement_new.hpp"

module xv.process;
import :poll;
import :wait_channels;
import :poll_entry;
import :storage_platform;

namespace xv::process
{
struct PollWait::Page
{
  struct Table final : public xv::sync::PollTable
  {
    explicit Table(PollWait &wait) : owner(wait) {}
    void watch(void *channel) override { owner.watch(channel); }
    PollWait &owner;
  } table;
  explicit Page(PollWait &wait) : table(wait) {}
  Page *next = nullptr;
  unsigned used = 0;
  PollEntry entries[(4096 - 64) / sizeof(PollEntry)]{};
};

PollWait::Page *PollWait::allocate_page()
{
  static_assert(sizeof(Page) <= 4096);
  auto *storage = detail::allocate_process_page();
  if (!storage) { failed_ = true; return nullptr; }
  auto *page = new (storage) Page(*this);
  page->next = pages_;
  pages_ = page;
  return page;
}

xv::sync::PollTable *PollWait::table()
{
  if (!pages_ && !allocate_page()) return nullptr;
  return &pages_->table;
}

void PollWait::watch(void *channel)
{
  if (!channel || failed_) return;
  for (auto *page = pages_; page; page = page->next)
    for (unsigned i = 0; i < page->used; ++i)
      if (page->entries[i].channel == channel) return;
  if (!pages_ || pages_->used == sizeof(pages_->entries) / sizeof(PollEntry))
    if (!allocate_page()) return;
  auto &entry = pages_->entries[pages_->used++];
  entry.owner = this;
  channels_.subscribe(entry, channel);
}

PollWait::~PollWait()
{
  channels_.remove_timer(*this);
  while (pages_)
  {
    auto *page = pages_;
    for (unsigned i = 0; i < page->used; ++i) channels_.unsubscribe(page->entries[i]);
    pages_ = page->next;
    page->~Page();
    detail::free_process_page(page);
  }
}

bool PollWait::triggered() const
{
  xv::sync::ScopedLock guard(process_.lock);
  return triggered_;
}

void PollWait::notify()
{
  // Caller holds the subscription bucket or timer lock, pinning this context.
  xv::sync::ScopedLock guard(process_.lock);
  triggered_ = true;
  if (process_.chan == this)
  {
    process_.chan = nullptr;
    process_.interruptible_ = false;
    if (process_.state == ProcessState::Sleeping)
      channels_.scheduler_.make_runnable(process_);
  }
}

void PollWait::set_deadline(unsigned long milliseconds)
{
  channels_.arm_timer(*this, milliseconds);
}

void PollWait::sleep()
{
  // Prior syscalls may have left an unconsumed single-channel registration.
  channels_.cancel(process_);
  process_.lock.acquire();
  // The process lock is already held; is_killed() would acquire it again.
  bool interrupted = process_.killed || process_.pending_signals_.terminating_signal() ||
                     process_.pending_signals_.stopping_signal();
  if (!triggered_ && !interrupted)
  {
    process_.chan = this;
    process_.interruptible_ = true;
    process_.state = ProcessState::Sleeping;
    channels_.scheduler_.suspend();
    process_.chan = nullptr;
    process_.interruptible_ = false;
  }
  triggered_ = false;
  process_.lock.release();
}

void WaitChannels::subscribe(PollEntry &entry, void *channel)
{
  auto &bucket = buckets_[index(channel)];
  xv::sync::ScopedLock guard(bucket.lock);
  entry.channel = channel;
  entry.next = bucket.pollers;
  bucket.pollers = &entry;
}

void WaitChannels::unsubscribe(PollEntry &entry)
{
  auto &bucket = buckets_[index(entry.channel)];
  xv::sync::ScopedLock guard(bucket.lock);
  auto **link = &bucket.pollers;
  while (*link && *link != &entry) link = &(*link)->next;
  if (!*link) detail::storage_fail("poll: missing subscription");
  *link = entry.next;
}

void WaitChannels::arm_timer(PollWait &wait, unsigned long deadline)
{
  xv::sync::ScopedLock guard(timer_lock_);
  if (wait.timer_linked_) detail::storage_fail("poll: duplicate timer");
  wait.deadline_ = deadline;
  wait.timer_next_ = timers_;
  wait.timer_linked_ = true;
  timers_ = &wait;
}

void WaitChannels::remove_timer(PollWait &wait)
{
  xv::sync::ScopedLock guard(timer_lock_);
  if (!wait.timer_linked_) return;
  auto **link = &timers_;
  while (*link && *link != &wait) link = &(*link)->timer_next_;
  if (!*link) detail::storage_fail("poll: missing timer");
  *link = wait.timer_next_;
  wait.timer_linked_ = false;
}

void WaitChannels::expire_timers(unsigned long milliseconds)
{
  xv::sync::ScopedLock guard(timer_lock_);
  auto **link = &timers_;
  while (*link)
  {
    auto *wait = *link;
    if (milliseconds >= wait->deadline_)
    {
      *link = wait->timer_next_;
      wait->timer_linked_ = false;
      wait->notify();
    }
    else link = &wait->timer_next_;
  }
}
}
