module;
#include "kernel/support/flow.h"
#include "kernel/arch/riscv/register_constants.h"
module xv.vm_regions;
import xv.memory;

namespace xv::memory
{
namespace flow = xv::flow;

struct VirtualRegions::ReserveFlow
{
  struct Context
  {
    VirtualRegions &owner;
    unsigned long begin, end;
    unsigned permissions;
    RegionKind kind;
    bool demand;
    VirtualRegion *adjacent = nullptr, *entry = nullptr;
    bool result = false;
  };

  static bool valid(Context &c) noexcept
  {
    return c.begin < c.end && c.end <= (1UL << 38) &&
           !(c.permissions & ~(PTE_R | PTE_W | PTE_X | PTE_U)) &&
           (!(c.permissions & PTE_W) || (c.permissions & PTE_R)) &&
           (c.kind != RegionKind::Guard || (!c.permissions && !c.demand));
  }

  static bool scan(Context &c) noexcept
  {
    for (auto *page = c.owner.pages_; page; page = page->next)
    {
      for (auto &entry : page->entries)
      {
        if (entry.begin == entry.end)
        {
          continue;
        }
        if (c.begin < entry.end && entry.begin < c.end)
        {
          return false;
        }
        if (entry.permissions == c.permissions && entry.kind == c.kind &&
            entry.demand == c.demand && (entry.end == c.begin || entry.begin == c.end))
        {
          c.adjacent = &entry;
        }
      }
    }
    return true;
  }

  static bool adjacent(Context &c) noexcept
  {
    return c.adjacent != nullptr;
  }

  static void extend(Context &c) noexcept
  {
    if (c.adjacent->end == c.begin)
    {
      c.adjacent->end = c.end;
    }
    else
    {
      c.adjacent->begin = c.begin;
    }
  }

  static bool allocate(Context &c) noexcept
  {
    c.entry = c.owner.vacant();
    return c.entry != nullptr;
  }

  static void insert(Context &c) noexcept
  {
    *c.entry = {c.begin, c.end, c.permissions, c.kind, c.demand};
  }

  static void success(Context &c) noexcept
  {
    c.result = true;
  }
  struct Validate;
  struct Scan;
  struct Adjacent;
  struct Extend;
  struct Allocate;
  struct Insert;
  struct Success;
  using Graph = flow::graph<
      Validate,
      flow::types<
          flow::node<Validate, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<Scan>, flow::stop>>,
          flow::node<
              Scan, flow::noop,
              flow::branch<flow::condition<&scan>, flow::go<Adjacent>, flow::stop>>,
          flow::node<Adjacent, flow::noop,
                     flow::branch<flow::condition<&adjacent>, flow::go<Extend>,
                                  flow::go<Allocate>>>,
          flow::node<Extend, flow::action<&extend>, flow::go<Success>>,
          flow::node<
              Allocate, flow::noop,
              flow::branch<flow::condition<&allocate>, flow::go<Insert>, flow::stop>>,
          flow::node<Insert, flow::action<&insert>, flow::go<Success>>,
          flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct VirtualRegions::EraseFlow
{
  struct Context
  {
    VirtualRegions &owner;
    unsigned long begin, end;
    VirtualRegion *split = nullptr, *tail = nullptr;
    bool result = false;
  };

  static bool valid(Context &c) noexcept
  {
    return c.begin < c.end;
  }

  static void find_split(Context &c) noexcept
  {
    for (auto *page = c.owner.pages_; page; page = page->next)
    {
      for (auto &entry : page->entries)
      {
        if (entry.begin < c.begin && c.end < entry.end)
        {
          c.split = &entry;
        }
      }
    }
  }

  static bool split_needed(Context &c) noexcept
  {
    return c.split != nullptr;
  }

  static bool allocate(Context &c) noexcept
  {
    c.tail = c.owner.vacant();
    return c.tail != nullptr;
  }

  static void split(Context &c) noexcept
  {
    *c.tail = *c.split;
    c.tail->begin = c.end;
    c.split->end = c.begin;
  }

  static void trim(Context &c) noexcept
  {
    for (auto *page = c.owner.pages_; page; page = page->next)
    {
      for (auto &entry : page->entries)
      {
        if (entry.begin == entry.end || entry.end <= c.begin || entry.begin >= c.end)
        {
          continue;
        }
        if (c.begin <= entry.begin && c.end >= entry.end)
        {
          entry = {};
        }
        else if (c.begin <= entry.begin)
        {
          entry.begin = c.end;
        }
        else
        {
          entry.end = c.begin;
        }
      }
    }
  }

  static void reclaim(Context &c) noexcept
  {
    auto **link = &c.owner.pages_;
    while (*link)
    {
      auto *page = *link;
      bool empty = true;
      for (const auto &entry : page->entries)
      {
        if (entry.begin != entry.end)
        {
          empty = false;
        }
      }
      if (empty)
      {
        *link = page->next;
        allocator().deallocate(page);
      }
      else
      {
        link = &page->next;
      }
    }
  }

  static void success(Context &c) noexcept
  {
    c.result = true;
  }
  struct Validate;
  struct FindSplit;
  struct Allocate;
  struct Split;
  struct Trim;
  struct Reclaim;
  struct Success;
  using Graph = flow::graph<
      Validate,
      flow::types<flow::node<Validate, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<FindSplit>,
                                          flow::stop>>,
                  flow::node<FindSplit, flow::action<&find_split>,
                             flow::branch<flow::condition<&split_needed>,
                                          flow::go<Allocate>, flow::go<Trim>>>,
                  flow::node<Allocate, flow::noop,
                             flow::branch<flow::condition<&allocate>, flow::go<Split>,
                                          flow::stop>>,
                  flow::node<Split, flow::action<&split>, flow::go<Success>>,
                  flow::node<Trim, flow::action<&trim>, flow::go<Reclaim>>,
                  flow::node<Reclaim, flow::action<&reclaim>, flow::go<Success>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct VirtualRegions::CopyFlow
{
  struct Context
  {
    const VirtualRegions &owner;
    VirtualRegions &other;
    VirtualRegions candidate;
    bool result = false;
  };

  static bool populate(Context &c) noexcept
  {
    for (auto *page = c.owner.pages_; page; page = page->next)
    {
      for (const auto &entry : page->entries)
      {
        if (entry.begin != entry.end &&
            !c.candidate.reserve(entry.begin, entry.end, entry.permissions, entry.kind,
                                 entry.demand))
        {
          return false;
        }
      }
    }
    return true;
  }

  static void rollback(Context &c) noexcept
  {
    c.candidate.reset();
  }

  static void commit(Context &c) noexcept
  {
    c.other.reset();
    c.other.swap(c.candidate);
    c.result = true;
  }
  struct Populate;
  struct Rollback;
  struct Commit;
  using Graph = flow::graph<
      Populate,
      flow::types<flow::node<Populate, flow::noop,
                             flow::branch<flow::condition<&populate>, flow::go<Commit>,
                                          flow::go<Rollback>>>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>,
                  flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

VirtualRegion *VirtualRegions::vacant()
{
  for (auto *page = pages_; page; page = page->next)
  {
    for (auto &entry : page->entries)
    {
      if (entry.begin == entry.end)
      {
        return &entry;
      }
    }
  }
  static_assert(sizeof(Page) <= PageSize);
  auto *page = static_cast<Page *>(allocator().allocate());
  if (!page)
  {
    return nullptr;
  }
  auto *bytes = reinterpret_cast<unsigned char *>(page);
  for (unsigned long i = 0; i < PageSize; ++i)
  {
    bytes[i] = 0;
  }
  page->next = pages_;
  pages_ = page;
  return &page->entries[0];
}

const VirtualRegion *VirtualRegions::find(unsigned long address) const
{
  for (auto *page = pages_; page; page = page->next)
  {
    for (const auto &entry : page->entries)
    {
      if (entry.begin <= address && address < entry.end)
      {
        return &entry;
      }
    }
  }
  return nullptr;
}

bool VirtualRegions::reserve(unsigned long begin, unsigned long end,
                             unsigned permissions, RegionKind kind, bool demand)
{
  ReserveFlow::Context context{*this, begin, end, permissions, kind, demand};
  (void)ReserveFlow::Graph::run<0>(context);
  return context.result;
}

bool VirtualRegions::erase(unsigned long begin, unsigned long end)
{
  EraseFlow::Context context{*this, begin, end};
  (void)EraseFlow::Graph::run<0>(context);
  return context.result;
}

bool VirtualRegions::copy_to(VirtualRegions &other) const
{
  CopyFlow::Context context{*this, other, {}};
  (void)CopyFlow::Graph::run<0>(context);
  return context.result;
}

void VirtualRegions::swap(VirtualRegions &other)
{
  auto *page = pages_;
  pages_ = other.pages_;
  other.pages_ = page;
}

void VirtualRegions::reset()
{
  while (pages_)
  {
    auto *page = pages_;
    pages_ = page->next;
    allocator().deallocate(page);
  }
}
} // namespace xv::memory
