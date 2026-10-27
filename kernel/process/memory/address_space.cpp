module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;

import xv.memory;

namespace xv::process
{
extern "C" char trampoline[];

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
bool AddressSpace::create(void *trap_frame, void *user_info)
{
  PageTable pagetable;

  // An empty page table.
  pagetable = xv::memory::create_user_table();
  if (pagetable == 0)
  {
    return false;
  }

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if (xv::memory::map_user_pages(pagetable, TRAMPOLINE, PGSIZE, (uint64)trampoline,
                                 PTE_R | PTE_X) < 0)
  {
    xv::memory::free_user_table(pagetable, 0);
    return false;
  }

  // map the trapframe page just below the trampoline page, for
  // trampoline.S.
  if (xv::memory::map_user_pages(pagetable, TRAPFRAME, PGSIZE,
                                 reinterpret_cast<uint64>(trap_frame),
                                 PTE_R | PTE_W) < 0)
  {
    xv::memory::unmap_user_pages(pagetable, TRAMPOLINE, 1, 0);
    xv::memory::free_user_table(pagetable, 0);
    return false;
  }

#ifdef LAB_PGTBL
  if (xv::memory::map_user_pages(pagetable, USYSCALL, PGSIZE,
                                 reinterpret_cast<uint64>(user_info),
                                 PTE_R | PTE_U) < 0)
  {
    xv::memory::unmap_user_pages(pagetable, TRAPFRAME, 1, 0);
    xv::memory::unmap_user_pages(pagetable, TRAMPOLINE, 1, 0);
    xv::memory::free_user_table(pagetable, 0);
    return false;
  }
#endif

  table_ = pagetable;
  if (!regions_.reserve(TRAMPOLINE, TRAMPOLINE + PGSIZE, PTE_R | PTE_X, xv::memory::RegionKind::Special) ||
      !regions_.reserve(TRAPFRAME, TRAPFRAME + PGSIZE, PTE_R | PTE_W, xv::memory::RegionKind::Special)
#ifdef LAB_PGTBL
      || !regions_.reserve(USYSCALL, USYSCALL + PGSIZE, PTE_R | PTE_U, xv::memory::RegionKind::Special)
#endif
     ) { reset(); return false; }
  size_ = 0;
  __atomic_store_n(&observed_size_, size_, __ATOMIC_RELEASE);
  return true;
}

// Free a process's page table, and free the
// physical memory it refers to.
void AddressSpace::reset()
{
  if (!table_)
  {
    return;
  }
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
  auto heap = heap_base_; heap_base_ = other.heap_base_; other.heap_base_ = heap;
  auto size = size_;
  size_ = other.size_;
  __atomic_store_n(&observed_size_, size_, __ATOMIC_RELEASE);
  other.size_ = size;
  __atomic_store_n(&other.observed_size_, other.size_, __ATOMIC_RELEASE);
}

bool AddressSpace::copy_to(AddressSpace &child) const
{
  if (!regions_.copy_to(child.regions_)) return false;
  if (xv::memory::clone_user_pages(table_, child.table_, size_) < 0)
  {
    return false;
  }
  child.size_ = size_;
  child.heap_base_ = heap_base_;
  __atomic_store_n(&child.observed_size_, child.size_, __ATOMIC_RELEASE);
  return true;
}

int AddressSpace::resize(int bytes, bool eager)
{
  auto next = size_;
  if (bytes > 0)
  {
    if (static_cast<unsigned long>(bytes) > UTOP - next) return -1;
    next += bytes;
    auto begin = PGROUNDUP(size_), end = PGROUNDUP(next);
    if (begin != end && !regions_.reserve(begin, end, PTE_R | PTE_W | PTE_U,
                                          xv::memory::RegionKind::Heap, true)) return -1;
    if (eager && !xv::memory::allocate_user_pages(table_, size_, next, PTE_W))
    { if (begin != end) regions_.erase(begin, end); return -1; }
  }
  else if (bytes < 0)
  {
    auto shrink = static_cast<unsigned long>(-static_cast<long>(bytes));
    if (shrink > size_ || size_ - shrink < heap_base_) return -1;
    next -= shrink;
    auto begin = PGROUNDUP(next), end = PGROUNDUP(size_);
    if (begin != end && !regions_.erase(begin, end)) return -1;
    xv::memory::deallocate_user_pages(table_, size_, next);
  }
  size_ = next;
  __atomic_store_n(&observed_size_, size_, __ATOMIC_RELEASE);
  return 0;
}

bool AddressSpace::grow_to(unsigned long end, int permissions, bool stack)
{
  if (end < size_ || end > UTOP) return false;
  auto begin = PGROUNDUP(size_), limit = PGROUNDUP(end);
  if (begin != limit && !regions_.reserve(begin, limit, permissions | PTE_R | PTE_U,
      stack ? xv::memory::RegionKind::UserStack : xv::memory::RegionKind::Image)) return false;
  auto next = xv::memory::allocate_user_pages(table_, size_, end, permissions);
  if (!next)
  { if (begin != limit) regions_.erase(begin, limit); return false; }
  size_ = next; heap_base_ = next;
  __atomic_store_n(&observed_size_, size_, __ATOMIC_RELEASE);
  return true;
}

bool AddressSpace::protect_guard(unsigned long address)
{
  if (!regions_.erase(address, address + PGSIZE) ||
      !regions_.reserve(address, address + PGSIZE, 0, xv::memory::RegionKind::Guard)) return false;
  xv::memory::protect_user_guard(table_, address);
  return true;
}

unsigned long AddressSpace::translate(unsigned long address) const
{
  return xv::memory::translate_user_page(table_, address);
}

xv::memory::FaultOutcome AddressSpace::handle_fault(unsigned long address, xv::memory::FaultAccess access)
{
  return xv::memory::resolve_user_fault(table_, size_, regions_, address, access);
}

int AddressSpace::copy_in(void *destination, unsigned long source,
                          unsigned long bytes) const
{
  return xv::memory::copy_from_user(table_, size_, regions_, static_cast<char *>(destination),
                                    source, bytes);
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
  return xv::memory::copy_user_string(table_, size_, regions_, destination, source, limit);
}
} // namespace xv::process
