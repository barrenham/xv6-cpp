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
  };

  static void find_cached(Context &c) noexcept
  {
    for (auto *buffer = c.owner.head_.next; buffer != &c.owner.head_;
         buffer = buffer->next)
    {
      if (buffer->dev == c.device && buffer->blockno == c.block)
      {
        c.result = buffer;
        break;
      }
    }
  }

  static bool found(Context &c) noexcept
  {
    return c.result != nullptr;
  }

  static void retain(Context &c) noexcept
  {
    ++c.result->refcnt;
  }

  static void find_unused(Context &c) noexcept
  {
    for (auto *buffer = c.owner.head_.prev; buffer != &c.owner.head_;
         buffer = buffer->prev)
    {
      if (buffer->refcnt == 0)
      {
        c.result = buffer;
        break;
      }
    }
  }

  static void assign(Context &c) noexcept
  {
    c.result->dev = c.device;
    c.result->blockno = c.block;
    c.result->valid = 0;
    c.result->refcnt = 1;
  }

  static void missing(Context &) noexcept
  {
    detail::fail("bget: no buffers");
  }

  static void unlock_registry(Context &c) noexcept
  {
    c.owner.lock_.release();
  }

  static void lock_buffer(Context &c) noexcept
  {
    c.result->lock.acquire();
  }
  struct Cached;
  struct Retain;
  struct Unused;
  struct Assign;
  struct Missing;
  struct Unlock;
  struct BufferLock;
  using Graph = flow::graph<
      Cached,
      flow::types<
          flow::node<Cached, flow::action<&find_cached>,
                     flow::branch<flow::condition<&found>, flow::go<Retain>,
                                  flow::go<Unused>>>,
          flow::node<Retain, flow::action<&retain>, flow::go<Unlock>>,
          flow::node<Unused, flow::action<&find_unused>,
                     flow::branch<flow::condition<&found>, flow::go<Assign>,
                                  flow::go<Missing>>>,
          flow::node<Assign, flow::action<&assign>, flow::go<Unlock>>,
          flow::node<Missing, flow::action<&missing>, flow::stop>,
          flow::node<Unlock, flow::action<&unlock_registry>, flow::go<BufferLock>>,
          flow::node<BufferLock, flow::action<&lock_buffer>, flow::stop>>>;
};

struct LruBlockCache::ReleaseFlow
{
  struct Context
  {
    LruBlockCache &owner;
    Buffer &buffer;
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
    c.owner.lock_.acquire();
    --c.buffer.refcnt;
  }

  static bool unused(Context &c) noexcept
  {
    return c.buffer.refcnt == 0;
  }

  static void move_front(Context &c) noexcept
  {
    auto *buffer = &c.buffer;
    buffer->next->prev = buffer->prev;
    buffer->prev->next = buffer->next;
    buffer->next = c.owner.head_.next;
    buffer->prev = &c.owner.head_;
    c.owner.head_.next->prev = buffer;
    c.owner.head_.next = buffer;
  }

  static void unlock_registry(Context &c) noexcept
  {
    c.owner.lock_.release();
  }
  struct Held;
  struct Invalid;
  struct BufferUnlock;
  struct Drop;
  struct Move;
  struct Unlock;
  using Graph = flow::graph<
      Held, flow::types<
                flow::node<Held, flow::noop,
                           flow::branch<flow::condition<&held>, flow::go<BufferUnlock>,
                                        flow::go<Invalid>>>,
                flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                flow::node<BufferUnlock, flow::action<&unlock_buffer>, flow::go<Drop>>,
                flow::node<Drop, flow::action<&drop>,
                           flow::branch<flow::condition<&unused>, flow::go<Move>,
                                        flow::go<Unlock>>>,
                flow::node<Move, flow::action<&move_front>, flow::go<Unlock>>,
                flow::node<Unlock, flow::action<&unlock_registry>, flow::stop>>>;
};

} // namespace xv::fs
