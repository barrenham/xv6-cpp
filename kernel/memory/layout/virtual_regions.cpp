module;
#include "kernel/arch/riscv/register_constants.h"
module xv.vm_regions;
import xv.memory;

namespace xv::memory
{
VirtualRegion *VirtualRegions::vacant()
{
  for (auto *page = pages_; page; page = page->next)
    for (auto &entry : page->entries)
      if (entry.begin == entry.end) return &entry;
  static_assert(sizeof(Page) <= PageSize);
  auto *page = static_cast<Page *>(allocator().allocate());
  if (!page) return nullptr;
  auto *bytes = reinterpret_cast<unsigned char *>(page);
  for (unsigned long i = 0; i < PageSize; ++i) bytes[i] = 0;
  page->next = pages_; pages_ = page;
  return &page->entries[0];
}
const VirtualRegion *VirtualRegions::find(unsigned long address) const
{
  for (auto *page = pages_; page; page = page->next)
    for (const auto &entry : page->entries)
      if (entry.begin <= address && address < entry.end) return &entry;
  return nullptr;
}
bool VirtualRegions::reserve(unsigned long begin, unsigned long end, unsigned permissions,
                             RegionKind kind, bool demand)
{
  if (begin >= end || end > (1UL << 38) ||
      (permissions & ~(PTE_R | PTE_W | PTE_X | PTE_U)) ||
      ((permissions & PTE_W) && !(permissions & PTE_R)) ||
      (kind == RegionKind::Guard && (permissions || demand))) return false;
  VirtualRegion *adjacent = nullptr;
  for (auto *page = pages_; page; page = page->next)
    for (auto &entry : page->entries)
    {
      if (entry.begin == entry.end) continue;
      if (begin < entry.end && entry.begin < end) return false;
      if (entry.permissions == permissions && entry.kind == kind && entry.demand == demand &&
          (entry.end == begin || entry.begin == end)) adjacent = &entry;
    }
  if (adjacent)
  {
    if (adjacent->end == begin) adjacent->end = end;
    else adjacent->begin = begin;
    return true;
  }
  auto *entry = vacant();
  if (!entry) return false;
  *entry = {begin, end, permissions, kind, demand};
  return true;
}
bool VirtualRegions::erase(unsigned long begin, unsigned long end)
{
  if (begin >= end) return false;
  // At most one non-overlapping region can require a middle split. Obtain its
  // metadata before editing anything, so allocation failure leaves the set intact.
  VirtualRegion *split = nullptr;
  for (auto *page = pages_; page; page = page->next)
    for (auto &entry : page->entries)
      if (entry.begin < begin && end < entry.end) split = &entry;
  if (split)
  {
    auto *tail = vacant();
    if (!tail) return false;
    *tail = *split; tail->begin = end; split->end = begin;
    return true;
  }
  for (auto *page = pages_; page; page = page->next)
    for (auto &entry : page->entries)
    {
      if (entry.begin == entry.end || entry.end <= begin || entry.begin >= end) continue;
      if (begin <= entry.begin && end >= entry.end) entry = {};
      else if (begin <= entry.begin) entry.begin = end;
      else entry.end = begin;
    }
  auto **link = &pages_;
  while (*link)
  {
    auto *page = *link;
    bool empty = true;
    for (const auto &entry : page->entries) if (entry.begin != entry.end) empty = false;
    if (empty) { *link = page->next; allocator().deallocate(page); }
    else link = &page->next;
  }
  return true;
}
bool VirtualRegions::copy_to(VirtualRegions &other) const
{
  VirtualRegions candidate;
  for (auto *page = pages_; page; page = page->next)
    for (const auto &entry : page->entries)
      if (entry.begin != entry.end &&
          !candidate.reserve(entry.begin, entry.end, entry.permissions, entry.kind, entry.demand))
      { candidate.reset(); return false; }
  other.reset(); other.swap(candidate);
  return true;
}
void VirtualRegions::swap(VirtualRegions &other)
{
  auto *page = pages_; pages_ = other.pages_; other.pages_ = page;
}
void VirtualRegions::reset()
{
  while (pages_)
  {
    auto *page = pages_; pages_ = page->next; allocator().deallocate(page);
  }
}
}
