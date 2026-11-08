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
PageReclaimer page_reclaimer = nullptr;
PageReclaimer slow_page_reclaimer = nullptr;
constexpr unsigned Revoked = 1U << 31, ReferenceMask = Revoked - 1;
unsigned long reclamation_epoch = 1;
unsigned long revoked_pages = 0;
int initializing = 0;
} // namespace

void
set_page_reclaimer(PageReclaimer callback) {
  __atomic_store_n(&page_reclaimer, callback, __ATOMIC_RELEASE);
}

unsigned
reclaim_pages(unsigned target) {
  auto callback = __atomic_load_n(&page_reclaimer, __ATOMIC_ACQUIRE);
  return callback && target ? callback(target) : 0;
}

void
set_slow_page_reclaimer(PageReclaimer callback) {
  __atomic_store_n(&slow_page_reclaimer, callback, __ATOMIC_RELEASE);
}

unsigned
reclaim_slow_pages(unsigned target) {
  auto callback = __atomic_load_n(&slow_page_reclaimer, __ATOMIC_ACQUIRE);
  return callback && target ? callback(target) : 0;
}

unsigned long
page_reclamation_epoch() {
  return __atomic_load_n(&reclamation_epoch, __ATOMIC_ACQUIRE);
}

unsigned long
revoked_page_count() {
  return __atomic_load_n(&revoked_pages, __ATOMIC_ACQUIRE);
}

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
    if ((old & ReferenceMask) == ReferenceMask)
    {
      detail::fail("page retain: overflow");
    }
  } while (!__atomic_compare_exchange_n(
      count, &old,
      (old & Revoked) | ((old & ReferenceMask) ? (old & ReferenceMask) + 1 : 2), false,
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
    auto references = old & ReferenceMask;
    if (!references)
    {
      detail::fail("page release: revoked frame without owner");
    }
    auto next = references == 1 ? 0 : old - 1;
    if (__atomic_compare_exchange_n(count, &old, next, false, __ATOMIC_ACQ_REL,
                                    __ATOMIC_ACQUIRE))
    {
      if (references == 1)
      {
        if (old & Revoked)
        {
          __atomic_sub_fetch(&revoked_pages, 1UL, __ATOMIC_RELEASE);
        }
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
  count &= ReferenceMask;
  return count ? count : 1;
}

void
PhysicalPageAllocator::revoke_page(void *address) {
  auto physical = PhysicalAddress::from_pointer(address);
  if (!shared_refs_ || !layout_.contains_usable_block(physical, PageSize) ||
      !physical.is_aligned())
  {
    detail::fail("page revoke: invalid frame");
  }
  auto *slot = &shared_refs_[layout_.ram().page_index(physical)];
  auto old = __atomic_load_n(slot, __ATOMIC_ACQUIRE);
  for (;;)
  {
    if (old & Revoked)
    {
      return;
    }
    unsigned next = Revoked | (old ? old : 1);
    if (__atomic_compare_exchange_n(slot, &old, next, false, __ATOMIC_ACQ_REL,
                                    __ATOMIC_ACQUIRE))
    {
      break;
    }
  }
  __atomic_add_fetch(&reclamation_epoch, 1UL, __ATOMIC_RELEASE);
  __atomic_add_fetch(&revoked_pages, 1UL, __ATOMIC_RELEASE);
}

bool
PhysicalPageAllocator::page_revoked(void *address) const {
  auto physical = PhysicalAddress::from_pointer(address);
  if (!shared_refs_ || !layout_.contains_usable_block(physical, PageSize) ||
      !physical.is_aligned())
  {
    detail::fail("page revoked: invalid frame");
  }
  return __atomic_load_n(&shared_refs_[layout_.ram().page_index(physical)],
                         __ATOMIC_ACQUIRE) &
         Revoked;
}

bool
PhysicalPageAllocator::adopt_revoked_page(void *address) {
  auto physical = PhysicalAddress::from_pointer(address);
  if (!shared_refs_ || !layout_.contains_usable_block(physical, PageSize) ||
      !physical.is_aligned())
  {
    detail::fail("page adopt: invalid frame");
  }
  auto *slot = &shared_refs_[layout_.ram().page_index(physical)];
  unsigned expected = Revoked | 1U;
  bool adopted = __atomic_compare_exchange_n(slot, &expected, 1U, false,
                                             __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
  if (adopted)
  {
    __atomic_sub_fetch(&revoked_pages, 1UL, __ATOMIC_RELEASE);
  }
  return adopted;
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
