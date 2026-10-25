module;

#include "kernel/support/placement_new.hpp"

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
  if (!planned.configure(ram, first_usable))
  {
    return false;
  }
  for (unsigned i = 0; i < memory.reservation_count(); ++i)
  {
    const auto &reservation = memory.reservation(i);
    if (reservation.no_map && reservation.range.begin() < first_usable &&
        reservation.range.end() > ram.begin())
    {
      return false;
    }
    if (!planned.reserve(reservation.range, reservation.no_map))
    {
      return false;
    }
  }
  int expected = 0;
  if (!__atomic_compare_exchange_n(&initializing, &expected, 1, false, __ATOMIC_ACQ_REL,
                                   __ATOMIC_ACQUIRE))
  {
    return false;
  }
  PhysicalPageAllocator *selected;
  switch (kind)
  {
  case AllocatorKind::Buddy:
    selected = new (storage) BuddyPageAllocator{};
    break;
  case AllocatorKind::FreeList:
    selected = new (storage) FreeListPageAllocator{};
    break;
  default:
    __builtin_unreachable();
  }
  if (!planned.reserve_metadata(selected->metadata_bytes(ram.page_count())) ||
      planned.usable_pages() == 0)
  {
    __atomic_store_n(&initializing, 0, __ATOMIC_RELEASE);
    return false;
  }
  selected->layout_ = planned;
  selected->initialize_layout();
  __atomic_store_n(&active, selected, __ATOMIC_RELEASE);
  return true;
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
