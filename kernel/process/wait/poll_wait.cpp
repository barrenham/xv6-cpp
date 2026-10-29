module;
#include "kernel/support/placement_new.hpp"
#include "kernel/support/flow.h"

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
    explicit Table(PollWait &wait) : owner(wait)
    {
    }

    void watch(void *channel) override
    {
      owner.watch(channel);
    }

    PollWait &owner;
  } table;

  explicit Page(PollWait &wait) : table(wait)
  {
  }

  Page *next = nullptr;
  unsigned used = 0;
  PollEntry entries[(4096 - 64) / sizeof(PollEntry)]{};
};

namespace flow = xv::flow;

struct PollWait::WatchFlow
{
  struct Context
  {
    PollWait &owner;
    void *channel;
  };

  static bool duplicate(Context &c) noexcept
  {
    for (auto *page = c.owner.pages_; page; page = page->next)
    {
      for (unsigned i = 0; i < page->used; ++i)
      {
        if (page->entries[i].channel == c.channel)
        {
          return true;
        }
      }
    }
    return false;
  }

  static bool needs_page(Context &c) noexcept
  {
    return !c.owner.pages_ ||
           c.owner.pages_->used == sizeof(c.owner.pages_->entries) / sizeof(PollEntry);
  }

  static bool allocate(Context &c) noexcept
  {
    return c.owner.allocate_page() != nullptr;
  }

  static void subscribe(Context &c) noexcept
  {
    auto &entry = c.owner.pages_->entries[c.owner.pages_->used++];
    entry.owner = &c.owner;
    c.owner.channels_.subscribe(entry, c.channel);
  }
  struct Duplicate;
  struct Capacity;
  struct Allocate;
  struct Subscribe;
  using Graph = flow::graph<
      Duplicate,
      flow::types<flow::node<Duplicate, flow::noop,
                             flow::branch<flow::condition<&duplicate>, flow::stop,
                                          flow::go<Capacity>>>,
                  flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&needs_page>,
                                          flow::go<Allocate>, flow::go<Subscribe>>>,
                  flow::node<Allocate, flow::noop,
                             flow::branch<flow::condition<&allocate>,
                                          flow::go<Subscribe>, flow::stop>>,
                  flow::node<Subscribe, flow::action<&subscribe>, flow::stop>>>;
};

struct PollWait::ReleaseFlow
{
  struct Context
  {
    PollWait &owner;
    Page *page = nullptr;
  };

  static void timer(Context &c) noexcept
  {
    c.owner.channels_.remove_timer(c.owner);
  }

  static bool pages(Context &c) noexcept
  {
    return c.owner.pages_ != nullptr;
  }

  static void unsubscribe(Context &c) noexcept
  {
    c.page = c.owner.pages_;
    for (unsigned i = 0; i < c.page->used; ++i)
    {
      c.owner.channels_.unsubscribe(c.page->entries[i]);
    }
    c.owner.pages_ = c.page->next;
  }

  static void free_page(Context &c) noexcept
  {
    c.page->~Page();
    detail::free_process_page(c.page);
  }
  struct Timer;
  struct Unsubscribe;
  struct Free;
  using Graph = flow::graph<
      Timer,
      flow::types<flow::node<Timer, flow::action<&timer>,
                             flow::branch<flow::condition<&pages>,
                                          flow::go<Unsubscribe>, flow::stop>>,
                  flow::node<Unsubscribe, flow::action<&unsubscribe>, flow::go<Free>>,
                  flow::node<Free, flow::action<&free_page>,
                             flow::branch<flow::condition<&pages>,
                                          flow::go<Unsubscribe>, flow::stop>>>>;
};

struct PollWait::SleepFlow
{
  struct Context
  {
    PollWait &owner;
    bool interrupted = false;
  };

  static void prepare(Context &c) noexcept
  {
    c.owner.channels_.cancel(c.owner.process_);
    c.owner.process_.lock.acquire();
    c.interrupted =
        c.owner.process_.killed || c.owner.process_.pending_signals_.interrupting();
    if (c.owner.process_.pending_signals_.caught_signal())
    {
      c.owner.process_.syscall_interrupted_ = true;
    }
  }

  static bool ready(Context &c) noexcept
  {
    return !c.owner.triggered_ && !c.interrupted;
  }

  static void suspend(Context &c) noexcept
  {
    c.owner.process_.chan = &c.owner;
    c.owner.process_.interruptible_ = true;
    c.owner.process_.state = ProcessState::Sleeping;
    c.owner.channels_.scheduler_.suspend();
    c.owner.process_.chan = nullptr;
    c.owner.process_.interruptible_ = false;
  }

  static void finish(Context &c) noexcept
  {
    c.owner.triggered_ = false;
    c.owner.process_.lock.release();
  }
  struct Prepare;
  struct Suspend;
  struct Finish;
  using Graph = flow::graph<
      Prepare,
      flow::types<flow::node<Prepare, flow::action<&prepare>,
                             flow::branch<flow::condition<&ready>, flow::go<Suspend>,
                                          flow::go<Finish>>>,
                  flow::node<Suspend, flow::action<&suspend>, flow::go<Finish>>,
                  flow::node<Finish, flow::action<&finish>, flow::stop>>>;
};

struct WaitChannels::ExpireFlow
{
  struct Context
  {
    PollWait **link;
    unsigned long milliseconds;
    PollWait *wait = nullptr;
  };

  static void scan(Context &c) noexcept
  {
    while (*c.link)
    {
      c.wait = *c.link;
      if (c.milliseconds >= c.wait->deadline_)
      {
        return;
      }
      c.link = &c.wait->timer_next_;
    }
    c.wait = nullptr;
  }

  static bool expired(Context &c) noexcept
  {
    return c.wait != nullptr;
  }

  static void notify(Context &c) noexcept
  {
    *c.link = c.wait->timer_next_;
    c.wait->timer_linked_ = false;
    c.wait->notify();
  }
  struct Scan;
  struct Notify;
  using Graph = flow::graph<
      Scan, flow::types<flow::node<Scan, flow::action<&scan>,
                                   flow::branch<flow::condition<&expired>,
                                                flow::go<Notify>, flow::stop>>,
                        flow::node<Notify, flow::action<&notify>, flow::go<Scan>>>>;
};

PollWait::Page *PollWait::allocate_page()
{
  static_assert(sizeof(Page) <= 4096);
  auto *storage = detail::allocate_process_page();
  if (!storage)
  {
    failed_ = true;
    return nullptr;
  }
  auto *page = new (storage) Page(*this);
  page->next = pages_;
  pages_ = page;
  return page;
}

xv::sync::PollTable *PollWait::table()
{
  if (!pages_ && !allocate_page())
  {
    return nullptr;
  }
  return &pages_->table;
}

void PollWait::watch(void *channel)
{
  if (!channel || failed_)
  {
    return;
  }
  WatchFlow::Context context{*this, channel};
  (void)WatchFlow::Graph::run<0>(context);
}

PollWait::~PollWait()
{
  ReleaseFlow::Context context{*this};
  (void)ReleaseFlow::Graph::run<0>(context);
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
    {
      channels_.scheduler_.make_runnable(process_);
    }
  }
}

void PollWait::set_deadline(unsigned long milliseconds)
{
  channels_.arm_timer(*this, milliseconds);
}

void PollWait::sleep()
{
  SleepFlow::Context context{*this};
  (void)SleepFlow::Graph::run<0>(context);
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
  while (*link && *link != &entry)
  {
    link = &(*link)->next;
  }
  if (!*link)
  {
    detail::storage_fail("poll: missing subscription");
  }
  *link = entry.next;
}

void WaitChannels::arm_timer(PollWait &wait, unsigned long deadline)
{
  xv::sync::ScopedLock guard(timer_lock_);
  if (wait.timer_linked_)
  {
    detail::storage_fail("poll: duplicate timer");
  }
  wait.deadline_ = deadline;
  wait.timer_next_ = timers_;
  wait.timer_linked_ = true;
  timers_ = &wait;
}

void WaitChannels::remove_timer(PollWait &wait)
{
  xv::sync::ScopedLock guard(timer_lock_);
  if (!wait.timer_linked_)
  {
    return;
  }
  auto **link = &timers_;
  while (*link && *link != &wait)
  {
    link = &(*link)->timer_next_;
  }
  if (!*link)
  {
    detail::storage_fail("poll: missing timer");
  }
  *link = wait.timer_next_;
  wait.timer_linked_ = false;
}

void WaitChannels::expire_timers(unsigned long milliseconds)
{
  xv::sync::ScopedLock guard(timer_lock_);
  ExpireFlow::Context context{&timers_, milliseconds};
  (void)ExpireFlow::Graph::run<0>(context);
}
} // namespace xv::process
