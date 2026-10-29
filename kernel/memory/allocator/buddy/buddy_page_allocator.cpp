module;
#include "kernel/support/flow.h"

module xv.memory;

import :buddy;
import :detail;

namespace xv::memory
{

namespace flow = xv::flow;

struct BuddyPageAllocator::AllocateFlow
{
  struct Context
  {
    BuddyPageAllocator &owner;
    unsigned order, found;
    PhysicalAddress address{};
    void *result = nullptr;
  };

  static void search(Context &c) noexcept
  {
    for (; c.found <= SuperpageOrder; ++c.found)
    {
      c.owner.area_[c.found].lock.acquire();
      if (c.owner.area_[c.found].head)
      {
        c.address = PhysicalAddress::from_pointer(c.owner.area_[c.found].head);
        c.owner.remove_block(c.address, c.found);
      }
      c.owner.area_[c.found].lock.release();
      if (!c.address.is_null())
      {
        break;
      }
    }
  }

  static bool found(Context &c) noexcept
  {
    return !c.address.is_null();
  }

  static bool splitting(Context &c) noexcept
  {
    return c.found > c.order;
  }

  static void split(Context &c) noexcept
  {
    --c.found;
    c.owner.area_[c.found].lock.acquire();
    c.owner.insert(c.address.advanced(detail::block_size(c.found)), c.found);
    c.owner.area_[c.found].lock.release();
  }

  static void publish(Context &c) noexcept
  {
    c.owner.account_free(-(1L << c.order));
    c.owner.set_tag(c.owner.layout_.ram().page_index(c.address),
                    AllocBase + static_cast<int>(c.order));
    detail::fill(c.address.pointer(), 5, detail::block_size(c.order));
    c.result = c.address.pointer();
  }
  struct Search;
  struct Splitting;
  struct Split;
  struct Publish;
  using Graph = flow::graph<
      Search, flow::types<flow::node<Search, flow::action<&search>,
                                     flow::branch<flow::condition<&found>,
                                                  flow::go<Splitting>, flow::stop>>,
                          flow::node<Splitting, flow::noop,
                                     flow::branch<flow::condition<&splitting>,
                                                  flow::go<Split>, flow::go<Publish>>>,
                          flow::node<Split, flow::action<&split>,
                                     flow::branch<flow::condition<&splitting>,
                                                  flow::go<Split>, flow::go<Publish>>>,
                          flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct BuddyPageAllocator::DeallocateFlow
{
  struct Context
  {
    BuddyPageAllocator &owner;
    void *memory;
    PhysicalAddress address{};
    unsigned order;
    bool merge = false;
    PhysicalAddress buddy{};
  };

  static void claim(Context &c) noexcept
  {
    c.owner.claim(c.address, c.order);
    c.owner.account_free(1L << c.order);
    detail::fill(c.memory, 1, detail::block_size(c.order));
  }

  static void inspect(Context &c) noexcept
  {
    c.owner.area_[c.order].lock.acquire();
    auto index = c.owner.layout_.ram().page_index(c.address) ^ (1UL << c.order);
    c.merge = c.order < SuperpageOrder && index < c.owner.layout_.ram().page_count() &&
              c.owner.page_tag(index) == FreeBase + static_cast<int>(c.order);
    if (c.merge)
    {
      c.buddy = c.owner.layout_.ram().page_address(index);
      c.owner.remove_block(c.buddy, c.order);
    }
    else
    {
      c.owner.insert(c.address, c.order);
    }
    c.owner.area_[c.order].lock.release();
  }

  static bool merging(Context &c) noexcept
  {
    return c.merge;
  }

  static void advance(Context &c) noexcept
  {
    if (c.buddy < c.address)
    {
      c.address = c.buddy;
    }
    ++c.order;
  }
  struct Claim;
  struct Inspect;
  struct Advance;
  using Graph = flow::graph<
      Claim,
      flow::types<flow::node<Claim, flow::action<&claim>, flow::go<Inspect>>,
                  flow::node<Inspect, flow::action<&inspect>,
                             flow::branch<flow::condition<&merging>, flow::go<Advance>,
                                          flow::stop>>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Inspect>>>>;
};

struct BuddyPageAllocator::InitializeFlow
{
  struct Context
  {
    BuddyPageAllocator &owner;
    unsigned region = 0;
    PhysicalAddress address{};
  };

  static void metadata(Context &c) noexcept
  {
    c.owner.tags_ = c.owner.layout_.metadata().begin().pointer<int>();
    detail::fill(c.owner.tags_, 0, c.owner.layout_.metadata().size());
    for (unsigned order = 0; order <= SuperpageOrder; ++order)
    {
      c.owner.area_[order].lock.init("buddy");
    }
  }

  static bool remaining(Context &c) noexcept
  {
    return c.region < c.owner.layout_.usable_count();
  }

  static void range(Context &c) noexcept
  {
    c.address = c.owner.layout_.usable(c.region).begin();
  }

  static bool inside(Context &c) noexcept
  {
    return c.address < c.owner.layout_.usable(c.region).end();
  }

  static void seed(Context &c) noexcept
  {
    const auto &usable = c.owner.layout_.usable(c.region);
    unsigned order = SuperpageOrder;
    while (!c.address.is_aligned(detail::block_size(order)) ||
           !usable.contains_block(c.address, detail::block_size(order)))
    {
      --order;
    }
    c.owner.insert(c.address, order);
    c.address = c.address.advanced(detail::block_size(order));
  }

  static void next(Context &c) noexcept
  {
    ++c.region;
  }
  struct Metadata;
  struct Range;
  struct Seed;
  struct Next;
  using Graph =
      flow::graph<Metadata,
                  flow::types<flow::node<Metadata, flow::action<&metadata>,
                                         flow::branch<flow::condition<&remaining>,
                                                      flow::go<Range>, flow::stop>>,
                              flow::node<Range, flow::action<&range>,
                                         flow::branch<flow::condition<&inside>,
                                                      flow::go<Seed>, flow::go<Next>>>,
                              flow::node<Seed, flow::action<&seed>,
                                         flow::branch<flow::condition<&inside>,
                                                      flow::go<Seed>, flow::go<Next>>>,
                              flow::node<Next, flow::action<&next>,
                                         flow::branch<flow::condition<&remaining>,
                                                      flow::go<Range>, flow::stop>>>>;
};

int BuddyPageAllocator::page_tag(PageIndex index) const
{
  return __atomic_load_n(&tags_[index], __ATOMIC_ACQUIRE);
}

void BuddyPageAllocator::set_tag(PageIndex index, int tag)
{
  __atomic_store_n(&tags_[index], tag, __ATOMIC_RELEASE);
}

bool BuddyPageAllocator::valid_block(PhysicalAddress pa, unsigned order) const
{
  if (order > SuperpageOrder)
  {
    return 0;
  }
  MemorySize size = detail::block_size(order);
  return pa.is_aligned(size) && layout_.contains_usable_block(pa, size);
}

// Each helper's caller holds only the lock for this order. No operation
// acquires another order's lock while holding the current one.
void BuddyPageAllocator::insert(PhysicalAddress pa, unsigned order)
{
  Run *r = pa.pointer<Run>();
  r->prev = 0;
  r->next = area_[order].head;
  if (r->next)
  {
    r->next->prev = r;
  }
  area_[order].head = r;
  set_tag(layout_.ram().page_index(pa), FreeBase + static_cast<int>(order));
}

void BuddyPageAllocator::remove_block(PhysicalAddress pa, unsigned order)
{
  Run *r = pa.pointer<Run>();
  set_tag(layout_.ram().page_index(pa), Busy);
  if (r->prev)
  {
    r->prev->next = r->next;
  }
  else
  {
    area_[order].head = r->next;
  }
  if (r->next)
  {
    r->next->prev = r->prev;
  }
}

void *BuddyPageAllocator::allocate(unsigned order)
{
  if (order > SuperpageOrder)
  {
    return nullptr;
  }
  AllocateFlow::Context context{*this, order, order, PhysicalAddress()};
  (void)AllocateFlow::Graph::run<0>(context);
  return context.result;
}

void BuddyPageAllocator::claim(PhysicalAddress pa, unsigned order)
{
  int expected = AllocBase + static_cast<int>(order);
  if (!__atomic_compare_exchange_n(&tags_[layout_.ram().page_index(pa)], &expected,
                                   Busy, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
  {
    detail::fail("buddy: state or order");
  }
}

void BuddyPageAllocator::deallocate(void *mem, unsigned order)
{
  auto address = PhysicalAddress::from_pointer(mem);
  if (!valid_block(address, order))
  {
    detail::fail("deallocate: range or alignment");
  }
  DeallocateFlow::Context context{*this, mem, address, order};
  (void)DeallocateFlow::Graph::run<0>(context);
}

// Seed the allocator with maximal aligned blocks. A non-power-of-two RAM
// range, its reserved prefix, and an incomplete trailing page are handled
// without rounding usable memory beyond its discovered region.
void BuddyPageAllocator::initialize_layout()
{
  InitializeFlow::Context context{*this};
  (void)InitializeFlow::Graph::run<0>(context);
}

// A demoted user superpage becomes 512 independently owned order-0 pages.
// No page becomes free here; data and the VM mappings remain untouched.
bool BuddyPageAllocator::split_to_pages(void *mem, unsigned order)
{
  if (!supports(order))
  {
    return false;
  }
  PhysicalAddress pa = PhysicalAddress::from_pointer(mem);
  if (!valid_block(pa, order))
  {
    detail::fail("supersplit: range");
  }
  claim(pa, order);
  PageIndex index = layout_.ram().page_index(pa);
  // Publish the original head last. The caller owns the allocation and
  // makes the demoted mappings available only after this function returns.
  for (int i = (1 << order) - 1; i >= 0; i--)
  {
    set_tag(index + i, AllocBase);
  }
  return true;
}
} // namespace xv::memory
