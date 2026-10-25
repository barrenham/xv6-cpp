module xv.memory;

import :buddy;
import :detail;

namespace xv::memory
{
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
    return 0;
  }
  PhysicalAddress pa;
  unsigned found;
  for (found = order; found <= SuperpageOrder; found++)
  {
    area_[found].lock.acquire();
    if (area_[found].head)
    {
      pa = PhysicalAddress::from_pointer(area_[found].head);
      remove_block(pa, found);
    }
    area_[found].lock.release();
    if (!pa.is_null())
    {
      break;
    }
  }
  if (pa.is_null())
  {
    return 0;
  }

  // The popped block remains privately owned while each right half is
  // published under its own order's lock.
  while (found > order)
  {
    found--;
    area_[found].lock.acquire();
    insert(pa.advanced(detail::block_size(found)), found);
    area_[found].lock.release();
  }
  set_tag(layout_.ram().page_index(pa), AllocBase + static_cast<int>(order));
  detail::fill(pa.pointer(), 5, detail::block_size(order));
  return pa.pointer();
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
  PhysicalAddress pa = PhysicalAddress::from_pointer(mem);
  if (!valid_block(pa, order))
  {
    detail::fail("deallocate: range or alignment");
  }
  claim(pa, order);
  detail::fill(mem, 1, detail::block_size(order));

  for (;;)
  {
    area_[order].lock.acquire();
    PageIndex buddy = layout_.ram().page_index(pa) ^ (1UL << order);
    if (order < SuperpageOrder && buddy < layout_.ram().page_count() &&
        page_tag(buddy) == FreeBase + static_cast<int>(order))
    {
      PhysicalAddress buddy_pa = layout_.ram().page_address(buddy);
      remove_block(buddy_pa, order);
      area_[order].lock.release();
      // Both halves are Busy before the merged block moves up an order.
      if (buddy_pa < pa)
      {
        pa = buddy_pa;
      }
      order++;
    }
    else
    {
      insert(pa, order);
      area_[order].lock.release();
      return;
    }
  }
}

// Seed the allocator with maximal aligned blocks. A non-power-of-two RAM
// range, its reserved prefix, and an incomplete trailing page are handled
// without rounding usable memory beyond its discovered region.
void BuddyPageAllocator::initialize_layout()
{
  tags_ = layout_.metadata().begin().pointer<int>();
  detail::fill(tags_, 0, layout_.metadata().size());
  for (unsigned order = 0; order <= SuperpageOrder; ++order)
  {
    area_[order].lock.init("buddy");
  }

  // Seed free blocks directly. Unowned RAM is not eagerly touched/poisoned;
  // allocation still fills every returned byte and deallocation still poisons it.
  for (unsigned region = 0; region < layout_.usable_count(); ++region)
  {
    const auto &usable = layout_.usable(region);
    for (PhysicalAddress pa = usable.begin(); pa < usable.end();)
    {
      unsigned order = SuperpageOrder;
      while (!pa.is_aligned(detail::block_size(order)) ||
             !usable.contains_block(pa, detail::block_size(order)))
      {
        --order;
      }
      insert(pa, order);
      pa = pa.advanced(detail::block_size(order));
    }
  }
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
