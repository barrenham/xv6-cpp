module;

#include "kernel/support/param.h"
#include "kernel/support/types.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/fs/format/layout.h"

module xv.memory;

import xv.runtime;
import xv.arch;
import xv.print;

namespace xv::memory
{

// Return the address of the PTE in page table pagetable
// that corresponds to virtual address va.  If alloc!=0,
// create any required page-table pages.
//
// The risc-v Sv39 scheme has three levels of page-table
// pages. A page-table page contains 512 64-bit PTEs.
// A 64-bit virtual address is split into five fields:
//   39..63 -- must be zero.
//   30..38 -- 9 bits of level-2 index.
//   21..29 -- 9 bits of level-1 index.
//   12..20 -- 9 bits of level-0 index.
//    0..11 -- 12 bits of byte offset within the page.
static pte_t *walkto(pagetable_t pagetable, uint64 va, int alloc, int target,
                     int *leaflevel)
{
  if (va >= MAXVA)
  {
    xv::print::panic("walk_entry");
  }

  for (int level = 2; level > target; level--)
  {
    pte_t *pte = &pagetable[PX(level, va)];
    if (*pte & PTE_V)
    {
      if (*pte & (PTE_R | PTE_W | PTE_X))
      {
        if (leaflevel)
        {
          *leaflevel = level;
        }
        return pte;
      }
      pagetable = (pagetable_t)PTE2PA(*pte);
    }
    else
    {
      if (!alloc || (pagetable = (pde_t *)xv::memory::allocator().allocate()) == 0)
      {
        return 0;
      }
      xv::runtime::fill(pagetable, 0, PGSIZE);
      *pte = PA2PTE(pagetable) | PTE_V;
    }
  }
  if (leaflevel)
  {
    *leaflevel = target;
  }
  return &pagetable[PX(target, va)];
}

pte_t *walk_entry(pagetable_t pagetable, uint64 va, int alloc)
{
  return walkto(pagetable, va, alloc, 0, 0);
}

// Look up a virtual address, return the physical address,
// or 0 if not mapped.
// Can only be used to look up user pages.
uint64 translate_user_page(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  uint64 pa;
  int level;

  if (va >= MAXVA)
  {
    return 0;
  }

  pte = walkto(pagetable, va, 0, 0, &level);
  if (pte == 0)
  {
    return 0;
  }
  if ((*pte & PTE_V) == 0)
  {
    return 0;
  }
  if ((*pte & PTE_U) == 0)
  {
    return 0;
  }
  // copy_from_user/out copy one 4 KiB slice at a time, even within a superpage.
  pa = PTE2PA(*pte) + (PGROUNDDOWN(va) & ((1L << PXSHIFT(level)) - 1));
  return pa;
}

#if defined(LAB_PGTBL) || defined(SOL_MMAP) || defined(SOL_COW)
static void vmprint_level(pagetable_t pagetable, int level, uint64 base)
{
  for (int i = 0; i < 512; i++)
  {
    pte_t pte = pagetable[i];
    if ((pte & PTE_V) == 0)
    {
      continue;
    }
    uint64 va = base | ((uint64)i << PXSHIFT(level));
    for (int depth = 2; depth >= level; depth--)
    {
      xv::print::printk(" ..");
    }
    xv::print::printk("%p: pte %p pa %p", (void *)va, (void *)pte, (void *)PTE2PA(pte));
    if (PTE_LEAF(pte))
    {
      xv::print::printk(" %s%s%s%s", (pte & PTE_R) ? "R" : "", (pte & PTE_W) ? "W" : "",
                        (pte & PTE_X) ? "X" : "", (pte & PTE_U) ? "U" : "");
      if (level > 0)
      {
        xv::print::printk(" superpage");
      }
    }
    xv::print::printk("\n");
    if (level > 0 && !PTE_LEAF(pte))
    {
      vmprint_level((pagetable_t)PTE2PA(pte), level - 1, va);
    }
  }
}

void print_page_table(pagetable_t pagetable)
{
  xv::print::printk("page table %p\n", pagetable);
  vmprint_level(pagetable, 2, 0);
}
#endif

#ifdef LAB_PGTBL
static int supermap(pagetable_t pagetable, uint64 va, uint64 pa, int perm)
{
  int level;
  if ((va | pa) % SUPERPGSIZE != 0)
  {
    xv::print::panic("supermap: alignment");
  }
  pte_t *pte = walkto(pagetable, va, 1, 1, &level);
  if (pte == 0)
  {
    return -1;
  }
  if (level != 1)
  {
    xv::print::panic("supermap: level");
  }
  if (*pte & PTE_V)
  {
    if (PTE_LEAF(*pte))
    {
      xv::print::panic("supermap: remap");
    }
    // A previous shrink may have left an empty level-0 page table.
    pagetable_t child = (pagetable_t)PTE2PA(*pte);
    for (int i = 0; i < 512; i++)
    {
      if (child[i] & PTE_V)
      {
        xv::print::panic("supermap: occupied");
      }
    }
    xv::memory::allocator().deallocate(child);
  }
  *pte = PA2PTE(pa) | perm | PTE_V;
  return 0;
}

// Split without copying data. Under memory pressure, use a page that this
// unmap is about to discard as the new page table instead of failing shrink.
static int superdemote(pte_t *pte, uint64 va, int do_free)
{
  uint64 pa = PTE2PA(*pte);
  uint flags = PTE_FLAGS(*pte);
  pagetable_t child = (pagetable_t)xv::memory::allocator().allocate();
  int recycled = -1;
  if (child == 0)
  {
    if (!do_free)
    {
      return -1;
    }
    recycled = PX(0, va);
    child = (pagetable_t)(pa + (uint64)recycled * PGSIZE);
  }
  if (!xv::memory::allocator().split_to_pages((void *)pa, xv::memory::SuperpageOrder))
  {
    xv::print::panic("unsupported page split");
  }
  for (int i = 0; i < 512; i++)
  {
    child[i] = i == recycled ? 0 : PA2PTE(pa + (uint64)i * PGSIZE) | flags;
  }
  *pte = PA2PTE(child) | PTE_V;
  return 0;
}

// pgaccess needs independent access bits for each 4 KiB page. A superpage
// has only one A bit, so split any covered superpages before sampling.
int demote_user_pages(pagetable_t pagetable, uint64 va, uint64 npages)
{
  if (va % PGSIZE != 0 || va >= MAXVA || npages > (MAXVA - va) / PGSIZE)
  {
    return -1;
  }
  uint64 end = va + npages * PGSIZE;
  for (uint64 a = va; a < end; a = SUPERPGROUNDDOWN(a) + SUPERPGSIZE)
  {
    int level;
    pte_t *pte = walkto(pagetable, a, 0, 0, &level);
    if (pte == 0 || (*pte & PTE_V) == 0)
    {
      return -1;
    }
    if (level == 1 && superdemote(pte, a, 0) < 0)
    {
      return -1;
    }
  }
  return 0;
}
#endif

// The mapping policy can be reused with a 4 KiB-only maximum in other contexts.
// Here it selects the largest legal Sv39 leaf without crossing permissions.
void map_kernel_range(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
{
  xv::memory::LargestPagePolicy policy;
  xv::memory::Sv39PageTable table(kpgtbl, xv::memory::allocator(), policy);
  if (table.map_range(va, xv::memory::PhysicalAddress(pa), sz, perm) !=
      xv::memory::MappingResult::Ok)
  {
    xv::print::panic("map_kernel_range");
  }
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa.
// va and size MUST be page-aligned.
// Returns 0 on success, -1 if walk_entry() couldn't
// allocate a needed page-table page.

int map_user_pages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
  uint64 a, last;
  pte_t *pte;

  if ((va % PGSIZE) != 0)
  {
    xv::print::panic("map_user_pages: va not aligned");
  }

  if ((size % PGSIZE) != 0)
  {
    xv::print::panic("map_user_pages: size not aligned");
  }

  if (size == 0)
  {
    xv::print::panic("map_user_pages: size");
  }

  a = va;
  last = va + size - PGSIZE;
  for (;;)
  {
    if ((pte = walk_entry(pagetable, a, 1)) == 0)
    {
      return -1;
    }
    if (*pte & PTE_V)
    {
      xv::print::panic("map_user_pages: remap");
    }
    *pte = PA2PTE(pa) | perm | PTE_V;
    if (a == last)
    {
      break;
    }
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

// create an empty user page table.
// returns 0 if out of memory.
pagetable_t create_user_table()
{
  pagetable_t pagetable;
  pagetable = (pagetable_t)xv::memory::allocator().allocate();
  if (pagetable == 0)
  {
    return 0;
  }
  xv::runtime::fill(pagetable, 0, PGSIZE);
  return pagetable;
}

// Remove npages of mappings starting from va. va must be
// page-aligned. It's OK if the mappings don't exist.
// Optionally free the physical memory.
void unmap_user_pages(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
{
  uint64 a;
  pte_t *pte;
  int sz = PGSIZE;
  int level;

  if ((va % PGSIZE) != 0)
  {
    xv::print::panic("unmap_user_pages: not aligned");
  }

  for (a = va; a < va + npages * PGSIZE; a += sz)
  {
    sz = PGSIZE;
    if ((pte = walkto(pagetable, a, 0, 0, &level)) == 0)
    {
      continue;
    }
    if ((*pte & PTE_V) == 0) // has physical page been allocated?
    {
      continue;
    }
#ifdef LAB_PGTBL
    if (level == 1)
    {
      if (a % SUPERPGSIZE == 0 && va + npages * PGSIZE - a >= SUPERPGSIZE)
      {
        if (do_free)
        {
          xv::memory::allocator().deallocate((void *)PTE2PA(*pte),
                                             xv::memory::SuperpageOrder);
        }
        *pte = 0;
        sz = SUPERPGSIZE;
        continue;
      }
      if (superdemote(pte, a, do_free) < 0)
      {
        xv::print::panic("unmap_user_pages: split");
      }
      pte = walk_entry(pagetable, a, 0);
      if ((*pte & PTE_V) == 0) // recycled as the level-0 page table
      {
        continue;
      }
    }
#endif
    if (PTE_FLAGS(*pte) == PTE_V)
    {
      xv::print::panic("unmap_user_pages: not a leaf");
    }
    if (do_free)
    {
      uint64 pa = PTE2PA(*pte);
      xv::memory::allocator().release_page((void *)pa);
    }
    *pte = 0;
  }
}

// Allocate PTEs and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
uint64 allocate_user_pages(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm)
{
  char *mem;
  uint64 a;
  int sz;

  if (newsz < oldsz)
  {
    return oldsz;
  }
#ifdef LAB_PGTBL
  if (newsz > UTOP)
  {
    return 0;
  }
#endif

  oldsz = PGROUNDUP(oldsz);
  for (a = oldsz; a < newsz; a += sz)
  {
    sz = PGSIZE;
#ifdef LAB_PGTBL
    if (a % SUPERPGSIZE == 0 && newsz - a >= SUPERPGSIZE &&
        (mem = static_cast<char *>(
             xv::memory::allocator().allocate(xv::memory::SuperpageOrder))) != 0)
    {
      xv::runtime::fill(mem, 0, SUPERPGSIZE);
      if (supermap(pagetable, a, (uint64)mem, PTE_R | PTE_U | xperm) < 0)
      {
        xv::memory::allocator().deallocate(mem, xv::memory::SuperpageOrder);
        deallocate_user_pages(pagetable, a, oldsz);
        return 0;
      }
      sz = SUPERPGSIZE;
      continue;
    }
#endif
    mem = static_cast<char *>(xv::memory::allocator().allocate());
    if (mem == 0)
    {
      deallocate_user_pages(pagetable, a, oldsz);
      return 0;
    }
#ifndef LAB_SYSCALL
    xv::runtime::fill(mem, 0, sz);
#endif
    if (map_user_pages(pagetable, a, sz, (uint64)mem, PTE_R | PTE_U | xperm) != 0)
    {
      xv::memory::allocator().deallocate(mem);
      deallocate_user_pages(pagetable, a, oldsz);
      return 0;
    }
  }
  return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
uint64 deallocate_user_pages(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
{
  if (newsz >= oldsz)
  {
    return oldsz;
  }

  if (PGROUNDUP(newsz) < PGROUNDUP(oldsz))
  {
    int npages = (PGROUNDUP(oldsz) - PGROUNDUP(newsz)) / PGSIZE;
    unmap_user_pages(pagetable, PGROUNDUP(newsz), npages, 1);
  }

  return newsz;
}

// Recursively free page-table pages.
// All leaf mappings must already have been removed.
static void freewalk(pagetable_t pagetable)
{
  // there are 2^9 = 512 PTEs in a page table.
  for (int i = 0; i < 512; i++)
  {
    pte_t pte = pagetable[i];
    if ((pte & PTE_V) && (pte & (PTE_R | PTE_W | PTE_X)) == 0)
    {
      // this PTE points to a lower-level page table.
      uint64 child = PTE2PA(pte);
      freewalk((pagetable_t)child);
      pagetable[i] = 0;
    }
    else if (pte & PTE_V)
    {
      // backtrace();
      xv::print::panic("freewalk: leaf");
    }
  }
  xv::memory::allocator().deallocate((void *)pagetable);
}

// Free user memory pages,
// then free page-table pages.
void free_user_table(pagetable_t pagetable, uint64 sz)
{
  if (sz > 0)
  {
    unmap_user_pages(pagetable, 0, PGROUNDUP(sz) / PGSIZE, 1);
  }
  freewalk(pagetable);
}

// Given a parent process's page table, copy
// its memory into a child's page table.
// Copies both the page table and the
// physical memory.
// returns 0 on success, -1 on failure.
// frees any allocated pages on failure.
int clone_user_pages(pagetable_t old, pagetable_t new_table, uint64 sz)
{
  uint64 address = 0;
  for (; address < sz; address += PGSIZE)
  {
    int level;
    auto *pte = walkto(old, address, 0, 0, &level);
    if (!pte || !(*pte & PTE_V)) continue;
#ifdef LAB_PGTBL
    // Preserve 2 MiB allocation until fork, then share independent 4 KiB frames.
    if (level == 1)
    {
      if (superdemote(pte, address, 0) < 0) goto failure;
      pte = walk_entry(old, address, 0);
      level = 0;
    }
#endif
    if (level != 0) goto failure;
    auto physical = PTE2PA(*pte);
    auto flags = PTE_FLAGS(*pte);
    if ((flags & (PTE_U | PTE_W)) == (PTE_U | PTE_W)) flags = (flags & ~PTE_W) | PTE_COW;
    if (map_user_pages(new_table, address, PGSIZE, physical, flags) < 0) goto failure;
    allocator().retain_page(reinterpret_cast<void *>(physical));
    *pte = PA2PTE(physical) | flags;
  }
  // The calling process is not concurrently executing on another hart. Each
  // later user return switches satp and fences before reusing its translations.
  xv::arch::sfence_vma();
  return 0;
failure:
  unmap_user_pages(new_table, 0, address / PGSIZE, 1);
  xv::arch::sfence_vma();
  return -1;
}

// mark a PTE invalid for user access.
// used by exec for the user stack guard page.
void protect_user_guard(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;

  pte = walk_entry(pagetable, va, 0);
  if (pte == 0)
  {
    xv::print::panic("protect_user_guard");
  }
  *pte &= ~PTE_U;
}

namespace
{
unsigned permission_for(FaultAccess access)
{
  return access == FaultAccess::Write ? PTE_W : access == FaultAccess::Execute ? PTE_X : PTE_R;
}
const VirtualRegion *accessible_region(const VirtualRegions &regions, uint64 size, uint64 address,
                                      FaultAccess access)
{
  auto *region = regions.find(address);
  auto permissions = permission_for(access) | PTE_U;
  if (!region || (region->permissions & permissions) != permissions ||
      (address >= size && region->kind != RegionKind::Special)) return nullptr;
  return region;
}
}

FaultOutcome resolve_user_fault(pagetable_t table, uint64 size, const VirtualRegions &regions,
                                uint64 address, FaultAccess access)
{
  if (address >= MAXVA) return {FaultResult::OutsideRegion};
  auto *region = regions.find(address);
  if (!region || (address >= size && region->kind != RegionKind::Special))
    return {FaultResult::OutsideRegion};
  if (!accessible_region(regions, size, address, access)) return {FaultResult::Permission};
  int level;
  auto *pte = walkto(table, address, 0, 0, &level);
  if (pte && (*pte & PTE_V))
  {
    if (!PTE_LEAF(*pte)) return {FaultResult::InvalidMapping};
    if (!(*pte & PTE_U)) return {FaultResult::Permission};
    if (access != FaultAccess::Write || !(*pte & PTE_COW) || (*pte & PTE_W))
      return {(*pte & permission_for(access)) ? FaultResult::AlreadyMapped : FaultResult::Permission};
    if (level != 0) return {FaultResult::InvalidMapping};
    auto physical = PTE2PA(*pte);
    auto flags = (PTE_FLAGS(*pte) | PTE_W) & ~PTE_COW;
    if (allocator().page_references(reinterpret_cast<void *>(physical)) == 1)
    {
      *pte = PA2PTE(physical) | flags;
      xv::arch::sfence_vma();
      return {FaultResult::Resolved, physical};
    }
    auto *page = allocator().allocate();
    if (!page) return {FaultResult::OutOfMemory};
    xv::runtime::move(page, reinterpret_cast<void *>(physical), PGSIZE);
    *pte = PA2PTE(page) | flags;
    xv::arch::sfence_vma();
    allocator().release_page(reinterpret_cast<void *>(physical));
    return {FaultResult::Resolved, reinterpret_cast<uint64>(page)};
  }
  if (!region->demand || access == FaultAccess::Execute) return {FaultResult::OutsideRegion};
  auto *page = allocator().allocate();
  if (!page) return {FaultResult::OutOfMemory};
  xv::runtime::fill(page, 0, PGSIZE);
  if (map_user_pages(table, PGROUNDDOWN(address), PGSIZE, reinterpret_cast<uint64>(page), region->permissions) < 0)
  { allocator().deallocate(page); return {FaultResult::OutOfMemory}; }
  xv::arch::sfence_vma();
  return {FaultResult::Resolved, reinterpret_cast<uint64>(page)};
}

namespace
{
uint64 user_access(pagetable_t table, uint64 size, const VirtualRegions &regions,
                   uint64 address, FaultAccess access)
{
  if (!accessible_region(regions, size, address, access)) return 0;
  int level;
  auto *pte = walkto(table, address, 0, 0, &level);
  auto permission = static_cast<unsigned long>(permission_for(access) | PTE_U);
  if (pte && (*pte & PTE_V) && (*pte & permission) == permission)
    return PTE2PA(*pte) + (PGROUNDDOWN(address) & ((1UL << PXSHIFT(level)) - 1));
  auto result = resolve_user_fault(table, size, regions, address, access);
  return result.result == FaultResult::Resolved ? result.physical : 0;
}
uint64 accessible_bytes(const VirtualRegions &regions, uint64 size, uint64 address)
{
  auto *region = regions.find(address);
  auto end = region->end;
  if (region->kind != RegionKind::Special && size < end) end = size;
  auto page_end = PGROUNDDOWN(address) + PGSIZE;
  if (page_end < end) end = page_end;
  return end - address;
}
}

int copy_to_user(pagetable_t table, uint64 size, const VirtualRegions &regions,
                 uint64 destination, const char *source, uint64 bytes)
{
  if (bytes > MAXVA || destination >= MAXVA || bytes > MAXVA - destination) return -1;
  while (bytes)
  {
    auto physical = user_access(table, size, regions, destination, FaultAccess::Write);
    if (!physical) return -1;
    auto count = accessible_bytes(regions, size, destination);
    if (bytes < count) count = bytes;
    xv::runtime::move(reinterpret_cast<void *>(physical + (destination % PGSIZE)), source, count);
    bytes -= count; source += count; destination += count;
  }
  return 0;
}
int copy_from_user(pagetable_t table, uint64 size, const VirtualRegions &regions,
                   char *destination, uint64 source, uint64 bytes)
{
  if (bytes > MAXVA || source >= MAXVA || bytes > MAXVA - source) return -1;
  while (bytes)
  {
    auto physical = user_access(table, size, regions, source, FaultAccess::Read);
    if (!physical) return -1;
    auto count = accessible_bytes(regions, size, source);
    if (bytes < count) count = bytes;
    xv::runtime::move(destination, reinterpret_cast<void *>(physical + (source % PGSIZE)), count);
    bytes -= count; destination += count; source += count;
  }
  return 0;
}
int copy_user_string(pagetable_t table, uint64 size, const VirtualRegions &regions,
                     char *destination, uint64 source, uint64 maximum)
{
  while (maximum)
  {
    auto physical = user_access(table, size, regions, source, FaultAccess::Read);
    if (!physical) return -1;
    auto count = accessible_bytes(regions, size, source);
    if (maximum < count) count = maximum;
    auto *input = reinterpret_cast<char *>(physical + source % PGSIZE);
    for (uint64 i = 0; i < count; ++i)
    {
      *destination++ = input[i];
      if (!input[i]) return 0;
    }
    source += count; maximum -= count;
  }
  return -1;
}
int is_user_mapped(pagetable_t table, uint64 address)
{
  if (address >= MAXVA) return 0;
  auto *pte = walk_entry(table, address, 0);
  return pte && (*pte & PTE_V);
}
} // namespace xv::memory
