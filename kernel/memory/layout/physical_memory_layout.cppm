module;
#include "kernel/support/flow.h"

export module xv.memory:layout;

export import :range;

export namespace xv::memory
{
// Copies of this descriptor own no RAM. The selected allocator owns its metadata.
class PhysicalMemoryLayout final
{
public:

  static constexpr unsigned MaxRanges = 32;

  bool configure(PhysicalMemoryRange ram, PhysicalAddress first_usable)
  {
    if (ram.empty() || !ram.begin().is_aligned() || !ram.end().is_aligned() ||
        first_usable < ram.begin() || first_usable >= ram.end())
    {
      return false;
    }
    auto begin = first_usable.align_up();
    if (begin >= ram.end())
    {
      return false;
    }
    ram_ = ram;
    metadata_ = {};
    usable_count_ = direct_count_ = 1;
    usable_[0] = PhysicalMemoryRange(begin, ram.end());
    direct_[0] = ram;
    return !usable_[0].empty();
  }

  bool reserve(PhysicalMemoryRange reservation, bool no_map = false);

  bool reserve_metadata(MemorySize bytes);

  const PhysicalMemoryRange &ram() const
  {
    return ram_;
  }

  const PhysicalMemoryRange &metadata() const
  {
    return metadata_;
  }

  unsigned usable_count() const
  {
    return usable_count_;
  }

  unsigned direct_count() const
  {
    return direct_count_;
  }

  const PhysicalMemoryRange &usable(unsigned index) const
  {
    if (index >= usable_count_)
    {
      detail::fail("memory layout: invalid usable index");
    }
    return usable_[index];
  }

  const PhysicalMemoryRange &direct(unsigned index) const
  {
    if (index >= direct_count_)
    {
      detail::fail("memory layout: invalid direct index");
    }
    return direct_[index];
  }

  PageIndex usable_pages() const
  {
    PageIndex count = 0;
    for (unsigned i = 0; i < usable_count_; ++i)
    {
      count += usable_[i].page_count();
    }
    return count;
  }

  bool contains_usable_block(PhysicalAddress address, MemorySize bytes) const
  {
    for (unsigned i = 0; i < usable_count_; ++i)
    {
      if (usable_[i].contains_block(address, bytes))
      {
        return true;
      }
    }
    return false;
  }

private:

  struct ReserveFlow;
  struct MetadataFlow;
  struct SubtractFlow;

  static bool can_subtract(const PhysicalMemoryRange *ranges, unsigned count,
                           PhysicalMemoryRange excluded)
  {
    if (count < MaxRanges)
    {
      return true;
    }
    for (unsigned i = 0; i < count; ++i)
    {
      if (excluded.begin() > ranges[i].begin() && excluded.end() < ranges[i].end())
      {
        return false;
      }
    }
    return true;
  }

  // Callers preflight both arrays before subtracting any interval.
  static bool subtract(PhysicalMemoryRange *ranges, unsigned &count,
                       PhysicalMemoryRange excluded);

  PhysicalMemoryRange ram_;
  PhysicalMemoryRange metadata_;
  PhysicalMemoryRange usable_[MaxRanges]{};
  PhysicalMemoryRange direct_[MaxRanges]{};
  unsigned usable_count_ = 0;
  unsigned direct_count_ = 0;
};

namespace flow = xv::flow;

struct PhysicalMemoryLayout::ReserveFlow
{
  struct Context
  {
    PhysicalMemoryLayout &owner;
    PhysicalMemoryRange pages;
    bool no_map;
    bool ok = false, result = false;
  };

  static bool capacity(Context &c) noexcept
  {
    return can_subtract(c.owner.usable_, c.owner.usable_count_, c.pages) &&
           (!c.no_map || can_subtract(c.owner.direct_, c.owner.direct_count_, c.pages));
  }

  static void usable(Context &c) noexcept
  {
    c.ok = subtract(c.owner.usable_, c.owner.usable_count_, c.pages);
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static bool no_map(Context &c) noexcept
  {
    return c.no_map;
  }

  static void direct(Context &c) noexcept
  {
    c.ok = subtract(c.owner.direct_, c.owner.direct_count_, c.pages);
  }

  static void success(Context &c) noexcept
  {
    c.result = true;
  }
  struct Capacity;
  struct Usable;
  struct NoMap;
  struct Direct;
  struct Success;
  using Graph = flow::graph<
      Capacity,
      flow::types<
          flow::node<
              Capacity, flow::noop,
              flow::branch<flow::condition<&capacity>, flow::go<Usable>, flow::stop>>,
          flow::node<Usable, flow::action<&usable>,
                     flow::branch<flow::condition<&ok>, flow::go<NoMap>, flow::stop>>,
          flow::node<NoMap, flow::noop,
                     flow::branch<flow::condition<&no_map>, flow::go<Direct>,
                                  flow::go<Success>>>,
          flow::node<Direct, flow::action<&direct>,
                     flow::branch<flow::condition<&ok>, flow::go<Success>, flow::stop>>,
          flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct PhysicalMemoryLayout::MetadataFlow
{
  struct Context
  {
    PhysicalMemoryLayout &owner;
    MemorySize bytes;
    PhysicalMemoryRange chosen{};
    bool found = false, ok = false, result = false;
  };

  static void choose(Context &c) noexcept
  {
    for (unsigned i = 0; i < c.owner.usable_count_; ++i)
    {
      if (c.owner.usable_[i].size() >= c.bytes)
      {
        auto begin = c.owner.usable_[i].begin();
        c.chosen = PhysicalMemoryRange(begin, begin.advanced(c.bytes));
        c.found = true;
        return;
      }
    }
  }

  static bool found(Context &c) noexcept
  {
    return c.found;
  }

  static void reserve(Context &c) noexcept
  {
    c.ok = c.owner.reserve(c.chosen);
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void publish(Context &c) noexcept
  {
    c.owner.metadata_ = c.chosen;
    c.result = true;
  }
  struct Choose;
  struct Reserve;
  struct Publish;
  using Graph = flow::graph<
      Choose,
      flow::types<
          flow::node<
              Choose, flow::action<&choose>,
              flow::branch<flow::condition<&found>, flow::go<Reserve>, flow::stop>>,
          flow::node<Reserve, flow::action<&reserve>,
                     flow::branch<flow::condition<&ok>, flow::go<Publish>, flow::stop>>,
          flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct PhysicalMemoryLayout::SubtractFlow
{
  struct Context
  {
    PhysicalMemoryRange *ranges;
    unsigned &count;
    PhysicalMemoryRange excluded;
    unsigned index = 0;
    PhysicalMemoryRange range{};
    PhysicalAddress left{}, right{};
    bool found = false, result = false;
  };

  static void scan(Context &c) noexcept
  {
    c.found = false;
    for (; c.index < c.count; ++c.index)
    {
      c.range = c.ranges[c.index];
      if (c.excluded.end() <= c.range.begin() || c.excluded.begin() >= c.range.end())
      {
        continue;
      }
      c.left =
          c.excluded.begin() > c.range.begin() ? c.excluded.begin() : c.range.begin();
      c.right = c.excluded.end() < c.range.end() ? c.excluded.end() : c.range.end();
      c.found = true;
      return;
    }
  }

  static bool found(Context &c) noexcept
  {
    return c.found;
  }

  static bool whole(Context &c) noexcept
  {
    return c.left == c.range.begin() && c.right == c.range.end();
  }

  static bool left(Context &c) noexcept
  {
    return c.left == c.range.begin();
  }

  static bool right(Context &c) noexcept
  {
    return c.right == c.range.end();
  }

  static void remove(Context &c) noexcept
  {
    for (unsigned j = c.index + 1; j < c.count; ++j)
    {
      c.ranges[j - 1] = c.ranges[j];
    }
    --c.count;
  }

  static void trim_left(Context &c) noexcept
  {
    c.ranges[c.index++] = PhysicalMemoryRange(c.right, c.range.end());
  }

  static void trim_right(Context &c) noexcept
  {
    c.ranges[c.index++] = PhysicalMemoryRange(c.range.begin(), c.left);
  }

  static void split(Context &c) noexcept
  {
    for (unsigned j = c.count; j > c.index + 1; --j)
    {
      c.ranges[j] = c.ranges[j - 1];
    }
    c.ranges[c.index] = PhysicalMemoryRange(c.range.begin(), c.left);
    c.ranges[c.index + 1] = PhysicalMemoryRange(c.right, c.range.end());
    ++c.count;
    c.index += 2;
  }

  static void success(Context &c) noexcept
  {
    c.result = true;
  }
  struct Scan;
  struct Whole;
  struct Left;
  struct Right;
  struct Remove;
  struct TrimLeft;
  struct TrimRight;
  struct Split;
  struct Success;
  using Graph = flow::graph<
      Scan,
      flow::types<flow::node<Scan, flow::action<&scan>,
                             flow::branch<flow::condition<&found>, flow::go<Whole>,
                                          flow::go<Success>>>,
                  flow::node<Whole, flow::noop,
                             flow::branch<flow::condition<&whole>, flow::go<Remove>,
                                          flow::go<Left>>>,
                  flow::node<Left, flow::noop,
                             flow::branch<flow::condition<&left>, flow::go<TrimLeft>,
                                          flow::go<Right>>>,
                  flow::node<Right, flow::noop,
                             flow::branch<flow::condition<&right>, flow::go<TrimRight>,
                                          flow::go<Split>>>,
                  flow::node<Remove, flow::action<&remove>, flow::go<Scan>>,
                  flow::node<TrimLeft, flow::action<&trim_left>, flow::go<Scan>>,
                  flow::node<TrimRight, flow::action<&trim_right>, flow::go<Scan>>,
                  flow::node<Split, flow::action<&split>, flow::go<Scan>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

inline bool PhysicalMemoryLayout::reserve(PhysicalMemoryRange reservation, bool no_map)
{
  if (reservation.empty() || reservation.end() <= ram_.begin() ||
      reservation.begin() >= ram_.end())
  {
    return true;
  }
  auto begin = reservation.begin() < ram_.begin() ? ram_.begin() : reservation.begin();
  auto end = reservation.end() > ram_.end() ? ram_.end() : reservation.end();
  ReserveFlow::Context context{
      *this, PhysicalMemoryRange(begin.align_down(), end.align_up()), no_map};
  (void)ReserveFlow::Graph::run<0>(context);
  return context.result;
}

inline bool PhysicalMemoryLayout::reserve_metadata(MemorySize bytes)
{
  if (!metadata_.empty() || bytes == 0 || bytes > ~MemorySize(0) - (PageSize - 1))
  {
    return false;
  }
  bytes = (bytes + PageSize - 1) / PageSize * PageSize;
  MetadataFlow::Context context{*this, bytes};
  (void)MetadataFlow::Graph::run<0>(context);
  return context.result;
}

inline bool PhysicalMemoryLayout::subtract(PhysicalMemoryRange *ranges, unsigned &count,
                                           PhysicalMemoryRange excluded)
{
  for (unsigned i = 0; i < count; ++i)
  {
    if (excluded.begin() > ranges[i].begin() && excluded.end() < ranges[i].end() &&
        count == MaxRanges)
    {
      return false;
    }
  }
  SubtractFlow::Context context{ranges, count, excluded};
  (void)SubtractFlow::Graph::run<0>(context);
  return context.result;
}

} // namespace xv::memory
