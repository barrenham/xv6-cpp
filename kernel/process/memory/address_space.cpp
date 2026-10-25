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
  size_ = 0;
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
}

void AddressSpace::swap(AddressSpace &other)
{
  auto table = table_;
  table_ = other.table_;
  other.table_ = table;
  auto size = size_;
  size_ = other.size_;
  other.size_ = size;
}

bool AddressSpace::copy_to(AddressSpace &child) const
{
  if (xv::memory::clone_user_pages(table_, child.table_, size_) < 0)
  {
    return false;
  }
  child.size_ = size_;
  return true;
}

int AddressSpace::resize(int bytes, bool eager)
{
  auto next = size_;
  if (bytes > 0)
  {
    if (next + bytes < next || next + bytes > UTOP)
    {
      return -1;
    }
    if (eager)
    {
      next = xv::memory::allocate_user_pages(table_, size_, size_ + bytes, PTE_W);
      if (!next)
      {
        return -1;
      }
    }
    else
    {
      next += bytes;
    }
  }
  else if (bytes < 0)
  {
    next = xv::memory::deallocate_user_pages(table_, size_, size_ + bytes);
  }
  size_ = next;
  return 0;
}

bool AddressSpace::grow_to(unsigned long end, int permissions)
{
  auto next = xv::memory::allocate_user_pages(table_, size_, end, permissions);
  if (!next)
  {
    return false;
  }
  size_ = next;
  return true;
}

void AddressSpace::protect_guard(unsigned long address)
{
  xv::memory::protect_user_guard(table_, address);
}

unsigned long AddressSpace::translate(unsigned long address) const
{
  return xv::memory::translate_user_page(table_, address);
}

unsigned long AddressSpace::handle_fault(unsigned long address, bool read)
{
  return xv::memory::resolve_user_fault(table_, size_, address, read);
}

int AddressSpace::copy_in(void *destination, unsigned long source,
                          unsigned long bytes) const
{
  return xv::memory::copy_from_user(table_, size_, static_cast<char *>(destination),
                                    source, bytes);
}

int AddressSpace::copy_out(unsigned long destination, const void *source,
                           unsigned long bytes) const
{
  return xv::memory::copy_to_user(table_, size_, destination,
                                  static_cast<const char *>(source), bytes);
}

int AddressSpace::copy_string(char *destination, unsigned long source,
                              unsigned long limit) const
{
  return xv::memory::copy_user_string(table_, size_, destination, source, limit);
}
} // namespace xv::process
