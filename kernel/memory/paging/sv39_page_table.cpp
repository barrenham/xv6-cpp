module;
#include "kernel/support/flow.h"

module xv.memory;

import :sv39;
import :detail;

namespace xv::memory
{
namespace
{
constexpr MemorySize Valid = 1, Leaf = Sv39PageTable::Read | Sv39PageTable::Write |
                                       Sv39PageTable::Execute;

MemorySize *child_table(MemorySize entry)
{
  return PhysicalAddress((entry >> 10) << 12).pointer<MemorySize>();
}

MemorySize encode(PhysicalAddress address)
{
  return (address.raw() >> 12) << 10;
}

unsigned index_at(VirtualAddress va, unsigned level)
{
  return (va >> (12 + 9 * level)) & 511;
}
} // namespace

Sv39LeafLevel LargestPagePolicy::choose(VirtualAddress va, PhysicalAddress pa,
                                        MemorySize remaining) const
{
  for (int level = static_cast<int>(maximum_); level >= 0; --level)
  {
    auto size = Sv39PageTable::leaf_size(static_cast<Sv39LeafLevel>(level));
    if (remaining >= size && va % size == 0 && pa.is_aligned(size))
    {
      return static_cast<Sv39LeafLevel>(level);
    }
  }
  return Sv39LeafLevel::Page4KiB;
}

OwnedSv39PageTable::OwnedSv39PageTable(PhysicalPageAllocator &memory,
                                       const PageMappingPolicy &policy)
    : root_(memory), view_(root_.get(), memory, policy)
{
  if (root_)
  {
    detail::fill(root_.get(), 0, PageSize);
  }
}

namespace flow = xv::flow;

struct Sv39PageTable::LeafFlow
{
  struct Context
  {
    Sv39PageTable &owner;
    VirtualAddress va;
    PhysicalAddress pa;
    unsigned target;
    MemorySize permissions;
    MemorySize *table, *entry = nullptr;
    unsigned level = 2;
    MappingResult result = MappingResult::Ok;
  };

  static bool descending(Context &c) noexcept
  {
    return c.level > c.target;
  }

  static void lookup(Context &c) noexcept
  {
    c.entry = &c.table[index_at(c.va, c.level)];
  }

  static bool valid(Context &c) noexcept
  {
    return *c.entry & Valid;
  }

  static bool leaf(Context &c) noexcept
  {
    return *c.entry & Leaf;
  }

  static void collision(Context &c) noexcept
  {
    c.result = MappingResult::Collision;
  }

  static void descend(Context &c) noexcept
  {
    c.table = child_table(*c.entry);
  }

  static void allocate(Context &c) noexcept
  {
    OwnedPage candidate(c.owner.memory_);
    if (!candidate)
    {
      c.result = MappingResult::OutOfMemory;
      return;
    }
    detail::fill(candidate.get(), 0, PageSize);
    auto address = PhysicalAddress::from_pointer(candidate.get());
    if (!address.is_aligned())
    {
      c.result = MappingResult::InvalidRange;
      return;
    }
    *c.entry = encode(address) | Valid;
    c.table = static_cast<MemorySize *>(candidate.release());
  }

  static bool ok(Context &c) noexcept
  {
    return c.result == MappingResult::Ok;
  }

  static void next(Context &c) noexcept
  {
    --c.level;
  }

  static void publish(Context &c) noexcept
  {
    auto &entry = c.table[index_at(c.va, c.target)];
    if (entry & Valid)
    {
      c.result = MappingResult::Collision;
      return;
    }
    entry = encode(c.pa) | c.permissions | Valid;
    c.result = MappingResult::Ok;
  }
  struct Level;
  struct Lookup;
  struct ExistingLeaf;
  struct Collision;
  struct Descend;
  struct Allocate;
  struct Next;
  struct Publish;
  using Graph = flow::graph<
      Level,
      flow::types<
          flow::node<Level, flow::noop,
                     flow::branch<flow::condition<&descending>, flow::go<Lookup>,
                                  flow::go<Publish>>>,
          flow::node<Lookup, flow::action<&lookup>,
                     flow::branch<flow::condition<&valid>, flow::go<ExistingLeaf>,
                                  flow::go<Allocate>>>,
          flow::node<ExistingLeaf, flow::noop,
                     flow::branch<flow::condition<&leaf>, flow::go<Collision>,
                                  flow::go<Descend>>>,
          flow::node<Collision, flow::action<&collision>, flow::stop>,
          flow::node<Descend, flow::action<&descend>, flow::go<Next>>,
          flow::node<Allocate, flow::action<&allocate>,
                     flow::branch<flow::condition<&ok>, flow::go<Next>, flow::stop>>,
          flow::node<Next, flow::action<&next>, flow::go<Level>>,
          flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct Sv39PageTable::RangeFlow
{
  struct Context
  {
    Sv39PageTable &owner;
    VirtualAddress va;
    PhysicalAddress pa;
    MemorySize bytes, permissions;
    Sv39LeafLevel level = Sv39LeafLevel::Page4KiB;
    MemorySize step = 0;
    MappingResult result = MappingResult::InvalidRange;
  };

  static bool root(Context &c) noexcept
  {
    return c.owner.root_ != nullptr;
  }

  static void out_of_memory(Context &c) noexcept
  {
    c.result = MappingResult::OutOfMemory;
  }

  static bool range(Context &c) noexcept
  {
    return c.bytes && !(c.va % PageSize) && c.pa.is_aligned() &&
           !(c.bytes % PageSize) && c.va < VirtualLimit &&
           c.bytes <= VirtualLimit - c.va && c.pa.raw() < (1UL << 56) &&
           c.bytes <= (1UL << 56) - c.pa.raw();
  }

  static bool permissions(Context &c) noexcept
  {
    constexpr MemorySize Allowed =
        Read | Write | Execute | User | Global | Accessed | Dirty;
    return !(c.permissions & ~Allowed) && (c.permissions & (Read | Execute)) &&
           (!(c.permissions & Write) || (c.permissions & Read));
  }

  static void invalid_permissions(Context &c) noexcept
  {
    c.result = MappingResult::InvalidPermissions;
  }

  static void invalid_range(Context &c) noexcept
  {
    c.result = MappingResult::InvalidRange;
  }

  static bool remaining(Context &c) noexcept
  {
    return c.bytes != 0;
  }

  static void choose(Context &c) noexcept
  {
    c.level = c.owner.policy_.choose(c.va, c.pa, c.bytes);
  }

  static bool level(Context &c) noexcept
  {
    return static_cast<unsigned>(c.level) <= 2;
  }

  static void step(Context &c) noexcept
  {
    c.step = leaf_size(c.level);
  }

  static bool aligned(Context &c) noexcept
  {
    return c.step <= c.bytes && !(c.va % c.step) && c.pa.is_aligned(c.step);
  }

  static void map(Context &c) noexcept
  {
    c.result =
        c.owner.map_leaf(c.va, c.pa, static_cast<unsigned>(c.level), c.permissions);
  }

  static bool ok(Context &c) noexcept
  {
    return c.result == MappingResult::Ok;
  }

  static void advance(Context &c) noexcept
  {
    c.va += c.step;
    c.pa = c.pa.advanced(c.step);
    c.bytes -= c.step;
  }

  static void success(Context &c) noexcept
  {
    c.result = MappingResult::Ok;
  }
  struct Root;
  struct OutOfMemory;
  struct Range;
  struct Permissions;
  struct InvalidPermissions;
  struct InvalidRange;
  struct Remaining;
  struct Choose;
  struct Step;
  struct Map;
  struct Advance;
  struct Success;
  using Graph = flow::graph<
      Root,
      flow::types<
          flow::node<Root, flow::noop,
                     flow::branch<flow::condition<&root>, flow::go<Range>,
                                  flow::go<OutOfMemory>>>,
          flow::node<OutOfMemory, flow::action<&out_of_memory>, flow::stop>,
          flow::node<Range, flow::noop,
                     flow::branch<flow::condition<&range>, flow::go<Permissions>,
                                  flow::go<InvalidRange>>>,
          flow::node<Permissions, flow::noop,
                     flow::branch<flow::condition<&permissions>, flow::go<Remaining>,
                                  flow::go<InvalidPermissions>>>,
          flow::node<InvalidPermissions, flow::action<&invalid_permissions>,
                     flow::stop>,
          flow::node<InvalidRange, flow::action<&invalid_range>, flow::stop>,
          flow::node<Remaining, flow::noop,
                     flow::branch<flow::condition<&remaining>, flow::go<Choose>,
                                  flow::go<Success>>>,
          flow::node<Choose, flow::action<&choose>,
                     flow::branch<flow::condition<&level>, flow::go<Step>,
                                  flow::go<InvalidRange>>>,
          flow::node<Step, flow::action<&step>,
                     flow::branch<flow::condition<&aligned>, flow::go<Map>,
                                  flow::go<InvalidRange>>>,
          flow::node<Map, flow::action<&map>,
                     flow::branch<flow::condition<&ok>, flow::go<Advance>, flow::stop>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Remaining>>,
          flow::node<Success, flow::action<&success>, flow::stop>>>;
};

MappingResult Sv39PageTable::map_leaf(VirtualAddress va, PhysicalAddress pa,
                                      unsigned target, MemorySize permissions)
{
  LeafFlow::Context context{*this, va, pa, target, permissions, root_};
  (void)LeafFlow::Graph::run<0>(context);
  return context.result;
}

MappingResult Sv39PageTable::map_range(VirtualAddress va, PhysicalAddress pa,
                                       MemorySize bytes, MemorySize permissions)
{
  RangeFlow::Context context{*this, va, pa, bytes, permissions};
  (void)RangeFlow::Graph::run<0>(context);
  return context.result;
}

bool Sv39PageTable::translate(VirtualAddress va, PageTranslation &result) const
{
  if (!root_ || va >= VirtualLimit)
  {
    return false;
  }
  const auto *table = root_;
  for (int level = 2; level >= 0; --level)
  {
    auto entry = table[index_at(va, level)];
    if (!(entry & Valid))
    {
      return false;
    }
    if (entry & Leaf)
    {
      auto physical = PhysicalAddress((entry >> 10) << 12);
      auto leaf = static_cast<Sv39LeafLevel>(level);
      auto bytes = leaf_size(leaf);
      if (!physical.is_aligned(bytes) || ((entry & Write) && !(entry & Read)))
      {
        return false;
      }
      result = {physical.advanced(va % bytes), entry & 0x3ff, leaf};
      return true;
    }
    if (level == 0)
    {
      return false;
    }
    table = child_table(entry);
  }
  return false;
}

void Sv39PageTable::count_table(const MemorySize *table, unsigned level,
                                PageTableStatistics &result)
{
  ++result.table_pages;
  for (unsigned i = 0; i < 512; ++i)
  {
    auto entry = __atomic_load_n(&table[i], __ATOMIC_ACQUIRE);
    if (!(entry & Valid))
    {
      continue;
    }
    if (entry & Leaf)
    {
      if (level == 2)
      {
        ++result.leaves_1g;
      }
      else if (level == 1)
      {
        ++result.leaves_2m;
      }
      else
      {
        ++result.leaves_4k;
      }
    }
    else if (level)
    {
      count_table(child_table(entry), level - 1, result);
    }
  }
}

PageTableStatistics Sv39PageTable::statistics() const
{
  PageTableStatistics result;
  if (root_)
  {
    count_table(root_, 2, result);
  }
  return result;
}

void Sv39PageTable::clear_table(MemorySize *table, unsigned level)
{
  for (unsigned i = 0; i < 512; ++i)
  {
    auto entry = table[i];
    if ((entry & Valid) && !(entry & Leaf) && level)
    {
      auto *child = child_table(entry);
      clear_table(child, level - 1);
      memory_.deallocate(child);
    }
    table[i] = 0;
  }
}

void Sv39PageTable::clear()
{
  if (root_)
  {
    clear_table(root_, 2);
  }
}
} // namespace xv::memory
