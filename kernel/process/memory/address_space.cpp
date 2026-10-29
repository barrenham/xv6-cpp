module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;
import :address_space_flow;

import xv.memory;

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
