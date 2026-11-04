module;

#include "kernel/support/types.h"
#include "kernel/support/flow.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;
import :address_space_flow;

import xv.memory;
import xv.runtime;

namespace xv::process
{
extern "C" char trampoline[];

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
bool AddressSpace::create(void *trap_frame, void *user_info)
{
  CreateFlow::Context context{*this, trap_frame, user_info};
  (void)CreateFlow::Graph::run<0>(context);
  return context.result;
}

// Free a process's page table, and free the
// physical memory it refers to.
void AddressSpace::reset()
{
  if (!table_)
  {
    return;
  }
  unmap_device_pages();
  reset_shared_pages();
  auto pagetable = table_;
  xv::memory::unmap_user_pages(pagetable, TRAMPOLINE, 1, 0);
  xv::memory::unmap_user_pages(pagetable, TRAPFRAME, 1, 0);
#ifdef LAB_PGTBL
  // The page belongs to the proc and survives exec's page-table replacement.
  xv::memory::unmap_user_pages(pagetable, USYSCALL, 1, 0);
#endif
  xv::memory::free_user_table(pagetable, size_);
  table_ = nullptr;
  size_ = 0;
  heap_base_ = 0;
  regions_.reset();
  __atomic_store_n(&observed_size_, size_, __ATOMIC_RELEASE);
}

void AddressSpace::swap(AddressSpace &other)
{
  auto table = table_;
  table_ = other.table_;
  other.table_ = table;
  regions_.swap(other.regions_);
  auto shared = shared_;
  shared_ = other.shared_;
  other.shared_ = shared;
  auto device = device_pages_;
  device_pages_ = other.device_pages_;
  other.device_pages_ = device;
  auto identity = device_identity_;
  device_identity_ = other.device_identity_;
  other.device_identity_ = identity;
  auto heap = heap_base_;
  heap_base_ = other.heap_base_;
  other.heap_base_ = heap;
  auto size = size_;
  size_ = other.size_;
  __atomic_store_n(&observed_size_, size_, __ATOMIC_RELEASE);
  other.size_ = size;
  __atomic_store_n(&other.observed_size_, other.size_, __ATOMIC_RELEASE);
}

bool AddressSpace::copy_to(AddressSpace &child) const
{
  CopyFlow::Context context{*this, child};
  (void)CopyFlow::Graph::run<0>(context);
  if (context.result && device_pages_ &&
      !child.map_device(nullptr, device_pages_, this))
  {
    child.reset();
    return false;
  }
  if (context.result && !copy_shared_pages(child))
  {
    child.reset();
    return false;
  }
  return context.result;
}

int AddressSpace::resize(int bytes, bool eager)
{
  ResizeFlow::Context context{*this, bytes, eager, size_};
  (void)ResizeFlow::Graph::run<0>(context);
  return context.result;
}

bool AddressSpace::grow_to(unsigned long end, int permissions, bool stack)
{
  GrowFlow::Context context{*this, end, permissions, stack};
  (void)GrowFlow::Graph::run<0>(context);
  return context.result;
}

bool AddressSpace::protect_guard(unsigned long address)
{
  GuardFlow::Context context{*this, address};
  (void)GuardFlow::Graph::run<0>(context);
  return context.result;
}

unsigned long AddressSpace::translate(unsigned long address) const
{
  return xv::memory::translate_user_page(table_, address);
}

xv::memory::FaultOutcome AddressSpace::handle_fault(unsigned long address,
                                                    xv::memory::FaultAccess access)
{
  return xv::memory::resolve_user_fault(table_, size_, regions_, address, access);
}

namespace
{
constexpr unsigned long DeviceBase = UTOP - 32UL * 1024 * 1024;
constexpr unsigned DeviceMaxPages = 4096;
} // namespace

unsigned long AddressSpace::map_device_pages(void *const *pages, unsigned count)
{
  if (device_pages_)
  {
    return device_mapping(pages, count) ? DeviceBase : 0;
  }
  return map_device(pages, count, nullptr);
}

bool AddressSpace::device_mapping(void *const *pages, unsigned count) const
{
  if (!pages || !count || count != device_pages_)
  {
    return false;
  }
  return device_identity_ == reinterpret_cast<unsigned long>(pages[0]);
}

bool AddressSpace::unmap_device_pages()
{
  if (!device_pages_)
  {
    return false;
  }
  xv::memory::unmap_user_pages(table_, DeviceBase, device_pages_, 1);
  device_pages_ = 0;
  device_identity_ = 0;
  regions_.release(DeviceBase);
  // The current process owns all page-table edits; fork builds an inactive table.
  // Returning through the trampoline also fences the user page table.
  return true;
}

struct AddressSpace::DeviceMapFlow
{
  struct Context
  {
    AddressSpace &owner;
    void *const *pages;
    unsigned count, index = 0;
    const AddressSpace *inherited;
    unsigned long physical = 0, result = 0;
  };

  static bool reserve(Context &c) noexcept
  {
    if (!c.owner.table_ || !c.count || c.count > DeviceMaxPages ||
        (!c.pages && !c.inherited) || c.owner.device_pages_)
    {
      return false;
    }
    return c.inherited || c.owner.regions_.reserve(
                              DeviceBase, DeviceBase + c.count * PGSIZE,
                              PTE_R | PTE_W | PTE_U, xv::memory::RegionKind::Special);
  }

  static bool map(Context &c) noexcept
  {
    c.physical = c.inherited ? c.inherited->translate(DeviceBase + c.index * PGSIZE)
                             : reinterpret_cast<unsigned long>(c.pages[c.index]);
    return c.physical && !(c.physical % PGSIZE) &&
           xv::memory::map_user_pages(c.owner.table_, DeviceBase + c.index * PGSIZE,
                                      PGSIZE, c.physical, PTE_R | PTE_W | PTE_U) >= 0;
  }

  static void retain(Context &c) noexcept
  {
    xv::memory::allocator().retain_page(reinterpret_cast<void *>(c.physical));
    if (!c.index)
    {
      c.owner.device_identity_ = c.physical;
    }
    c.owner.device_pages_ = ++c.index;
  }

  static bool finished(Context &c) noexcept
  {
    return c.index == c.count;
  }

  static void commit(Context &c) noexcept
  {
    c.result = DeviceBase;
  }

  static void rollback(Context &c) noexcept
  {
    if (!c.owner.unmap_device_pages())
    {
      c.owner.regions_.release(DeviceBase);
    }
  }

  struct Reserve;
  struct Map;
  struct Retain;
  struct Commit;
  struct Rollback;
  using Graph = xv::flow::graph<
      Reserve,
      xv::flow::types<
          xv::flow::node<Reserve, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&reserve>,
                                          xv::flow::go<Map>, xv::flow::stop>>,
          xv::flow::node<
              Map, xv::flow::noop,
              xv::flow::branch<xv::flow::condition<&map>, xv::flow::go<Retain>,
                               xv::flow::go<Rollback>>>,
          xv::flow::node<Retain, xv::flow::action<&retain>,
                         xv::flow::branch<xv::flow::condition<&finished>,
                                          xv::flow::go<Commit>, xv::flow::go<Map>>>,
          xv::flow::node<Commit, xv::flow::action<&commit>, xv::flow::stop>,
          xv::flow::node<Rollback, xv::flow::action<&rollback>, xv::flow::stop>>>;
};

unsigned long AddressSpace::map_device(void *const *pages, unsigned count,
                                       const AddressSpace *inherited)
{
  DeviceMapFlow::Context context{*this, pages, count, 0, inherited};
  (void)DeviceMapFlow::Graph::run<0>(context);
  return context.result;
}

int AddressSpace::copy_in(void *destination, unsigned long source,
                          unsigned long bytes) const
{
  return xv::memory::copy_from_user(table_, size_, regions_,
                                    static_cast<char *>(destination), source, bytes);
}

int AddressSpace::copy_out(unsigned long destination, const void *source,
                           unsigned long bytes) const
{
  return xv::memory::copy_to_user(table_, size_, regions_, destination,
                                  static_cast<const char *>(source), bytes);
}

int AddressSpace::copy_string(char *destination, unsigned long source,
                              unsigned long limit) const
{
  return xv::memory::copy_user_string(table_, size_, regions_, destination, source,
                                      limit);
}
} // namespace xv::process

namespace xv::process
{
namespace
{
constexpr unsigned long SharedBegin = UTOP - 512UL * 1024 * 1024;
constexpr unsigned long SharedEnd = UTOP - 32UL * 1024 * 1024;
constexpr unsigned SharedSlots = 32;
constexpr unsigned SharedMaxPages = 256;
} // namespace

struct AddressSpace::SharedMappings
{
  struct Entry
  {
    unsigned long base = 0;
    unsigned pages = 0, permissions = 0;
    bool private_copy = false;
  } entries[SharedSlots];
};

unsigned long
AddressSpace::map_shared_pages(void *const *pages, unsigned count, unsigned permissions,
                               bool private_copy, unsigned long hint) {
  if (!table_ || !pages || !count || count > SharedMaxPages ||
      (permissions & ~(PTE_R | PTE_W | PTE_U)))
  {
    return 0;
  }
  static_assert(sizeof(SharedMappings) <= PGSIZE);
  if (!shared_)
  {
    auto *page = xv::memory::allocator().allocate();
    if (!page)
    {
      return 0;
    }
    shared_ = new (page) SharedMappings{};
  }
  unsigned slot = 0;
  while (slot < SharedSlots && shared_->entries[slot].pages)
  {
    ++slot;
  }
  if (slot == SharedSlots)
  {
    return 0;
  }
  unsigned long bytes = static_cast<unsigned long>(count) * PGSIZE;
  auto kind =
      permissions ? xv::memory::RegionKind::Special : xv::memory::RegionKind::Guard;
  unsigned long base = hint & ~(PGSIZE - 1);
  if (base < SharedBegin || base >= SharedEnd || bytes > SharedEnd - base ||
      !regions_.reserve(base, base + bytes, permissions, kind))
  {
    base =
        regions_.allocate_in(bytes, SharedBegin, SharedEnd, PGSIZE, permissions, kind);
  }
  if (!base)
  {
    bool empty = true;
    for (const auto &entry : shared_->entries)
    {
      empty &= !entry.pages;
    }
    if (empty)
    {
      xv::memory::allocator().deallocate(shared_);
      shared_ = nullptr;
    }
    return 0;
  }
  unsigned mapped = 0;
  if (permissions)
  {
    for (; mapped < count; ++mapped)
    {
      void *physical = pages[mapped];
      if (private_copy)
      {
        physical = xv::memory::allocator().allocate();
        if (!physical)
        {
          break;
        }
        xv::runtime::move(physical, pages[mapped], PGSIZE);
      }
      if (xv::memory::map_user_pages(table_, base + mapped * PGSIZE, PGSIZE,
                                     reinterpret_cast<unsigned long>(physical),
                                     permissions) < 0)
      {
        if (private_copy)
        {
          xv::memory::allocator().release_page(physical);
        }
        break;
      }
      if (!private_copy)
      {
        xv::memory::allocator().retain_page(physical);
      }
    }
    if (mapped != count)
    {
      if (mapped)
      {
        xv::memory::unmap_user_pages(table_, base, mapped, 1);
      }
      regions_.release(base);
      bool empty = true;
      for (const auto &entry : shared_->entries)
      {
        empty &= !entry.pages;
      }
      if (empty)
      {
        xv::memory::allocator().deallocate(shared_);
        shared_ = nullptr;
      }
      return 0;
    }
  }
  shared_->entries[slot] = {base, count, permissions, private_copy};
  return base;
}

int
AddressSpace::unmap_shared_pages(unsigned long begin, unsigned long bytes) {
  if (!bytes || begin % PGSIZE || begin < SharedBegin || begin >= SharedEnd ||
      bytes > SharedEnd - begin || begin < size_)
  {
    return -22;
  }
  unsigned long end = begin + ((bytes + PGSIZE - 1) & ~(PGSIZE - 1));
  if (end > SharedEnd)
  {
    return -22;
  }
  if (!shared_)
  {
    return 0;
  }
  unsigned free_slot = SharedSlots;
  bool needs_split = false;
  for (unsigned i = 0; i < SharedSlots; ++i)
  {
    const auto &entry = shared_->entries[i];
    if (!entry.pages)
    {
      free_slot = i;
      continue;
    }
    unsigned long limit = entry.base + static_cast<unsigned long>(entry.pages) * PGSIZE;
    needs_split |= entry.base < begin && end < limit;
  }
  if (needs_split && free_slot == SharedSlots)
  {
    return -12;
  }
  if (!regions_.erase(begin, end))
  {
    return -12;
  }
  for (auto &entry : shared_->entries)
  {
    if (!entry.pages)
    {
      continue;
    }
    unsigned long limit = entry.base + static_cast<unsigned long>(entry.pages) * PGSIZE;
    unsigned long low = entry.base > begin ? entry.base : begin;
    unsigned long high = limit < end ? limit : end;
    if (low >= high)
    {
      continue;
    }
    if (entry.permissions)
    {
      xv::memory::unmap_user_pages(table_, low, (high - low) / PGSIZE, 1);
    }
    if (entry.base < low && high < limit)
    {
      shared_->entries[free_slot] = {high, unsigned((limit - high) / PGSIZE),
                                     entry.permissions, entry.private_copy};
      entry.pages = (low - entry.base) / PGSIZE;
    }
    else if (entry.base < low)
    {
      entry.pages = (low - entry.base) / PGSIZE;
    }
    else if (high < limit)
    {
      entry.base = high;
      entry.pages = (limit - high) / PGSIZE;
    }
    else
    {
      entry = {};
    }
  }
  bool empty = true;
  for (const auto &entry : shared_->entries)
  {
    empty &= !entry.pages;
  }
  if (empty)
  {
    xv::memory::allocator().deallocate(shared_);
    shared_ = nullptr;
  }
  return 0;
}

void
AddressSpace::reset_shared_pages() {
  if (!shared_)
  {
    return;
  }
  for (const auto &entry : shared_->entries)
  {
    if (!entry.pages)
    {
      continue;
    }
    if (entry.permissions)
    {
      xv::memory::unmap_user_pages(table_, entry.base, entry.pages, 1);
    }
    regions_.release(entry.base);
  }
  xv::memory::allocator().deallocate(shared_);
  shared_ = nullptr;
}

bool
AddressSpace::copy_shared_pages(AddressSpace &child) const {
  if (!shared_)
  {
    return true;
  }
  auto *page = xv::memory::allocator().allocate();
  if (!page)
  {
    return false;
  }
  child.shared_ = new (page) SharedMappings{};
  for (unsigned i = 0; i < SharedSlots; ++i)
  {
    const auto &entry = shared_->entries[i];
    if (!entry.pages)
    {
      continue;
    }
    auto &out = child.shared_->entries[i];
    out = entry;
    out.pages = 0;
    if (!entry.permissions)
    {
      out.pages = entry.pages;
      continue;
    }
    for (unsigned index = 0; index < entry.pages; ++index)
    {
      auto physical = translate(entry.base + index * PGSIZE);
      if (!physical)
      {
        return false;
      }
      if (entry.private_copy)
      {
        void *copy = xv::memory::allocator().allocate();
        if (!copy)
        {
          return false;
        }
        xv::runtime::move(copy, reinterpret_cast<void *>(physical), PGSIZE);
        physical = reinterpret_cast<unsigned long>(copy);
      }
      if (xv::memory::map_user_pages(child.table_, entry.base + index * PGSIZE, PGSIZE,
                                     physical, entry.permissions) < 0)
      {
        if (entry.private_copy)
        {
          xv::memory::allocator().release_page(reinterpret_cast<void *>(physical));
        }
        return false;
      }
      if (!entry.private_copy)
      {
        xv::memory::allocator().retain_page(reinterpret_cast<void *>(physical));
      }
      ++out.pages;
    }
  }
  return true;
}
} // namespace xv::process
