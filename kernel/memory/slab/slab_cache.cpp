module;
#include "kernel/support/placement_new.hpp"
module xv.memory;
import :slab;

namespace xv::memory
{
bool SlabCache::initialize(PhysicalPageAllocator &pages, unsigned long bytes,
                            unsigned long alignment, unsigned empty_limit)
{
  if (pages_ || !bytes || bytes > 2048 || !alignment || alignment > 2048 ||
      (alignment & (alignment - 1))) return false;
  if (alignment < 8) alignment = 8;
  auto minimum = bytes < 16 ? 16 : bytes;
  auto stride = (minimum + alignment - 1) & ~(alignment - 1);
  auto offset = (sizeof(Page) + alignment - 1) & ~(alignment - 1);
  auto capacity = (PageSize - offset) / stride;
  if (!capacity || capacity > 256) return false;
  lock_.init("slab cache");
  pages_ = &pages;
  stride_ = stride;
  offset_ = offset;
  capacity_ = capacity;
  empty_limit_ = empty_limit;
  stats_.object_bytes = bytes;
  stats_.stride = stride;
  stats_.objects_per_page = capacity;
  return true;
}

void *SlabCache::slot_address(Page &page, unsigned slot) const
{
  return reinterpret_cast<unsigned char *>(&page) + offset_ + slot * stride_;
}
void SlabCache::link_available(Page &page)
{
  page.previous = nullptr;
  page.next = available_;
  if (available_) available_->previous = &page;
  available_ = &page;
}
void SlabCache::unlink_available(Page &page)
{
  if (page.previous) page.previous->next = page.next;
  else available_ = page.next;
  if (page.next) page.next->previous = page.previous;
  page.next = page.previous = nullptr;
}
void SlabCache::unlink_empty(Page &page)
{
  if (page.empty_previous) page.empty_previous->empty_next = page.empty_next;
  else empty_ = page.empty_next;
  if (page.empty_next) page.empty_next->empty_previous = page.empty_previous;
  page.empty_next = page.empty_previous = nullptr;
  --stats_.empty_pages;
}

void *SlabCache::allocate()
{
  if (!pages_) return nullptr;
  xv::sync::ScopedLock guard(lock_);
  ++stats_.allocations;
  if (!available_)
  {
    auto *storage = pages_->allocate();
    if (!storage) { ++stats_.failures; return nullptr; }
    auto *page = new (storage) Page{};
    page->owner = this;
    prepare_page(*page);
    link_available(*page);
    ++stats_.pages;
    ++stats_.page_allocations;
  }
  auto &page = *available_;
  if (!page.used && (page.empty_next || page.empty_previous || empty_ == &page))
    unlink_empty(page);
  unsigned slot = take_slot(page);
  page.live_bits[slot / 64] |= 1UL << (slot % 64);
  ++page.used;
  ++stats_.live;
  if (page.used == capacity_) unlink_available(page);
  return slot_address(page, slot);
}

SlabCache *SlabCache::owner_of(void *object)
{
  if (!object) return nullptr;
  auto *page = reinterpret_cast<Page *>(reinterpret_cast<unsigned long>(object) & ~(PageSize - 1));
  return page->owner;
}

bool SlabCache::deallocate(void *object)
{
  if (!pages_ || !object) return false;
  auto address = reinterpret_cast<unsigned long>(object);
  auto *page = reinterpret_cast<Page *>(address & ~(PageSize - 1));
  xv::sync::ScopedLock guard(lock_);
  if (page->owner != this) return false;
  auto displacement = address - reinterpret_cast<unsigned long>(page);
  if (displacement < offset_ || (displacement - offset_) % stride_) return false;
  unsigned slot = (displacement - offset_) / stride_;
  if (slot >= capacity_ || !(page->live_bits[slot / 64] & (1UL << (slot % 64)))) return false;
  if (page->used == capacity_) link_available(*page);
  page->live_bits[slot / 64] &= ~(1UL << (slot % 64));
  return_slot(*page, slot);
  --stats_.live;
  if (!--page->used)
  {
    if (stats_.empty_pages < empty_limit_)
    {
      page->empty_previous = nullptr;
      page->empty_next = empty_;
      if (empty_) empty_->empty_previous = page;
      empty_ = page;
      ++stats_.empty_pages;
    }
    else
    {
      unlink_available(*page);
      --stats_.pages;
      ++stats_.page_returns;
      pages_->deallocate(page);
    }
  }
  return true;
}

unsigned long SlabCache::trim()
{
  if (!pages_) return 0;
  xv::sync::ScopedLock guard(lock_);
  unsigned long returned = 0;
  while (empty_)
  {
    auto *page = empty_;
    unlink_empty(*page);
    unlink_available(*page);
    --stats_.pages;
    ++stats_.page_returns;
    ++returned;
    pages_->deallocate(page);
  }
  return returned;
}
SlabStatistics SlabCache::statistics()
{
  if (!pages_) return {};
  xv::sync::ScopedLock guard(lock_);
  return stats_;
}
void FreeListSlabCache::prepare_page(Page &page)
{
  for (unsigned i = 0; i < capacity_; ++i)
    *static_cast<unsigned *>(slot_address(page, i)) = i + 1;
}
unsigned FreeListSlabCache::take_slot(Page &page)
{
  auto slot = page.free_head;
  page.free_head = *static_cast<unsigned *>(slot_address(page, slot));
  return slot;
}
void FreeListSlabCache::return_slot(Page &page, unsigned slot)
{
  *static_cast<unsigned *>(slot_address(page, slot)) = page.free_head;
  page.free_head = slot;
}
void BitmapSlabCache::prepare_page(Page &) {}
unsigned BitmapSlabCache::take_slot(Page &page)
{
  for (unsigned word = 0; word < 4; ++word)
  {
    auto free = ~page.live_bits[word];
    if (free)
    {
      auto slot = word * 64 + __builtin_ctzl(free);
      if (slot < capacity_) return slot;
    }
  }
  __builtin_trap(); // available-list invariant: at least one slot exists
}
void BitmapSlabCache::return_slot(Page &, unsigned) {}
} // namespace xv::memory
