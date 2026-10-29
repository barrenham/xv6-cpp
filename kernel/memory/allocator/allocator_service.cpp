module;

#include "kernel/support/placement_new.hpp"
#include "kernel/support/flow.h"

module xv.memory;

import :buddy;
import :freelist;
import :detail;

namespace xv::memory
{

namespace
{
constexpr auto StorageSize = sizeof(BuddyPageAllocator) > sizeof(FreeListPageAllocator)
                                 ? sizeof(BuddyPageAllocator)
                                 : sizeof(FreeListPageAllocator);
constexpr auto StorageAlign = alignof(BuddyPageAllocator) >
                                      alignof(FreeListPageAllocator)
                                  ? alignof(BuddyPageAllocator)
                                  : alignof(FreeListPageAllocator);

alignas(StorageAlign) unsigned char storage[StorageSize];
PhysicalPageAllocator *active = nullptr;
int initializing = 0;
} // namespace

bool initialize(AllocatorKind kind, const BootMemoryInfo &memory,
                PhysicalAddress first_usable)
{
  const auto detected = memory.ram();
  const auto ram = detected.page_aligned();
  if ((kind != AllocatorKind::Buddy && kind != AllocatorKind::FreeList) ||
      ram.empty() || detected.begin() != detail::KernelLoadAddress ||
      detected.end() > detail::DirectMapLimit || !ram.begin().is_aligned() ||
      !ram.end().is_aligned() || first_usable < ram.begin() ||
      first_usable >= ram.end())
  {
    return false;
  }
  PhysicalMemoryLayout planned;
  namespace flow = xv::flow;

  struct InitializeFlow
  {
    struct Context
    {
      AllocatorKind kind;
      const BootMemoryInfo &memory;
      PhysicalAddress first;
      PhysicalMemoryRange ram;
      PhysicalMemoryLayout &planned;
      unsigned index = 0;
      PhysicalPageAllocator *selected = nullptr;
      bool ok = false, result = false;
    };

    static void configure(Context &c) noexcept
    {
      c.ok = c.planned.configure(c.ram, c.first);
    }

    static bool ok(Context &c) noexcept
    {
      return c.ok;
    }

    static bool remaining(Context &c) noexcept
    {
      return c.index < c.memory.reservation_count();
    }

    static void reserve(Context &c) noexcept
    {
      const auto &reservation = c.memory.reservation(c.index);
      c.ok = !(reservation.no_map && reservation.range.begin() < c.first &&
               reservation.range.end() > c.ram.begin()) &&
             c.planned.reserve(reservation.range, reservation.no_map);
    }

    static void next(Context &c) noexcept
    {
      ++c.index;
    }

    static void claim(Context &c) noexcept
    {
      int expected = 0;
      c.ok = __atomic_compare_exchange_n(&initializing, &expected, 1, false,
                                         __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    }

    static void construct(Context &c) noexcept
    {
      switch (c.kind)
      {
      case AllocatorKind::Buddy:
        c.selected = new (storage) BuddyPageAllocator{};
        break;
      case AllocatorKind::FreeList:
        c.selected = new (storage) FreeListPageAllocator{};
        break;
      default:
        __builtin_unreachable();
      }
    }

    static void metadata(Context &c) noexcept
    {
      c.ok = c.planned.reserve_metadata(
                 ((c.selected->metadata_bytes(c.ram.page_count()) + 3) & ~3UL) +
                 c.ram.page_count() * sizeof(unsigned)) &&
             c.planned.usable_pages() != 0;
    }

    static void reset(Context &) noexcept
    {
      __atomic_store_n(&initializing, 0, __ATOMIC_RELEASE);
    }

    static void initialize_layout(Context &c) noexcept
    {
      c.selected->layout_ = c.planned;
      c.selected->initialize_layout();
      auto bytes = (c.selected->metadata_bytes(c.ram.page_count()) + 3) & ~3UL;
      c.selected->shared_refs_ =
          reinterpret_cast<unsigned *>(c.planned.metadata().begin().raw() + bytes);
      detail::fill(c.selected->shared_refs_, 0, c.ram.page_count() * sizeof(unsigned));
      c.selected->free_pages_ = 0;
      for (unsigned i = 0; i < c.planned.usable_count(); ++i)
      {
        c.selected->free_pages_ += c.planned.usable(i).page_count();
      }
    }

    static void publish(Context &c) noexcept
    {
      __atomic_store_n(&active, c.selected, __ATOMIC_RELEASE);
      c.result = true;
    }
    struct Configure;
    struct Reservations;
    struct Reserve;
    struct Next;
    struct Claim;
    struct Construct;
    struct Metadata;
    struct Reset;
    struct Initialize;
    struct Publish;
    using Graph = flow::graph<
        Configure,
        flow::types<
            flow::node<
                Configure, flow::action<&configure>,
                flow::branch<flow::condition<&ok>, flow::go<Reservations>, flow::stop>>,
            flow::node<Reservations, flow::noop,
                       flow::branch<flow::condition<&remaining>, flow::go<Reserve>,
                                    flow::go<Claim>>>,
            flow::node<Reserve, flow::action<&reserve>,
                       flow::branch<flow::condition<&ok>, flow::go<Next>, flow::stop>>,
            flow::node<Next, flow::action<&next>, flow::go<Reservations>>,
            flow::node<Claim, flow::action<&claim>,
                       flow::branch<flow::condition<&ok>, flow::go<Construct>,
                                    flow::stop>>,
            flow::node<Construct, flow::action<&construct>, flow::go<Metadata>>,
            flow::node<Metadata, flow::action<&metadata>,
                       flow::branch<flow::condition<&ok>, flow::go<Initialize>,
                                    flow::go<Reset>>>,
            flow::node<Reset, flow::action<&reset>, flow::stop>,
            flow::node<Initialize, flow::action<&initialize_layout>, flow::go<Publish>>,
            flow::node<Publish, flow::action<&publish>, flow::stop>>>;
  };

  InitializeFlow::Context context{kind, memory, first_usable, ram, planned};
  (void)InitializeFlow::Graph::run<0>(context);
  return context.result;
}

bool initialize(AllocatorKind kind, BootMemorySource &source,
                PhysicalAddress first_usable)
{
  BootMemoryInfo memory;
  return source.discover(memory) && initialize(kind, memory, first_usable);
}

bool initialize(AllocatorKind kind, PhysicalMemoryRange range)
{
  if (range.empty() || range.begin() < detail::KernelLoadAddress ||
      range.end() > detail::DirectMapLimit)
  {
    return false;
  }
  auto ram_end = range.end().align_down();
  if (ram_end <= range.begin())
  {
    return false;
  }
  BootMemoryInfo memory;
  if (!memory.set_ram(PhysicalMemoryRange(detail::KernelLoadAddress, ram_end)))
  {
    return false;
  }
  return initialize(kind, memory, range.begin());
}

bool initialize(AllocatorKind kind, void *first, void *last)
{
  auto begin = PhysicalAddress::from_pointer(first);
  auto end = PhysicalAddress::from_pointer(last);
  if (begin >= end)
  {
    return false;
  }
  return initialize(kind, PhysicalMemoryRange(begin, end));
}

void PhysicalPageAllocator::retain_page(void *address)
{
  auto physical = PhysicalAddress::from_pointer(address);
  if (!shared_refs_ || !layout_.contains_usable_block(physical, PageSize) ||
      !physical.is_aligned())
  {
    detail::fail("page retain: invalid frame");
  }
  auto *count = &shared_refs_[layout_.ram().page_index(physical)];
  auto old = __atomic_load_n(count, __ATOMIC_ACQUIRE);
  do
  {
    if (old == ~0U)
    {
      detail::fail("page retain: overflow");
    }
  } while (!__atomic_compare_exchange_n(count, &old, old ? old + 1 : 2, false,
                                        __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));
}

void PhysicalPageAllocator::release_page(void *address)
{
  auto physical = PhysicalAddress::from_pointer(address);
  if (!shared_refs_ || !layout_.contains_usable_block(physical, PageSize) ||
      !physical.is_aligned())
  {
    detail::fail("page release: invalid frame");
  }
  auto *count = &shared_refs_[layout_.ram().page_index(physical)];
  auto old = __atomic_load_n(count, __ATOMIC_ACQUIRE);
  for (;;)
  {
    if (!old)
    {
      deallocate(address);
      return;
    }
    if (__atomic_compare_exchange_n(count, &old, old - 1, false, __ATOMIC_ACQ_REL,
                                    __ATOMIC_ACQUIRE))
    {
      if (old == 1)
      {
        deallocate(address);
      }
      return;
    }
  }
}

unsigned PhysicalPageAllocator::page_references(void *address) const
{
  auto physical = PhysicalAddress::from_pointer(address);
  if (!shared_refs_ || !layout_.contains_usable_block(physical, PageSize) ||
      !physical.is_aligned())
  {
    detail::fail("page references: invalid frame");
  }
  auto count = __atomic_load_n(&shared_refs_[layout_.ram().page_index(physical)],
                               __ATOMIC_ACQUIRE);
  return count ? count : 1;
}

PhysicalPageAllocator &allocator()
{
  auto *selected = __atomic_load_n(&active, __ATOMIC_ACQUIRE);
  if (!selected)
  {
    detail::fail("physical allocator: accessed before initialization");
  }
  return *selected;
}
} // namespace xv::memory
