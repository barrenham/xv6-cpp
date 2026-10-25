module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

module xv.process;

namespace xv::process
{
extern "C" char trampoline[];

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
bool AddressSpace::create(void *trap_frame, void *user_info)
{
  PageTable pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if (pagetable == 0)
  {
    return false;
  }

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if (mappages(pagetable, TRAMPOLINE, PGSIZE, (uint64)trampoline, PTE_R | PTE_X) < 0)
  {
    uvmfree(pagetable, 0);
    return false;
  }

  // map the trapframe page just below the trampoline page, for
  // trampoline.S.
  if (mappages(pagetable, TRAPFRAME, PGSIZE, reinterpret_cast<uint64>(trap_frame),
               PTE_R | PTE_W) < 0)
  {
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return false;
  }

#ifdef LAB_PGTBL
  if (mappages(pagetable, USYSCALL, PGSIZE, reinterpret_cast<uint64>(user_info),
               PTE_R | PTE_U) < 0)
  {
    uvmunmap(pagetable, TRAPFRAME, 1, 0);
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
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
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
#ifdef LAB_PGTBL
  // The page belongs to the proc and survives exec's page-table replacement.
  uvmunmap(pagetable, USYSCALL, 1, 0);
#endif
  uvmfree(pagetable, size_);
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
  if (uvmcopy(table_, child.table_, size_) < 0)
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
      next = uvmalloc(table_, size_, size_ + bytes, PTE_W);
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
    next = uvmdealloc(table_, size_, size_ + bytes);
  }
  size_ = next;
  return 0;
}

bool AddressSpace::grow_to(unsigned long end, int permissions)
{
  auto next = uvmalloc(table_, size_, end, permissions);
  if (!next)
  {
    return false;
  }
  size_ = next;
  return true;
}

void AddressSpace::protect_guard(unsigned long address)
{
  uvmclear(table_, address);
}

unsigned long AddressSpace::translate(unsigned long address) const
{
  return walkaddr(table_, address);
}

unsigned long AddressSpace::handle_fault(unsigned long address, bool read)
{
  return vmfault(table_, size_, address, read);
}

int AddressSpace::copy_in(void *destination, unsigned long source,
                          unsigned long bytes) const
{
  return copyin(table_, size_, static_cast<char *>(destination), source, bytes);
}

int AddressSpace::copy_out(unsigned long destination, const void *source,
                           unsigned long bytes) const
{
  return copyout(table_, size_, destination, static_cast<const char *>(source), bytes);
}

int AddressSpace::copy_string(char *destination, unsigned long source,
                              unsigned long limit) const
{
  return copyinstr(table_, size_, destination, source, limit);
}
} // namespace xv::process
