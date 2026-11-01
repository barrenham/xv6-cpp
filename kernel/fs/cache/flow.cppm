module;
#include "kernel/support/flow.h"
module xv.fs:cache_flow;
import :lru;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct LruBlockCache::GetFlow
{
  struct Context
  {
    LruBlockCache &owner;
    unsigned device, block;
    Buffer *result = nullptr;
    Bucket *target = nullptr, *old = nullptr;
    unsigned long stamp = 0;
  };

  static void find_cached(Context &c) noexcept
  {
    c.target = &c.owner.buckets_[index(c.device, c.block)];
    c.owner.acquire(c.target->lock, c.target->statistics);
    lookup(c);
  }

  static void lookup(Context &c) noexcept
  {
    c.result = nullptr;
    unsigned steps = 0;
    for (auto *buffer = c.target->head; buffer; buffer = buffer->next)
    {
      ++steps;
      if (buffer->dev == c.device && buffer->blockno == c.block)
      {
        c.result = buffer;
        break;
      }
    }
    increment(c.target->statistics.lookup_steps, steps);
  }

  static bool found(Context &c) noexcept
  {
    return c.result != nullptr;
  }

  static void reference(Context &c) noexcept
  {
    auto refs = __atomic_load_n(&c.result->refcnt, __ATOMIC_RELAXED);
    __atomic_store_n(&c.result->refcnt, refs + 1, __ATOMIC_RELAXED);
  }

  static void retain(Context &c) noexcept
  {
    increment(c.target->statistics.hits);
    reference(c);
  }

  static void miss(Context &c) noexcept
  {
    increment(c.target->statistics.misses);
    c.target->lock.release();
    c.owner.acquire(c.owner.eviction_lock_, c.owner.eviction_statistics_);
  }

  static void recheck(Context &c) noexcept
  {
    // Recheck after obtaining replacement ownership: concurrent misses for the
    // same key must share the first published buffer, including in-flight I/O.
    c.owner.acquire(c.target->lock, c.target->statistics);
    lookup(c);
  }

  static void unlock_registry(Context &c) noexcept
  {
    c.target->lock.release();
  }

  static void unlock_eviction(Context &c) noexcept
  {
    c.owner.eviction_lock_.release();
  }

  static void find_unused(Context &c) noexcept
  {
    select(c);
  }

  static void select(Context &c) noexcept
  {
    c.result = nullptr;
    unsigned steps = 0;
    while (!c.result)
    {
      if (!c.owner.candidate_count_)
      {
        // Batch the ordering work instead of rescanning NBUF for each miss.
        // Concurrent hits may invalidate entries; bucket-locked revalidation
        // below prevents reuse of a busy or newly referenced buffer.
        for (auto &buffer : c.owner.buffers_)
        {
          ++steps;
          if (__atomic_load_n(&buffer.refcnt, __ATOMIC_RELAXED))
          {
            continue;
          }
          Candidate candidate{&buffer,
                              __atomic_load_n(&buffer.last_use_, __ATOMIC_RELAXED)};
          unsigned position = c.owner.candidate_count_++;
          while (position && candidate.stamp > c.owner.candidates_[position - 1].stamp)
          {
            c.owner.candidates_[position] = c.owner.candidates_[position - 1];
            --position;
          }
          c.owner.candidates_[position] = candidate;
        }
        if (!c.owner.candidate_count_)
        {
          break;
        }
      }
      auto candidate = c.owner.candidates_[--c.owner.candidate_count_];
      ++steps;
      if (__atomic_load_n(&candidate.buffer->refcnt, __ATOMIC_RELAXED) ||
          __atomic_load_n(&candidate.buffer->last_use_, __ATOMIC_RELAXED) !=
              candidate.stamp)
      {
        continue;
      }
      c.result = candidate.buffer;
      c.stamp = candidate.stamp;
    }
    increment(c.owner.eviction_statistics_.eviction_steps, steps);
  }

  static void lock_victim(Context &c) noexcept
  {
    // Other replacements are excluded; owner bucket is stable. Ordinary hits
    // can race with selection, so the reference count must be rechecked.
    c.old = &c.owner.buckets_[c.result->bucket_];
    if (c.old != c.target)
    {
      c.owner.acquire(c.old->lock, c.old->statistics);
    }
  }

  static bool unused(Context &c) noexcept
  {
    return !__atomic_load_n(&c.result->refcnt, __ATOMIC_RELAXED) &&
           __atomic_load_n(&c.result->last_use_, __ATOMIC_RELAXED) == c.stamp;
  }

  static void retry(Context &c) noexcept
  {
    if (c.old != c.target)
    {
      c.old->lock.release();
    }
    select(c);
  }

  static void assign(Context &c) noexcept
  {
    // Only the eviction owner ever holds two bucket locks. No hit path takes
    // eviction_lock_, so arbitrary old/target order cannot form a cycle.
    auto &buffer = *c.result;
    if (buffer.assigned_)
    {
      auto **link = &c.old->head;
      while (*link && *link != &buffer)
      {
        link = &(*link)->next;
      }
      if (!*link)
      {
        detail::fail("cache: missing hash identity");
      }
      *link = buffer.next;
      increment(c.target->statistics.evictions);
    }
    buffer.assigned_ = true;
    buffer.dev = c.device;
    buffer.blockno = c.block;
    buffer.valid = 0;
    buffer.bucket_ = index(c.device, c.block);
    __atomic_store_n(&buffer.refcnt, 1U, __ATOMIC_RELAXED);
    buffer.next = c.target->head;
    c.target->head = &buffer;
    if (c.old != c.target)
    {
      c.old->lock.release();
    }
  }

  static void missing(Context &c) noexcept
  {
    c.target->lock.release();
    c.owner.eviction_lock_.release();
    detail::fail("bget: no buffers");
  }

  static void lock_buffer(Context &c) noexcept
  {
    c.result->lock.acquire();
  }
  struct Cached;
  struct Retain;
  struct Miss;
  struct Recheck;
  struct Reuse;
  struct Select;
  struct Victim;
  struct Retry;
  struct Assign;
  struct Missing;
  struct Unlock;
  struct EvictionUnlock;
  struct BufferLock;
  struct ReuseUnlock;
  using Graph = flow::graph<
      Cached,
      flow::types<
          flow::node<
              Cached, flow::action<&find_cached>,
              flow::branch<flow::condition<&found>, flow::go<Retain>, flow::go<Miss>>>,
          flow::node<Retain, flow::action<&retain>, flow::go<Unlock>>,
          flow::node<Miss, flow::action<&miss>, flow::go<Recheck>>,
          flow::node<
              Recheck, flow::action<&recheck>,
              flow::branch<flow::condition<&found>, flow::go<Reuse>, flow::go<Select>>>,
          flow::node<Reuse, flow::action<&reference>, flow::go<ReuseUnlock>>,
          flow::node<ReuseUnlock, flow::action<&unlock_registry>,
                     flow::go<EvictionUnlock>>,
          flow::node<Select, flow::action<&find_unused>,
                     flow::branch<flow::condition<&found>, flow::go<Victim>,
                                  flow::go<Missing>>>,
          flow::node<Victim, flow::action<&lock_victim>,
                     flow::branch<flow::condition<&unused>, flow::go<Assign>,
                                  flow::go<Retry>>>,
          flow::node<Retry, flow::action<&retry>,
                     flow::branch<flow::condition<&found>, flow::go<Victim>,
                                  flow::go<Missing>>>,
          flow::node<Assign, flow::action<&assign>, flow::go<ReuseUnlock>>,
          flow::node<Missing, flow::action<&missing>, flow::stop>,
          flow::node<EvictionUnlock, flow::action<&unlock_eviction>,
                     flow::go<BufferLock>>,
          flow::node<Unlock, flow::action<&unlock_registry>, flow::go<BufferLock>>,
          flow::node<BufferLock, flow::action<&lock_buffer>, flow::stop>>>;
};

struct LruBlockCache::ReleaseFlow
{
  struct Context
  {
    LruBlockCache &owner;
    Buffer &buffer;
    Bucket *bucket = nullptr;
  };

  static bool held(Context &c) noexcept
  {
    return c.buffer.lock.holding();
  }

  static void invalid(Context &) noexcept
  {
    detail::fail("brelse");
  }

  static void unlock_buffer(Context &c) noexcept
  {
    c.buffer.lock.release();
  }

  static void drop(Context &c) noexcept
  {
    c.bucket = &c.owner.buckets_[c.buffer.bucket_];
    c.owner.acquire(c.bucket->lock, c.bucket->statistics);
    auto refs = __atomic_load_n(&c.buffer.refcnt, __ATOMIC_RELAXED);
    if (!refs)
    {
      detail::fail("cache: release without reference");
    }
    __atomic_store_n(&c.buffer.refcnt, refs - 1, __ATOMIC_RELAXED);
  }

  static bool unused(Context &c) noexcept
  {
    return !__atomic_load_n(&c.buffer.refcnt, __ATOMIC_RELAXED);
  }

  static void stamp(Context &c) noexcept
  {
    __atomic_store_n(&c.buffer.last_use_, detail::cache_ticks(), __ATOMIC_RELAXED);
  }

  static void unlock_registry(Context &c) noexcept
  {
    c.bucket->lock.release();
  }
  struct Held;
  struct Invalid;
  struct BufferUnlock;
  struct Drop;
  struct Stamp;
  struct Unlock;
  using Graph = flow::graph<
      Held, flow::types<
                flow::node<Held, flow::noop,
                           flow::branch<flow::condition<&held>, flow::go<BufferUnlock>,
                                        flow::go<Invalid>>>,
                flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                flow::node<BufferUnlock, flow::action<&unlock_buffer>, flow::go<Drop>>,
                flow::node<Drop, flow::action<&drop>,
                           flow::branch<flow::condition<&unused>, flow::go<Stamp>,
                                        flow::go<Unlock>>>,
                flow::node<Stamp, flow::action<&stamp>, flow::go<Unlock>>,
                flow::node<Unlock, flow::action<&unlock_registry>, flow::stop>>>;
};
} // namespace xv::fs
