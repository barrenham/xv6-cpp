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
import xv.arch;
import xv.runtime;

namespace xv::process
{
extern "C" char trampoline[];

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
bool
AddressSpace::create(void *trap_frame, void *user_info, unsigned frame_slot, bool describe) {
  Access access(*this);
  if (table_ || retiring_ || frame_slot >= THREAD_FRAME_SLOTS)
  {
    return false;
  }
  if (!invalidate_translations())
  {
    return false;
  }
  observed_reclamation_ = 0;
  CreateFlow::Context context{
      *this, trap_frame, user_info,
      static_cast<unsigned long>(THREAD_FRAME_ADDRESS(frame_slot)), describe};
  (void)CreateFlow::Graph::run<0>(context);
  return context.result;
}

// Free a process's page table, and free the
// physical memory it refers to.
void
AddressSpace::reset() {
  Access access(*this);
  observed_reclamation_ = 0;
  if (!table_)
  {
    return;
  }
  __atomic_store_n(&retiring_, true, __ATOMIC_RELEASE);
  if (!invalidate_translations())
  {
    __builtin_trap();
  }
  unmap_device_pages();
  reset_shared_pages();
  auto pagetable = table_;
  xv::memory::unmap_user_pages(pagetable, TRAMPOLINE, 1, 0);
  for (unsigned slot = 0; slot < THREAD_FRAME_SLOTS; ++slot)
  {
    auto address = THREAD_FRAME_ADDRESS(slot);
    auto *entry = xv::memory::walk_entry(pagetable, address, 0);
    if (entry && (*entry & PTE_V))
    {
      xv::memory::unmap_user_pages(pagetable, address, 1, 0);
    }
  }
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
  __atomic_store_n(&retiring_, false, __ATOMIC_RELEASE);
}

bool
AddressSpace::map_thread_frame(unsigned slot, void *frame) {
  Access access(*this);
  if (!invalidate_translations())
  {
    return false;
  }
  auto physical = reinterpret_cast<unsigned long>(frame);
  if (!table_ || retiring_ || !slot || slot >= THREAD_FRAME_SLOTS || !physical ||
      physical % PGSIZE)
  {
    return false;
  }
  auto address = THREAD_FRAME_ADDRESS(slot);
  auto *entry = xv::memory::walk_entry(table_, address, 0);
  if (entry && (*entry & PTE_V))
  {
    return false;
  }
  return xv::memory::map_user_pages(table_, address, PGSIZE, physical, PTE_R | PTE_W) >=
         0;
}

bool
AddressSpace::unmap_thread_frame(unsigned slot, void *frame) {
  Access access(*this);
  if (!invalidate_translations())
  {
    return false;
  }
  if (!table_ || !slot || slot >= THREAD_FRAME_SLOTS)
  {
    return false;
  }
  auto address = THREAD_FRAME_ADDRESS(slot);
  auto *entry = xv::memory::walk_entry(table_, address, 0);
  if (!entry || !(*entry & PTE_V) || (*entry & PTE_U) ||
      PTE2PA(*entry) != reinterpret_cast<unsigned long>(frame))
  {
    return false;
  }
  xv::memory::unmap_user_pages(table_, address, 1, 0);
  return true;
}

void
AddressSpace::swap(AddressSpace &other) {
  if (this == &other)
  {
    return;
  }
  // The other object is an inactive exec image, not a second running process.
  Access access(*this);
  if (!invalidate_translations())
  {
    __builtin_trap();
  }
  Access other_access(other);
  if (!other.invalidate_translations())
  {
    __builtin_trap();
  }
  observed_reclamation_ = other.observed_reclamation_ = 0;
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

bool
AddressSpace::copy_to(AddressSpace &child) const {
  if (this == &child)
  {
    return false;
  }
  // The destination belongs to an unpublished fork child.
  Access access(*this);
  if (!invalidate_translations())
  {
    return false;
  }
  Access child_access(child);
  if (retiring_ || child.retiring_)
  {
    return false;
  }
  if (!child.invalidate_translations())
  {
    return false;
  }
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

int
AddressSpace::resize(int bytes, bool eager, unsigned long *previous) {
  Access access(*this);
  if (retiring_ || !table_)
  {
    return -1;
  }
  if (!invalidate_translations())
  {
    return -1;
  }
  if (retiring_ || !table_)
  {
    return -1;
  }
  auto old_size = size_;
  ResizeFlow::Context context{*this, bytes, eager, size_};
  (void)ResizeFlow::Graph::run<0>(context);
  if (!context.result && previous)
  {
    *previous = old_size;
  }
  return context.result;
}

unsigned long
AddressSpace::page_entry(unsigned long address) const {
  Access access(*this);
  auto *pte = !retiring_ && table_ && address < MAXVA
                  ? xv::memory::walk_entry(table_, address, 0)
                  : nullptr;
  return pte ? __atomic_load_n(pte, __ATOMIC_ACQUIRE) : 0;
}

int
AddressSpace::sample_access(unsigned long base, int pages, unsigned long output) {
  Access access(*this);
  if (retiring_ || !table_ || pages <= 0 || pages > 4096 || base >= MAXVA)
  {
    return -1;
  }
  base = PGROUNDDOWN(base);
  auto count = static_cast<unsigned long>((pages + 7) / 8);
  if (static_cast<unsigned long>(pages) > (MAXVA - base) / PGSIZE || output >= MAXVA ||
      count > MAXVA - output)
  {
    return -1;
  }
  if (!invalidate_translations() || retiring_ || !table_)
  {
    return -1;
  }
  for (int i = 0; i < pages; ++i)
  {
    auto *pte = xv::memory::walk_entry(table_, base + i * PGSIZE, 0);
    if (!pte || (*pte & (PTE_V | PTE_U)) != (PTE_V | PTE_U) || !PTE_LEAF(*pte))
    {
      return -1;
    }
  }
  for (auto at = PGROUNDDOWN(output); at < output + count; at += PGSIZE)
  {
    auto *pte = xv::memory::walk_entry(table_, at, 0);
    if (!pte || (*pte & (PTE_V | PTE_U)) != (PTE_V | PTE_U) ||
        !(*pte & (PTE_W | PTE_COW)))
    {
      return -1;
    }
  }
  for (auto at = PGROUNDDOWN(output); at < output + count; at += PGSIZE)
  {
    auto fault = handle_fault(at, xv::memory::FaultAccess::Write).result;
    if (fault != xv::memory::FaultResult::Resolved &&
        fault != xv::memory::FaultResult::AlreadyMapped)
    {
      return -1;
    }
  }
  // Resolving the output may wait for backing I/O or another hart.
  for (int i = 0; i < pages; ++i)
  {
    auto *pte = xv::memory::walk_entry(table_, base + i * PGSIZE, 0);
    if (!pte || (*pte & (PTE_V | PTE_U)) != (PTE_V | PTE_U) || !PTE_LEAF(*pte))
    {
      return -1;
    }
  }
  if (xv::memory::demote_user_pages(table_, base, pages) < 0)
  {
    return -1;
  }
  unsigned char bits[512]{};
  for (int i = 0; i < pages; ++i)
  {
    auto *pte = xv::memory::walk_entry(table_, base + i * PGSIZE, 0);
    if (*pte & PTE_A)
    {
      bits[i / 8] |= 1U << (i % 8);
    }
  }
  auto generation = generation_;
  if (copy_out(output, bits, count) < 0)
  {
    return -1;
  }
  // A backing callback can drop Access. Never clear bits in a replacement image.
  if (generation != generation_)
  {
    return -1;
  }
  for (int i = 0; i < pages; ++i)
  {
    auto *pte = xv::memory::walk_entry(table_, base + i * PGSIZE, 0);
    __atomic_fetch_and(pte, ~static_cast<unsigned long>(PTE_A), __ATOMIC_ACQ_REL);
  }
  xv::arch::sfence_vma();
  return 0;
}

bool
AddressSpace::grow_to(unsigned long end, int permissions, bool stack) {
  Access access(*this);
  if (retiring_ || !table_)
  {
    return false;
  }
  if (!invalidate_translations())
  {
    return false;
  }
  if (retiring_ || !table_)
  {
    return false;
  }
  GrowFlow::Context context{*this, end, permissions, stack};
  (void)GrowFlow::Graph::run<0>(context);
  return context.result;
}

bool
AddressSpace::protect_guard(unsigned long address) {
  Access access(*this);
  if (retiring_ || !table_)
  {
    return false;
  }
  if (!invalidate_translations())
  {
    return false;
  }
  if (retiring_ || !table_)
  {
    return false;
  }
  GuardFlow::Context context{*this, address};
  (void)GuardFlow::Graph::run<0>(context);
  return context.result;
}

unsigned long
AddressSpace::translate(unsigned long address) const {
  Access access(*this);
  if (retiring_ || !table_)
  {
    return 0;
  }
  return xv::memory::translate_user_page(table_, address);
}

xv::memory::FaultOutcome
AddressSpace::handle_fault(unsigned long address, xv::memory::FaultAccess access) {
  Access operation(*this);
  if (retiring_ || !table_)
  {
    return {xv::memory::FaultResult::InvalidMapping};
  }
  auto *pte =
      address < MAXVA && table_ ? xv::memory::walk_entry(table_, address, 0) : nullptr;
  bool was_file_bound = pte && (*pte & (1UL << 9));
  refresh_file_mappings();
  auto mapped = fault_mapping(address, access);
  if (mapped.result != xv::memory::FaultResult::OutsideRegion)
  {
    return mapped;
  }
  unsigned long permission = access == xv::memory::FaultAccess::Write     ? PTE_W
                             : access == xv::memory::FaultAccess::Execute ? PTE_X
                                                                          : PTE_R;
  pte =
      address < MAXVA && table_ ? xv::memory::walk_entry(table_, address, 0) : nullptr;
  if (was_file_bound && pte &&
      (*pte & (PTE_V | PTE_U | permission)) == (PTE_V | PTE_U | permission))
  {
    return {xv::memory::FaultResult::Resolved, PTE2PA(*pte)};
  }
  auto *region = regions_.find(address);
  if (pte && (*pte & (1UL << 9)) && region &&
      (region->permissions & (PTE_U | permission)) == (PTE_U | permission))
  {
    return {xv::memory::FaultResult::FileBounds};
  }
  if (retiring_ || !table_)
  {
    return {xv::memory::FaultResult::InvalidMapping};
  }
  if (access == xv::memory::FaultAccess::Write)
  {
    if (!prepare_write(address, 1))
    {
      return {xv::memory::FaultResult::InvalidMapping};
    }
  }
  return xv::memory::resolve_user_fault(table_, size_, regions_, address, access);
}

namespace
{
constexpr unsigned long DeviceBase = UTOP - 32UL * 1024 * 1024;
constexpr unsigned DeviceMaxPages = 4096;
} // namespace

unsigned long
AddressSpace::map_device_pages(void *const *pages, unsigned count) {
  Access access(*this);
  if (!invalidate_translations())
  {
    return 0;
  }
  if (retiring_ || !table_)
  {
    return 0;
  }
  if (device_pages_)
  {
    return device_mapping(pages, count) ? DeviceBase : 0;
  }
  return map_device(pages, count, nullptr);
}

bool
AddressSpace::device_mapping(void *const *pages, unsigned count) const {
  Access access(*this);
  if (!pages || !count || count != device_pages_)
  {
    return false;
  }
  return device_identity_ == reinterpret_cast<unsigned long>(pages[0]);
}

bool
AddressSpace::unmap_device_pages() {
  Access access(*this);
  if (!device_pages_)
  {
    return false;
  }
  if (!invalidate_translations())
  {
    return false;
  }
  if (!device_pages_)
  {
    return false;
  }
  xv::memory::unmap_user_pages(table_, DeviceBase, device_pages_, 1);
  device_pages_ = 0;
  device_identity_ = 0;
  regions_.release(DeviceBase);
  // Translation quiescence precedes release of the shared physical references.
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

  static bool
  reserve(Context &c) noexcept {
    if (!c.owner.table_ || !c.count || c.count > DeviceMaxPages ||
        (!c.pages && !c.inherited) || c.owner.device_pages_)
    {
      return false;
    }
    return c.inherited || c.owner.regions_.reserve(
                              DeviceBase, DeviceBase + c.count * PGSIZE,
                              PTE_R | PTE_W | PTE_U, xv::memory::RegionKind::Special);
  }

  static bool
  map(Context &c) noexcept {
    c.physical = c.inherited ? c.inherited->translate(DeviceBase + c.index * PGSIZE)
                             : reinterpret_cast<unsigned long>(c.pages[c.index]);
    return c.physical && !(c.physical % PGSIZE) &&
           xv::memory::map_user_pages(c.owner.table_, DeviceBase + c.index * PGSIZE,
                                      PGSIZE, c.physical, PTE_R | PTE_W | PTE_U) >= 0;
  }

  static void
  retain(Context &c) noexcept {
    xv::memory::allocator().retain_page(reinterpret_cast<void *>(c.physical));
    if (!c.index)
    {
      c.owner.device_identity_ = c.physical;
    }
    c.owner.device_pages_ = ++c.index;
  }

  static bool
  finished(Context &c) noexcept {
    return c.index == c.count;
  }

  static void
  commit(Context &c) noexcept {
    c.result = DeviceBase;
  }

  static void
  rollback(Context &c) noexcept {
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

unsigned long
AddressSpace::map_device(void *const *pages, unsigned count,
                         const AddressSpace *inherited) {
  DeviceMapFlow::Context context{*this, pages, count, 0, inherited};
  (void)DeviceMapFlow::Graph::run<0>(context);
  return context.result;
}

bool
AddressSpace::load_word(unsigned long address, unsigned &value) const {
  if (address % sizeof(unsigned) || address >= MAXVA ||
      sizeof(unsigned) > MAXVA - address)
  {
    return false;
  }
  Access access(*this);
  unsigned prefault = 0;
  if (copy_in(&prefault, address, sizeof(prefault)) < 0)
  {
    return false;
  }
  auto *pte = xv::memory::walk_entry(table_, address, 0);
  if (!pte || (*pte & (PTE_V | PTE_U | PTE_R)) != (PTE_V | PTE_U | PTE_R))
  {
    return false;
  }
  auto physical = xv::memory::translate_user_page(table_, address);
  if (!physical)
  {
    return false;
  }
  auto *word = reinterpret_cast<unsigned *>(physical + address % PGSIZE);
  value = __atomic_load_n(word, __ATOMIC_SEQ_CST);
  return true;
}

int
AddressSpace::copy_in(void *destination, unsigned long source,
                      unsigned long bytes) const {
  Access access(*this);
  refresh_file_mappings();
  if (retiring_ || !table_ ||
      !prepare_mappings(source, bytes, xv::memory::FaultAccess::Read) || retiring_ ||
      !table_)
  {
    return -1;
  }
  return xv::memory::copy_from_user(table_, size_, regions_,
                                    static_cast<char *>(destination), source, bytes);
}

int
AddressSpace::copy_out(unsigned long destination, const void *source,
                       unsigned long bytes) const {
  Access access(*this);
  refresh_file_mappings();
  if (retiring_ || !table_ ||
      !prepare_mappings(destination, bytes, xv::memory::FaultAccess::Write) ||
      retiring_ || !table_)
  {
    return -1;
  }
  if (!prepare_write(destination, bytes))
  {
    return -1;
  }
  return xv::memory::copy_to_user(table_, size_, regions_, destination,
                                  static_cast<const char *>(source), bytes);
}

int
AddressSpace::copy_string(char *destination, unsigned long source,
                          unsigned long limit) const {
  Access access(*this);
  unsigned long done = 0;
  while (done < limit)
  {
    auto at = source + done;
    if (at < source)
    {
      return -1;
    }
    auto take = PGSIZE - at % PGSIZE;
    if (take > limit - done)
    {
      take = limit - done;
    }
    if (copy_in(destination + done, at, take) < 0)
    {
      return -1;
    }
    for (unsigned long i = 0; i < take; ++i)
    {
      if (!destination[done + i])
      {
        return 0;
      }
    }
    done += take;
  }
  return -1;
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
    unsigned pages = 0, maximum_permissions = 0;
    bool private_copy = false;
    xv::memory::MappingBacking *backing = nullptr;
    unsigned backing_offset = 0;
    unsigned long observed_length = ~0UL;
  } entries[SharedSlots];
};

namespace
{
constexpr unsigned long MappingCow = 1UL << 8;
}

xv::memory::FaultOutcome
AddressSpace::fault_mapping(unsigned long address,
                            xv::memory::FaultAccess access) const {
  using xv::memory::FaultResult;
  for (;;)
  {
    if (retiring_ || !table_)
    {
      return {FaultResult::InvalidMapping};
    }
    if (!shared_)
    {
      return {FaultResult::OutsideRegion};
    }
    bool retry = false;
    for (unsigned slot = 0; slot < SharedSlots; ++slot)
    {
      // Never keep an entry/region/PTE pointer across a backend callback.
      auto view = shared_->entries[slot];
      if (!view.pages || address < view.base ||
          address - view.base >= view.pages * PGSIZE)
      {
        continue;
      }
      auto *region = regions_.find(address);
      unsigned permission = access == xv::memory::FaultAccess::Write     ? PTE_W
                            : access == xv::memory::FaultAccess::Execute ? PTE_X
                                                                         : PTE_R;
      if (!region ||
          (region->permissions & (PTE_U | permission)) != (PTE_U | permission))
      {
        return {FaultResult::Permission};
      }
      auto permissions = region->permissions;
      unsigned index = (address - view.base) / PGSIZE;
      if (view.backing && !view.backing->page_available(view.backing_offset + index))
      {
        return {FaultResult::FileBounds};
      }
      auto *pte = xv::memory::walk_entry(table_, address, 1);
      if (!pte)
      {
        return {FaultResult::OutOfMemory};
      }
      if (!PTE2PA(*pte))
      {
        if (!view.backing || !view.backing->demand_paged())
        {
          return {FaultResult::InvalidMapping};
        }
        auto generation = generation_;
        if (view.backing->blocking_load() && xv::sync::interrupt_depth() > 1)
        {
          return {FaultResult::InvalidMapping};
        }
        auto *backing = view.backing->retain();
        bool pinned = backing->loads_pinned();
        void *frame = nullptr;
        int error = 0;
        if (!backing->blocking_load())
        {
          frame = backing->load_page(view.backing_offset + index, error);
          if (frame && !pinned)
          {
            xv::memory::allocator().retain_page(frame);
          }
          backing->release();
        }
        else
        {
          Suspension unlocked(*this);
          frame = backing->load_page(view.backing_offset + index, error);
          if (frame && !pinned)
          {
            xv::memory::allocator().retain_page(frame);
          }
          backing->release();
        }
        if (generation_ != generation || retiring_ || !table_)
        {
          if (frame)
          {
            xv::memory::allocator().release_page(frame);
          }
          retry = true;
          break;
        }
        if (!frame)
        {
          return {error == -12 ? FaultResult::OutOfMemory : FaultResult::FileBounds};
        }
        // EOF can change without a VMA edit while the load was in progress.
        // Refresh that transition without sweeping unrelated revoked PTEs on
        // every completed read; active copies retain their existing leases.
        if (view.backing->length() != shared_->entries[slot].observed_length)
        {
          refresh_file_mappings();
        }
        if (!view.backing->page_available(view.backing_offset + index))
        {
          xv::memory::allocator().release_page(frame);
          return {FaultResult::FileBounds};
        }
        pte = xv::memory::walk_entry(table_, address, 1);
        if (!pte)
        {
          xv::memory::allocator().release_page(frame);
          return {FaultResult::OutOfMemory};
        }
        if (PTE2PA(*pte))
        {
          // Another thread completed the same fault while this load waited.
          xv::memory::allocator().release_page(frame);
        }
        else
        {
          *pte = PA2PTE(reinterpret_cast<unsigned long>(frame)) | permissions | PTE_V;
          observed_reclamation_ = 0;
          if (view.private_copy)
          {
            *pte = (*pte & ~PTE_W) | MappingCow;
          }
        }
      }
      if (access == xv::memory::FaultAccess::Write && (*pte & MappingCow))
      {
        auto generation = generation_;
        if (!invalidate_translations())
        {
          return {FaultResult::InvalidMapping};
        }
        if (generation_ != generation + 1 || retiring_ || !table_)
        {
          retry = true;
          break;
        }
        refresh_file_mappings();
        if (view.backing && !view.backing->page_available(view.backing_offset + index))
        {
          return {FaultResult::FileBounds};
        }
        pte = xv::memory::walk_entry(table_, address, 0);
        if (!pte || !PTE2PA(*pte))
        {
          retry = true;
          break;
        }
        void *old = reinterpret_cast<void *>(PTE2PA(*pte));
        if (xv::memory::allocator().page_references(old) > 1)
        {
          auto *copy = xv::memory::allocator().allocate();
          if (!copy)
          {
            if (!xv::memory::allocator().adopt_revoked_page(old))
            {
              return {FaultResult::OutOfMemory};
            }
          }
          else
          {
            xv::runtime::move(copy, old, PGSIZE);
            *pte = PA2PTE(reinterpret_cast<unsigned long>(copy)) | PTE_FLAGS(*pte);
            xv::memory::allocator().release_page(old);
          }
        }
        else if (xv::memory::allocator().page_revoked(old))
        {
          (void)xv::memory::allocator().adopt_revoked_page(old);
        }
        *pte = (*pte & ~MappingCow) | PTE_W;
      }
      if ((*pte & (PTE_V | PTE_U | permission)) != (PTE_V | PTE_U | permission))
      {
        return {FaultResult::Permission};
      }
      return {FaultResult::Resolved, PTE2PA(*pte)};
    }
    if (!retry)
    {
      return {FaultResult::OutsideRegion};
    }
  }
}

bool
AddressSpace::prepare_mappings(unsigned long address, unsigned long bytes,
                               xv::memory::FaultAccess access) const {
  if (bytes > MAXVA || address > MAXVA - bytes)
  {
    return false;
  }
  for (auto cursor = address; cursor < address + bytes;)
  {
    auto result = fault_mapping(cursor, access).result;
    if (result != xv::memory::FaultResult::OutsideRegion &&
        result != xv::memory::FaultResult::Resolved)
    {
      return false;
    }
    unsigned long next = PGROUNDDOWN(cursor) + PGSIZE;
    cursor = next < address + bytes ? next : address + bytes;
  }
  return true;
}

bool
AddressSpace::prefault_mappings(unsigned long address, unsigned long bytes,
                                xv::memory::FaultAccess access) const {
  Access operation(*this);
  refresh_file_mappings();
  return !retiring_ && table_ && prepare_mappings(address, bytes, access) &&
         !retiring_ && table_;
}

unsigned long
AddressSpace::map_shared_pages(void *const *pages, unsigned count, unsigned permissions,
                               bool private_copy, unsigned long hint,
                               unsigned maximum_permissions,
                               xv::memory::MappingBacking *backing) {
  Access access(*this);
  if (!invalidate_translations())
  {
    return 0;
  }
  bool demand = backing && backing->demand_paged();
  if (!table_ || retiring_ || (!pages && !demand) || !count || count > SharedMaxPages ||
      (permissions & ~(PTE_R | PTE_W | PTE_X | PTE_U)) ||
      (permissions & ~maximum_permissions))
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
  for (; mapped < count; ++mapped)
  {
    if (demand)
    {
      continue;
    }
    // An invalid leaf retains the backing physical frame for PROT_NONE.
    // The attributed virtual region rejects access before any demand fault.
    auto *entry = xv::memory::walk_entry(table_, base + mapped * PGSIZE, 1);
    if (!entry || *entry)
    {
      break;
    }
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
    else
    {
      xv::memory::allocator().retain_page(physical);
    }
    *entry = PA2PTE(reinterpret_cast<unsigned long>(physical)) |
             (permissions ? permissions | PTE_V : 0);
  }
  if (mapped != count)
  {
    for (unsigned i = 0; i < mapped; ++i)
    {
      auto *entry = xv::memory::walk_entry(table_, base + i * PGSIZE, 0);
      xv::memory::allocator().release_page(reinterpret_cast<void *>(PTE2PA(*entry)));
      *entry = 0;
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
  shared_->entries[slot] = {base,
                            count,
                            maximum_permissions,
                            private_copy,
                            backing ? backing->retain() : nullptr,
                            0};
  observed_reclamation_ = 0;
  refresh_file_mappings();
  return base;
}

int
AddressSpace::unmap_shared_pages(unsigned long begin, unsigned long bytes) {
  Access access(*this);
  if (!invalidate_translations())
  {
    return -12;
  }
  if (retiring_ || !table_ || !bytes || begin % PGSIZE || begin < SharedBegin ||
      begin >= SharedEnd || bytes > SharedEnd - begin || begin < size_)
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
  xv::memory::MappingBacking *backings[SharedSlots]{};
  unsigned backing_count = 0;
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
    for (auto address = low; address < high; address += PGSIZE)
    {
      auto *pte = xv::memory::walk_entry(table_, address, 0);
      if (pte && PTE2PA(*pte))
      {
        xv::memory::allocator().release_page(reinterpret_cast<void *>(PTE2PA(*pte)));
        *pte = 0;
      }
    }
    if (entry.base < low && high < limit)
    {
      shared_->entries[free_slot] = {high,
                                     unsigned((limit - high) / PGSIZE),
                                     entry.maximum_permissions,
                                     entry.private_copy,
                                     entry.backing ? entry.backing->retain() : nullptr,
                                     entry.backing_offset +
                                         unsigned((high - entry.base) / PGSIZE),
                                     entry.observed_length};
      entry.pages = (low - entry.base) / PGSIZE;
    }
    else if (entry.base < low)
    {
      entry.pages = (low - entry.base) / PGSIZE;
    }
    else if (high < limit)
    {
      entry.backing_offset += (high - entry.base) / PGSIZE;
      entry.base = high;
      entry.pages = (limit - high) / PGSIZE;
    }
    else
    {
      auto *backing = entry.backing;
      entry = {};
      if (backing)
      {
        backings[backing_count++] = backing;
      }
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
  if (backing_count)
  {
    Suspension unlocked(*this);
    for (unsigned i = 0; i < backing_count; ++i)
    {
      backings[i]->release();
    }
  }
  return 0;
}

void
AddressSpace::reset_shared_pages() {
  if (!shared_)
  {
    return;
  }
  auto *old = shared_;
  shared_ = nullptr;
  xv::memory::MappingBacking *backings[SharedSlots]{};
  unsigned backing_count = 0;
  for (auto &entry : old->entries)
  {
    if (!entry.pages)
    {
      continue;
    }
    for (unsigned i = 0; i < entry.pages; ++i)
    {
      auto *pte = xv::memory::walk_entry(table_, entry.base + i * PGSIZE, 0);
      if (pte && PTE2PA(*pte))
      {
        xv::memory::allocator().release_page(reinterpret_cast<void *>(PTE2PA(*pte)));
        *pte = 0;
      }
    }
    regions_.erase(entry.base,
                   entry.base + static_cast<unsigned long>(entry.pages) * PGSIZE);
    auto *backing = entry.backing;
    entry = {};
    if (backing)
    {
      backings[backing_count++] = backing;
    }
  }
  xv::memory::allocator().deallocate(old);
  if (backing_count)
  {
    Suspension unlocked(*this);
    for (unsigned i = 0; i < backing_count; ++i)
    {
      backings[i]->release();
    }
  }
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
    out.backing = nullptr;
    for (unsigned index = 0; index < entry.pages; ++index)
    {
      auto *source = xv::memory::walk_entry(table_, entry.base + index * PGSIZE, 0);
      auto *target =
          xv::memory::walk_entry(child.table_, entry.base + index * PGSIZE, 1);
      if (!target)
      {
        return false;
      }
      if (source && PTE2PA(*source))
      {
        auto physical = PTE2PA(*source);
        if (entry.private_copy)
        {
          *source = (*source & ~PTE_W) | MappingCow;
        }
        *target = PA2PTE(physical) | PTE_FLAGS(*source);
        xv::memory::allocator().retain_page(reinterpret_cast<void *>(physical));
      }
      if (!out.pages && entry.backing)
      {
        out.backing = entry.backing->retain();
      }
      ++out.pages;
    }
  }
  return true;
}
} // namespace xv::process

namespace xv::process
{
int
AddressSpace::protect_mappings(unsigned long begin, unsigned long bytes,
                               unsigned permissions) {
  Access access(*this);
  if (!invalidate_translations())
  {
    return -11;
  }
  if (retiring_ || !table_ || begin % PGSIZE ||
      (permissions & ~(PTE_R | PTE_W | PTE_X | PTE_U)))
  {
    return -22;
  }
  if (!bytes)
  {
    return 0;
  }
  if (!shared_ || begin < SharedBegin || begin >= SharedEnd ||
      bytes > SharedEnd - begin)
  {
    return -12;
  }
  auto end = begin + PGROUNDUP(bytes);
  // Validate every page and all access ceilings before changing any region/PTE.
  for (auto address = begin; address < end; address += PGSIZE)
  {
    bool found = false;
    for (const auto &entry : shared_->entries)
    {
      if (entry.pages && address >= entry.base &&
          address - entry.base < static_cast<unsigned long>(entry.pages) * PGSIZE)
      {
        if (permissions & ~entry.maximum_permissions)
        {
          return -13;
        }
        found = true;
        break;
      }
    }
    if (!found)
    {
      return -12;
    }
  }
  xv::memory::VirtualAddressPool replacement;
  if (!regions_.copy_to(replacement))
  {
    return -12;
  }
  auto cursor = begin;
  while (cursor < end)
  {
    auto *region = replacement.find(cursor);
    auto high = region->end < end ? region->end : end;
    auto kind =
        permissions ? xv::memory::RegionKind::Special : xv::memory::RegionKind::Guard;
    if (!replacement.protect(cursor, high, permissions, kind))
    {
      replacement.reset();
      return -12;
    }
    cursor = high;
  }
  if (permissions & PTE_W)
  {
    for (unsigned i = 0; i < SharedSlots; ++i)
    {
      auto entry = shared_->entries[i];
      if (!entry.pages || !entry.backing || entry.private_copy || entry.base >= end ||
          entry.base + entry.pages * PGSIZE <= begin)
      {
        continue;
      }
      auto generation = generation_;
      auto *backing = entry.backing->retain();
      int error;
      {
        Suspension unlocked(*this);
        error = backing->enable_write();
        backing->release();
      }
      if (generation_ != generation || retiring_ || !table_)
      {
        replacement.reset();
        return -11;
      }
      if (error)
      {
        replacement.reset();
        return error;
      }
    }
  }
  auto generation = generation_;
  if (!invalidate_translations() || generation_ != generation + 1 || retiring_ ||
      !table_)
  {
    replacement.reset();
    return -11;
  }
  regions_.swap(replacement);
  replacement.reset();
  for (auto address = begin; address < end; address += PGSIZE)
  {
    auto *pte = xv::memory::walk_entry(table_, address, 0);
    if (!pte || !PTE2PA(*pte))
    {
      continue;
    }
    auto flags = permissions ? permissions | PTE_V : 0;
    if (*pte & MappingCow)
    {
      flags = (flags & ~PTE_W) | MappingCow;
    }
    *pte = (*pte & ~1023UL) | (*pte & (1UL << 9)) | flags;
  }
  refresh_file_mappings();
  // User return reacquires this operation lock and fences the hart's TLB.
  return 0;
}
} // namespace xv::process

namespace xv::process
{
unsigned
AddressSpace::reclaim_revoked_mappings() const {
  auto epoch = xv::memory::page_reclamation_epoch();
  if (observed_reclamation_ == epoch)
  {
    return 0;
  }
  observed_reclamation_ = epoch;
  if (!shared_)
  {
    return 0;
  }
  unsigned dropped = 0;
  for (const auto &view : shared_->entries)
  {
    if (!view.pages || !view.backing)
    {
      continue;
    }
    for (unsigned index = 0; index < view.pages; ++index)
    {
      auto *pte = xv::memory::walk_entry(table_, view.base + index * PGSIZE, 0);
      if (!pte || !PTE2PA(*pte))
      {
        continue;
      }
      auto *frame = reinterpret_cast<void *>(PTE2PA(*pte));
      if (!xv::memory::allocator().page_revoked(frame))
      {
        continue;
      }
      *pte = 0;
      xv::memory::allocator().release_page(frame);
      ++dropped;
    }
  }
  return dropped;
}

unsigned
AddressSpace::reclaim_inactive_mappings() const {
  // Other execution contexts can own this same address space. Never wait with
  // a process spin lock held, even when the selected context is not runnable.
  if (__atomic_load_n(&active_accesses_, __ATOMIC_SEQ_CST) ||
      !operation_lock_.try_acquire())
  {
    return 0;
  }
  auto dropped = __atomic_load_n(&active_accesses_, __ATOMIC_SEQ_CST)
                     ? 0
                     : reclaim_revoked_mappings();
  operation_lock_.release();
  return dropped;
}

void
AddressSpace::refresh_file_mappings() const {
  Access access(*this);
  if (retiring_ || !table_)
  {
    return;
  }
  reclaim_revoked_mappings();
  if (!shared_)
  {
    return;
  }
  constexpr unsigned long FileBounds = 1UL << 9;
  for (auto &entry : shared_->entries)
  {
    if (!entry.pages || !entry.backing)
    {
      continue;
    }
    auto length = entry.backing->length();
    bool shrunk = entry.observed_length != ~0UL && length < entry.observed_length;
    entry.observed_length = length;
    for (unsigned index = 0; index < entry.pages; ++index)
    {
      auto address = entry.base + index * PGSIZE;
      auto *pte = xv::memory::walk_entry(table_, address, 0);
      if (!pte || !PTE2PA(*pte))
      {
        continue;
      }
      unsigned long page_start = (entry.backing_offset + index) * PGSIZE;
      if (entry.private_copy && shrunk && length < page_start + PGSIZE)
      {
        auto bytes = length > page_start ? length - page_start : 0;
        xv::runtime::fill(reinterpret_cast<char *>(PTE2PA(*pte)) + bytes, 0,
                          PGSIZE - bytes);
      }
      bool available = entry.backing->page_available(entry.backing_offset + index);
      if (!available)
      {
        *pte = (*pte & ~1023UL) | FileBounds;
      }
      else if (*pte & FileBounds)
      {
        if (entry.private_copy)
        {
          // Re-fault after growth; do not write through a COW-shared frame.
          xv::memory::allocator().release_page(reinterpret_cast<void *>(PTE2PA(*pte)));
          *pte = 0;
          continue;
        }
        auto permissions = regions_.find(address)->permissions;
        *pte = (*pte & ~1023UL) | (permissions ? permissions | PTE_V : 0);
      }
    }
  }
}

int
AddressSpace::sync_mappings(unsigned long begin, unsigned long bytes) {
  Access access(*this);
  if (retiring_ || !table_ || begin % PGSIZE || !bytes || begin < SharedBegin ||
      begin >= SharedEnd || bytes > SharedEnd - begin)
  {
    return -22;
  }
  auto end = begin + PGROUNDUP(bytes);
  for (auto address = begin; address < end; address += PGSIZE)
  {
    bool found = false;
    if (shared_)
    {
      for (const auto &entry : shared_->entries)
      {
        if (entry.pages && address >= entry.base &&
            address - entry.base < entry.pages * PGSIZE)
        {
          found = true;
          break;
        }
      }
    }
    if (!found)
    {
      return -12;
    }
  }
  xv::memory::MappingBacking *backings[SharedSlots]{};
  unsigned count = 0;
  if (shared_)
  {
    for (const auto &entry : shared_->entries)
    {
      if (!entry.pages || !entry.backing || entry.private_copy || entry.base >= end ||
          entry.base + entry.pages * PGSIZE <= begin)
      {
        continue;
      }
      bool duplicate = false;
      for (unsigned j = 0; j < count; ++j)
      {
        duplicate |= backings[j] == entry.backing;
      }
      if (!duplicate)
      {
        backings[count++] = entry.backing->retain();
      }
    }
  }
  int result = 0;
  if (count)
  {
    Suspension unlocked(*this);
    for (unsigned i = 0; i < count; ++i)
    {
      if (!result)
      {
        result = backings[i]->synchronize();
      }
      backings[i]->release();
    }
  }
  return result;
}

void
AddressSpace::close_file_mappings() {
  Access access(*this);
  for (;;)
  {
    unsigned long base = 0, bytes = 0;
    if (shared_)
    {
      for (const auto &entry : shared_->entries)
      {
        if (entry.pages && entry.backing)
        {
          base = entry.base;
          bytes = entry.pages * PGSIZE;
          break;
        }
      }
    }
    if (!base)
    {
      return;
    }
    if (unmap_shared_pages(base, bytes) < 0)
    {
      // Exact mapping boundaries never need new interval metadata.
      __builtin_trap();
    }
  }
}
} // namespace xv::process
