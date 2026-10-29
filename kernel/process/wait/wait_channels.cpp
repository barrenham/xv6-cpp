module;
#include "kernel/support/flow.h"

module xv.process;
import :wait_channels;
import :storage_platform;
import :poll;

namespace xv::process
{

namespace flow = xv::flow;

struct WaitChannels::CancelFlow
{
  struct Context
  {
    WaitChannels &owner;
    Process &process;
    void *key;
    bool done = false;
  };

  static void attempt(Context &c) noexcept
  {
    auto &bucket = c.owner.buckets_[index(c.key)];
    xv::sync::ScopedLock bucket_guard(bucket.lock);
    xv::sync::ScopedLock process_guard(c.process.lock);
    c.done = __atomic_load_n(&c.process.wait_key_, __ATOMIC_RELAXED) == c.key;
    if (c.done)
    {
      unlink(bucket, c.process);
    }
  }

  static bool done(Context &c) noexcept
  {
    return c.done;
  }

  static void reload(Context &c) noexcept
  {
    c.key = __atomic_load_n(&c.process.wait_key_, __ATOMIC_ACQUIRE);
  }

  static bool registered(Context &c) noexcept
  {
    return c.key != nullptr;
  }
  struct Attempt;
  struct Reload;
  using Graph =
      flow::graph<Attempt,
                  flow::types<flow::node<Attempt, flow::action<&attempt>,
                                         flow::branch<flow::condition<&done>,
                                                      flow::stop, flow::go<Reload>>>,
                              flow::node<Reload, flow::action<&reload>,
                                         flow::branch<flow::condition<&registered>,
                                                      flow::go<Attempt>, flow::stop>>>>;
};

struct WaitChannels::PrepareFlow
{
  struct Context
  {
    Process &process;
    Bucket &bucket;
    void *channel;
    bool interruptible;
  };

  static bool interrupted(Context &c) noexcept
  {
    return c.interruptible &&
           (c.process.killed || c.process.pending_signals_.interrupting());
  }

  static void reject(Context &c) noexcept
  {
    if (c.process.pending_signals_.caught_signal())
    {
      c.process.syscall_interrupted_ = true;
    }
    c.process.chan = nullptr;
    c.process.interruptible_ = false;
  }

  static void publish(Context &c) noexcept
  {
    c.process.interruptible_ = c.interruptible;
    c.process.chan = c.channel;
    c.process.wait_next_ = c.bucket.head;
    c.bucket.head = &c.process;
    __atomic_store_n(&c.process.wait_key_, c.channel, __ATOMIC_RELEASE);
  }
  struct Check;
  struct Reject;
  struct Publish;
  using Graph = flow::graph<
      Check, flow::types<flow::node<Check, flow::noop,
                                    flow::branch<flow::condition<&interrupted>,
                                                 flow::go<Reject>, flow::go<Publish>>>,
                         flow::node<Reject, flow::action<&reject>, flow::stop>,
                         flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct WaitChannels::WakeFlow
{
  struct Context
  {
    WaitChannels &owner;
    Bucket &bucket;
    void *channel;
    Process *process, *next = nullptr;
  };

  static void scan(Context &c) noexcept
  {
    while (c.process)
    {
      c.next = c.process->wait_next_;
      if (__atomic_load_n(&c.process->wait_key_, __ATOMIC_RELAXED) == c.channel)
      {
        return;
      }
      c.process = c.next;
    }
  }

  static bool found(Context &c) noexcept
  {
    return c.process != nullptr;
  }

  static void wake(Context &c) noexcept
  {
    xv::sync::ScopedLock process_guard(c.process->lock);
    unlink(c.bucket, *c.process);
    if (c.process->state == ProcessState::Sleeping)
    {
      c.owner.scheduler_.make_runnable(*c.process);
    }
    // The bucket lock pins the saved successor throughout traversal.
    c.process = c.next;
  }

  static void pollers(Context &c) noexcept
  {
    for (auto *entry = c.bucket.pollers; entry; entry = entry->next)
    {
      if (entry->channel == c.channel)
      {
        entry->owner->notify();
      }
    }
  }
  struct Scan;
  struct Wake;
  struct Pollers;
  using Graph = flow::graph<
      Scan, flow::types<flow::node<Scan, flow::action<&scan>,
                                   flow::branch<flow::condition<&found>, flow::go<Wake>,
                                                flow::go<Pollers>>>,
                        flow::node<Wake, flow::action<&wake>, flow::go<Scan>>,
                        flow::node<Pollers, flow::action<&pollers>, flow::stop>>>;
};

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
  auto *key = __atomic_load_n(&process.wait_key_, __ATOMIC_ACQUIRE);
  if (!key)
  {
    return;
  }
  CancelFlow::Context context{*this, process, key};
  (void)CancelFlow::Graph::run<0>(context);
}

void WaitChannels::prepare(Process &process, void *channel, bool interruptible)
{
  if (!channel)
  {
    detail::storage_fail("prepare_sleep: zero channel");
  }
  // Retire old membership before acquiring the new bucket and process locks.
  cancel(process);
  auto &bucket = buckets_[index(channel)];
  xv::sync::ScopedLock bucket_guard(bucket.lock);
  xv::sync::ScopedLock process_guard(process.lock);
  PrepareFlow::Context context{process, bucket, channel, interruptible};
  (void)PrepareFlow::Graph::run<0>(context);
}

void WaitChannels::wake(void *channel)
{
  if (!channel)
  {
    return;
  }
  auto &bucket = buckets_[index(channel)];
  xv::sync::ScopedLock bucket_guard(bucket.lock);
  WakeFlow::Context context{*this, bucket, channel, bucket.head};
  (void)WakeFlow::Graph::run<0>(context);
}
} // namespace xv::process
