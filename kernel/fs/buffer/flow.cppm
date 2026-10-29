module;
#include "kernel/support/flow.h"
#include "kernel/support/placement_new.hpp"
module xv.fs:page_buffer_flow;
import :buffer;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct PageBuffer::PageFlow
{
  struct Context
  {
    PageBuffer &owner;
    unsigned index;
    bool create;
    Page *created = nullptr, *result = nullptr;
  };

  static bool needs_page(Context &c) noexcept
  {
    return c.create && c.owner.pages_ <= c.index;
  }

  static bool allocate(Context &c) noexcept
  {
    auto *memory = detail::allocate_page();
    if (!memory)
    {
      return false;
    }
    c.created = new (memory) Page{};
    return true;
  }

  static void link(Context &c) noexcept
  {
    if (c.owner.tail_)
    {
      c.owner.tail_->next = c.created;
    }
    else
    {
      c.owner.head_ = c.created;
    }
    c.owner.tail_ = c.created;
    ++c.owner.pages_;
  }

  static bool valid(Context &c) noexcept
  {
    return c.index < c.owner.pages_;
  }

  static bool last(Context &c) noexcept
  {
    return c.index + 1 == c.owner.pages_;
  }

  static void tail(Context &c) noexcept
  {
    c.result = c.owner.tail_;
  }

  static void traverse(Context &c) noexcept
  {
    c.result = c.owner.head_;
    auto index = c.index;
    while (index--)
    {
      c.result = c.result->next;
    }
  }
  struct Capacity;
  struct Allocate;
  struct Link;
  struct Validate;
  struct Last;
  struct Tail;
  struct Traverse;
  using Graph = flow::graph<
      Capacity,
      flow::types<
          flow::node<Capacity, flow::noop,
                     flow::branch<flow::condition<&needs_page>, flow::go<Allocate>,
                                  flow::go<Validate>>>,
          flow::node<
              Allocate, flow::noop,
              flow::branch<flow::condition<&allocate>, flow::go<Link>, flow::stop>>,
          flow::node<Link, flow::action<&link>, flow::go<Capacity>>,
          flow::node<Validate, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<Last>, flow::stop>>,
          flow::node<
              Last, flow::noop,
              flow::branch<flow::condition<&last>, flow::go<Tail>, flow::go<Traverse>>>,
          flow::node<Tail, flow::action<&tail>, flow::stop>,
          flow::node<Traverse, flow::action<&traverse>, flow::stop>>>;
};

struct PageBuffer::WriteFlow
{
  struct Context
  {
    PageBuffer &owner;
    unsigned offset;
    bool user;
    unsigned long source;
    unsigned count;
    unsigned done = 0, inside = 0, part = 0;
    Page *block = nullptr;
    int result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.count <= 0x7fffffffU && c.offset <= 0x7fffffffU - c.count;
  }

  static bool gap(Context &c) noexcept
  {
    return c.count && c.offset > c.owner.size_;
  }

  static bool fill_gap(Context &c) noexcept
  {
    unsigned position = c.owner.size_;
    while (position < c.offset)
    {
      unsigned inside = position % sizeof(Page::bytes);
      auto *block = c.owner.page(position / sizeof(Page::bytes), true);
      if (!block)
      {
        return false;
      }
      unsigned part = sizeof(Page::bytes) - inside;
      if (part > c.offset - position)
      {
        part = c.offset - position;
      }
      detail::fill(block->bytes + inside, 0, part);
      position += part;
    }
    return true;
  }

  static bool more(Context &c) noexcept
  {
    return c.done < c.count;
  }

  static bool select_block(Context &c) noexcept
  {
    unsigned position = c.offset + c.done;
    c.inside = position % sizeof(Page::bytes);
    c.block = c.owner.page(position / sizeof(Page::bytes), true);
    if (!c.block)
    {
      return false;
    }
    c.part = sizeof(Page::bytes) - c.inside;
    if (c.part > c.count - c.done)
    {
      c.part = c.count - c.done;
    }
    return true;
  }

  static bool copy(Context &c) noexcept
  {
    return detail::copy_in(c.block->bytes + c.inside, c.user, c.source + c.done,
                           c.part) >= 0;
  }

  static void progress(Context &c) noexcept
  {
    c.done += c.part;
    if (c.offset + c.done > c.owner.size_)
    {
      c.owner.size_ = c.offset + c.done;
    }
    c.result = c.done;
  }

  static void complete(Context &c) noexcept
  {
    c.result = c.done;
  }
  struct Validate;
  struct Gap;
  struct FillGap;
  struct More;
  struct Block;
  struct Copy;
  struct Progress;
  struct Complete;
  using Graph = flow::graph<
      Validate,
      flow::types<
          flow::node<Validate, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<Gap>, flow::stop>>,
          flow::node<
              Gap, flow::noop,
              flow::branch<flow::condition<&gap>, flow::go<FillGap>, flow::go<More>>>,
          flow::node<
              FillGap, flow::noop,
              flow::branch<flow::condition<&fill_gap>, flow::go<More>, flow::stop>>,
          flow::node<More, flow::noop,
                     flow::branch<flow::condition<&more>, flow::go<Block>,
                                  flow::go<Complete>>>,
          flow::node<Block, flow::noop,
                     flow::branch<flow::condition<&select_block>, flow::go<Copy>,
                                  flow::stop>>,
          flow::node<
              Copy, flow::noop,
              flow::branch<flow::condition<&copy>, flow::go<Progress>, flow::stop>>,
          flow::node<Progress, flow::action<&progress>, flow::go<More>>,
          flow::node<Complete, flow::action<&complete>, flow::stop>>>;
};

struct PageBuffer::ReadFlow
{
  struct Context
  {
    const PageBuffer &owner;
    unsigned offset;
    bool user;
    unsigned long destination;
    unsigned count;
    unsigned done = 0, inside = 0, part = 0;
    const Page *block = nullptr;
    int result = 0;
  };

  static bool in_range(Context &c) noexcept
  {
    return c.offset < c.owner.size_;
  }

  static void clamp(Context &c) noexcept
  {
    if (c.count > c.owner.size_ - c.offset)
    {
      c.count = c.owner.size_ - c.offset;
    }
  }

  static bool more(Context &c) noexcept
  {
    return c.done < c.count;
  }

  static void select_block(Context &c) noexcept
  {
    unsigned position = c.offset + c.done;
    c.inside = position % sizeof(Page::bytes);
    c.block = c.owner.page(position / sizeof(Page::bytes));
    if (!c.block)
    {
      detail::fail("page buffer: missing data page");
    }
    c.part = sizeof(Page::bytes) - c.inside;
    if (c.part > c.count - c.done)
    {
      c.part = c.count - c.done;
    }
  }

  static bool copy(Context &c) noexcept
  {
    return detail::copy_out(c.user, c.destination + c.done, c.block->bytes + c.inside,
                            c.part) >= 0;
  }

  static void progress(Context &c) noexcept
  {
    c.done += c.part;
    c.result = c.done;
  }

  static void failure(Context &c) noexcept
  {
    c.result = c.done ? static_cast<int>(c.done) : -1;
  }
  struct Range;
  struct Clamp;
  struct More;
  struct Block;
  struct Progress;
  struct Failure;
  using Graph = flow::graph<
      Range,
      flow::types<
          flow::node<
              Range, flow::noop,
              flow::branch<flow::condition<&in_range>, flow::go<Clamp>, flow::stop>>,
          flow::node<Clamp, flow::action<&clamp>, flow::go<More>>,
          flow::node<More, flow::noop,
                     flow::branch<flow::condition<&more>, flow::go<Block>, flow::stop>>,
          flow::node<Block, flow::action<&select_block>,
                     flow::branch<flow::condition<&copy>, flow::go<Progress>,
                                  flow::go<Failure>>>,
          flow::node<Progress, flow::action<&progress>, flow::go<More>>,
          flow::node<Failure, flow::action<&failure>, flow::stop>>>;
};

struct PageBuffer::ResizeFlow
{
  struct Context
  {
    PageBuffer &owner;
    unsigned size;
    unsigned keep = 0;
    Page **link = nullptr;
    Page *last = nullptr;
    bool result = false;
  };

  static bool valid(Context &c) noexcept
  {
    return c.size <= 0x7fffffffU;
  }

  static bool growing(Context &c) noexcept
  {
    return c.size > c.owner.size_;
  }

  static void extend(Context &c) noexcept
  {
    char zero = 0;
    c.result = c.owner.write(c.size - 1, false, reinterpret_cast<unsigned long>(&zero),
                             1) == 1;
  }

  static void select_tail(Context &c) noexcept
  {
    c.keep = (c.size + sizeof(Page::bytes) - 1) / sizeof(Page::bytes);
    c.link = &c.owner.head_;
    for (unsigned i = 0; i < c.keep; ++i)
    {
      c.last = *c.link;
      c.link = &c.last->next;
    }
  }

  static void release_tail(Context &c) noexcept
  {
    while (*c.link)
    {
      auto *next = (*c.link)->next;
      (*c.link)->~Page();
      detail::free_page(*c.link);
      *c.link = next;
    }
  }

  static void commit(Context &c) noexcept
  {
    c.owner.tail_ = c.last;
    c.owner.pages_ = c.keep;
    c.owner.size_ = c.size;
    if (c.last && c.size % sizeof(Page::bytes))
    {
      detail::fill(c.last->bytes + c.size % sizeof(Page::bytes), 0,
                   sizeof(Page::bytes) - c.size % sizeof(Page::bytes));
    }
    c.result = true;
  }
  struct Validate;
  struct Direction;
  struct Extend;
  struct Tail;
  struct Release;
  struct Commit;
  using Graph = flow::graph<
      Validate,
      flow::types<flow::node<Validate, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Direction>,
                                          flow::stop>>,
                  flow::node<Direction, flow::noop,
                             flow::branch<flow::condition<&growing>, flow::go<Extend>,
                                          flow::go<Tail>>>,
                  flow::node<Extend, flow::action<&extend>, flow::stop>,
                  flow::node<Tail, flow::action<&select_tail>, flow::go<Release>>,
                  flow::node<Release, flow::action<&release_tail>, flow::go<Commit>>,
                  flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

} // namespace xv::fs
