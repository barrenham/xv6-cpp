import xv.sync;

#include "kernel/support/param.h"
#include "kernel/support/types.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/process/elf.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"
#include "kernel/process/proc.h"
#include "kernel/fs/fs.h"

import xv.memory;

extern "C"
{

  /*
 * the kernel's page table.
 */
  pagetable_t kernel_pagetable;

  extern char etext[]; // kernel.ld sets this to end of kernel code.

  extern char trampoline[]; // trampoline.S

  // Make a direct-map page table for the kernel.
  pagetable_t kvmmake(void)
  {
    pagetable_t kpgtbl;

    xv::memory::LargestPagePolicy policy;
    xv::memory::OwnedSv39PageTable owned(xv::memory::allocator(), policy);
    kpgtbl = static_cast<pagetable_t>(owned.root());
    if (!kpgtbl)
    {
      panic("kvmmake: no root page");
    }

    // uart registers
    kvmmap(kpgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W);

    // virtio mmio disk interface
    kvmmap(kpgtbl, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

#ifdef LAB_NET
    // PCI-E ECAM (configuration space), for pci.c
    kvmmap(kpgtbl, 0x30000000L, 0x30000000L, 0x10000000, PTE_R | PTE_W);

    // pci.c maps the e1000's registers here.
    kvmmap(kpgtbl, 0x40000000L, 0x40000000L, 0x20000, PTE_R | PTE_W);
#endif

#ifdef LAB_LOCK
    // CLINT
    kvmmap(kpgtbl, CLINT_BASE, CLINT_BASE, PGSIZE, PTE_R | PTE_W);
#endif

    // PLIC
    kvmmap(kpgtbl, PLIC, PLIC, 0x4000000, PTE_R | PTE_W);

    // map kernel text executable and read-only.
    kvmmap(kpgtbl, KERNBASE, KERNBASE, (uint64)etext - KERNBASE, PTE_R | PTE_X);

    // map kernel data and the physical RAM we'll make use of.
    const auto &layout = xv::memory::allocator().layout();
    for (unsigned i = 0; i < layout.direct_count(); ++i)
    {
      const auto &region = layout.direct(i);
      uint64 begin =
          region.begin().raw() < (uint64)etext ? (uint64)etext : region.begin().raw();
      if (begin < region.end().raw())
      {
        kvmmap(kpgtbl, begin, begin, region.end().raw() - begin, PTE_R | PTE_W);
      }
    }

    // map the trampoline for trap entry/exit to
    // the highest virtual address in the kernel.
    kvmmap(kpgtbl, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);

    // allocate and map a kernel stack for each process.
    proc_mapstacks(kpgtbl);

    xv::memory::PageTranslation translated;
    if (!owned.view().translate((uint64)etext - 1, translated) ||
        (translated.permissions & PTE_W) || !(translated.permissions & PTE_X))
    {
      panic("direct map: text permissions");
    }
    if (!owned.view().translate((uint64)etext, translated) ||
        !(translated.permissions & PTE_W) || (translated.permissions & PTE_X))
    {
      panic("direct map: data permissions");
    }
    for (unsigned i = 0; i < layout.direct_count(); ++i)
    {
      const auto &range = layout.direct(i);
      uint64 boundaries[2] = {range.begin().raw(), range.end().raw() - 1};
      for (uint64 va : boundaries)
      {
        if (!owned.view().translate(va, translated) || translated.physical.raw() != va)
        {
          panic("direct map: region boundary");
        }
      }
    }
    if (owned.view().translate(layout.ram().end().raw(), translated))
    {
      panic("direct map: beyond RAM");
    }
    auto counts = owned.view().statistics();
    printk("kernel mappings: 1GiB=%lu 2MiB=%lu 4KiB=%lu table_pages=%lu\n",
           counts.leaves_1g, counts.leaves_2m, counts.leaves_4k, counts.table_pages);
    printk("direct map self-check: OK\n");
    return static_cast<pagetable_t>(owned.release());
  }

  // Initialize the kernel_pagetable, shared by all CPUs.
  void kvminit(void)
  {
    kernel_pagetable = kvmmake();
  }

  // Switch the current CPU's h/w page table register to
  // the kernel's page table, and enable paging.
  void kvminithart()
  {
    // wait for any previous writes to the page table memory to finish.
    sfence_vma();

    w_satp(MAKE_SATP(kernel_pagetable));

    // flush stale entries from the TLB.
    sfence_vma();
  }

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
      panic("walk");
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
        memset(pagetable, 0, PGSIZE);
        *pte = PA2PTE(pagetable) | PTE_V;
      }
    }
    if (leaflevel)
    {
      *leaflevel = target;
    }
    return &pagetable[PX(target, va)];
  }

  pte_t *walk(pagetable_t pagetable, uint64 va, int alloc)
  {
    return walkto(pagetable, va, alloc, 0, 0);
  }

  // Look up a virtual address, return the physical address,
  // or 0 if not mapped.
  // Can only be used to look up user pages.
  uint64 walkaddr(pagetable_t pagetable, uint64 va)
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
    // copyin/out copy one 4 KiB slice at a time, even within a superpage.
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
        printk(" ..");
      }
      printk("%p: pte %p pa %p", (void *)va, (void *)pte, (void *)PTE2PA(pte));
      if (PTE_LEAF(pte))
      {
        printk(" %s%s%s%s", (pte & PTE_R) ? "R" : "", (pte & PTE_W) ? "W" : "",
               (pte & PTE_X) ? "X" : "", (pte & PTE_U) ? "U" : "");
        if (level > 0)
        {
          printk(" superpage");
        }
      }
      printk("\n");
      if (level > 0 && !PTE_LEAF(pte))
      {
        vmprint_level((pagetable_t)PTE2PA(pte), level - 1, va);
      }
    }
  }

  void vmprint(pagetable_t pagetable)
  {
    printk("page table %p\n", pagetable);
    vmprint_level(pagetable, 2, 0);
  }
#endif

#ifdef LAB_PGTBL
  static int supermap(pagetable_t pagetable, uint64 va, uint64 pa, int perm)
  {
    int level;
    if ((va | pa) % SUPERPGSIZE != 0)
    {
      panic("supermap: alignment");
    }
    pte_t *pte = walkto(pagetable, va, 1, 1, &level);
    if (pte == 0)
    {
      return -1;
    }
    if (level != 1)
    {
      panic("supermap: level");
    }
    if (*pte & PTE_V)
    {
      if (PTE_LEAF(*pte))
      {
        panic("supermap: remap");
      }
      // A previous shrink may have left an empty level-0 page table.
      pagetable_t child = (pagetable_t)PTE2PA(*pte);
      for (int i = 0; i < 512; i++)
      {
        if (child[i] & PTE_V)
        {
          panic("supermap: occupied");
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
      panic("unsupported page split");
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
  int uvmdemote(pagetable_t pagetable, uint64 va, uint64 npages)
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
  void kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
  {
    xv::memory::LargestPagePolicy policy;
    xv::memory::Sv39PageTable table(kpgtbl, xv::memory::allocator(), policy);
    if (table.map_range(va, xv::memory::PhysicalAddress(pa), sz, perm) !=
        xv::memory::MappingResult::Ok)
    {
      panic("kvmmap");
    }
  }

  // Create PTEs for virtual addresses starting at va that refer to
  // physical addresses starting at pa.
  // va and size MUST be page-aligned.
  // Returns 0 on success, -1 if walk() couldn't
  // allocate a needed page-table page.

  int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
  {
    uint64 a, last;
    pte_t *pte;

    if ((va % PGSIZE) != 0)
    {
      panic("mappages: va not aligned");
    }

    if ((size % PGSIZE) != 0)
    {
      panic("mappages: size not aligned");
    }

    if (size == 0)
    {
      panic("mappages: size");
    }

    a = va;
    last = va + size - PGSIZE;
    for (;;)
    {
      if ((pte = walk(pagetable, a, 1)) == 0)
      {
        return -1;
      }
      if (*pte & PTE_V)
      {
        panic("mappages: remap");
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
  pagetable_t uvmcreate()
  {
    pagetable_t pagetable;
    pagetable = (pagetable_t)xv::memory::allocator().allocate();
    if (pagetable == 0)
    {
      return 0;
    }
    memset(pagetable, 0, PGSIZE);
    return pagetable;
  }

  // Remove npages of mappings starting from va. va must be
  // page-aligned. It's OK if the mappings don't exist.
  // Optionally free the physical memory.
  void uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
  {
    uint64 a;
    pte_t *pte;
    int sz = PGSIZE;
    int level;

    if ((va % PGSIZE) != 0)
    {
      panic("uvmunmap: not aligned");
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
          panic("uvmunmap: split");
        }
        pte = walk(pagetable, a, 0);
        if ((*pte & PTE_V) == 0) // recycled as the level-0 page table
        {
          continue;
        }
      }
#endif
      if (PTE_FLAGS(*pte) == PTE_V)
      {
        panic("uvmunmap: not a leaf");
      }
      if (do_free)
      {
        uint64 pa = PTE2PA(*pte);
        xv::memory::allocator().deallocate((void *)pa);
      }
      *pte = 0;
    }
  }

  // Allocate PTEs and physical memory to grow process from oldsz to
  // newsz, which need not be page aligned.  Returns new size or 0 on error.
  uint64 uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm)
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
        memset(mem, 0, SUPERPGSIZE);
        if (supermap(pagetable, a, (uint64)mem, PTE_R | PTE_U | xperm) < 0)
        {
          xv::memory::allocator().deallocate(mem, xv::memory::SuperpageOrder);
          uvmdealloc(pagetable, a, oldsz);
          return 0;
        }
        sz = SUPERPGSIZE;
        continue;
      }
#endif
      mem = static_cast<char *>(xv::memory::allocator().allocate());
      if (mem == 0)
      {
        uvmdealloc(pagetable, a, oldsz);
        return 0;
      }
#ifndef LAB_SYSCALL
      memset(mem, 0, sz);
#endif
      if (mappages(pagetable, a, sz, (uint64)mem, PTE_R | PTE_U | xperm) != 0)
      {
        xv::memory::allocator().deallocate(mem);
        uvmdealloc(pagetable, a, oldsz);
        return 0;
      }
    }
    return newsz;
  }

  // Deallocate user pages to bring the process size from oldsz to
  // newsz.  oldsz and newsz need not be page-aligned, nor does newsz
  // need to be less than oldsz.  oldsz can be larger than the actual
  // process size.  Returns the new process size.
  uint64 uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
  {
    if (newsz >= oldsz)
    {
      return oldsz;
    }

    if (PGROUNDUP(newsz) < PGROUNDUP(oldsz))
    {
      int npages = (PGROUNDUP(oldsz) - PGROUNDUP(newsz)) / PGSIZE;
      uvmunmap(pagetable, PGROUNDUP(newsz), npages, 1);
    }

    return newsz;
  }

  // Recursively free page-table pages.
  // All leaf mappings must already have been removed.
  void freewalk(pagetable_t pagetable)
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
        panic("freewalk: leaf");
      }
    }
    xv::memory::allocator().deallocate((void *)pagetable);
  }

  // Free user memory pages,
  // then free page-table pages.
  void uvmfree(pagetable_t pagetable, uint64 sz)
  {
    if (sz > 0)
    {
      uvmunmap(pagetable, 0, PGROUNDUP(sz) / PGSIZE, 1);
    }
    freewalk(pagetable);
  }

  // Given a parent process's page table, copy
  // its memory into a child's page table.
  // Copies both the page table and the
  // physical memory.
  // returns 0 on success, -1 on failure.
  // frees any allocated pages on failure.
  int uvmcopy(pagetable_t old, pagetable_t new_table, uint64 sz)
  {
    pte_t *pte;
    uint64 pa, i;
    uint flags;
    char *mem;
    int szinc = PGSIZE;
    int level;

    for (i = 0; i < sz; i += szinc)
    {
      szinc = PGSIZE;
      if ((pte = walkto(old, i, 0, 0, &level)) == 0)
      {
        continue;
      }
      if ((*pte & PTE_V) == 0)
      {
        continue;
      }
      szinc = PGSIZE;
      pa = PTE2PA(*pte);
      flags = PTE_FLAGS(*pte);
#ifdef LAB_PGTBL
      if (level == 1)
      {
        if ((mem = static_cast<char *>(
                 xv::memory::allocator().allocate(xv::memory::SuperpageOrder))) == 0)
        {
          goto err;
        }
        memmove(mem, (char *)pa, SUPERPGSIZE);
        if (supermap(new_table, i, (uint64)mem, flags) != 0)
        {
          xv::memory::allocator().deallocate(mem, xv::memory::SuperpageOrder);
          goto err;
        }
        szinc = SUPERPGSIZE;
        continue;
      }
#endif
      if ((mem = static_cast<char *>(xv::memory::allocator().allocate())) == 0)
      {
        goto err;
      }
      memmove(mem, (char *)pa, PGSIZE);
      if (mappages(new_table, i, PGSIZE, (uint64)mem, flags) != 0)
      {
        xv::memory::allocator().deallocate(mem);
        goto err;
      }
    }
    return 0;

  err:
    uvmunmap(new_table, 0, i / PGSIZE, 1);
    return -1;
  }

  // mark a PTE invalid for user access.
  // used by exec for the user stack guard page.
  void uvmclear(pagetable_t pagetable, uint64 va)
  {
    pte_t *pte;

    pte = walk(pagetable, va, 0);
    if (pte == 0)
    {
      panic("uvmclear");
    }
    *pte &= ~PTE_U;
  }

  // Copy from kernel to user.
  // Copy len bytes from src to virtual address dstva in a given page table.
  // Return 0 on success, -1 on error.
  int copyout(pagetable_t pagetable, uint64 psz, uint64 dstva, char *src, uint64 len)
  {
    uint64 n, va0, pa0;
    pte_t *pte;

    while (len > 0)
    {
      va0 = PGROUNDDOWN(dstva);
      if (va0 >= MAXVA)
      {
        return -1;
      }

      pa0 = walkaddr(pagetable, va0);
      if (pa0 == 0)
      {
        if ((pa0 = vmfault(pagetable, psz, va0, 0)) == 0)
        {
          return -1;
        }
      }

      if ((pte = walk(pagetable, va0, 0)) == 0)
      {
        // printk("copyout: pte should exist %lx %ld\n", dstva, len);
        return -1;
      }

      // forbid copyout over read-only user text pages.
      if ((*pte & PTE_W) == 0)
      {
        return -1;
      }

      n = PGSIZE - (dstva - va0);
      if (n > len)
      {
        n = len;
      }
      memmove((void *)(pa0 + (dstva - va0)), src, n);

      len -= n;
      src += n;
      dstva = va0 + PGSIZE;
    }
    return 0;
  }

  // Copy from user to kernel.
  // Copy len bytes to dst from virtual address srcva in a given page table.
  // Return 0 on success, -1 on error.
  int copyin(pagetable_t pagetable, uint64 psz, char *dst, uint64 srcva, uint64 len)
  {
    uint64 n, va0, pa0;

    while (len > 0)
    {
      va0 = PGROUNDDOWN(srcva);
      pa0 = walkaddr(pagetable, va0);
      if (pa0 == 0)
      {
        if ((pa0 = vmfault(pagetable, psz, va0, 1)) == 0)
        {
          return -1;
        }
      }
      n = PGSIZE - (srcva - va0);
      if (n > len)
      {
        n = len;
      }
      memmove(dst, (void *)(pa0 + (srcva - va0)), n);

      len -= n;
      dst += n;
      srcva = va0 + PGSIZE;
    }
    return 0;
  }

  // Copy a null-terminated string from user to kernel.
  // Copy bytes to dst from virtual address srcva in a given page table,
  // until a '\0', or max.
  // Return 0 on success, -1 on error.
  int copyinstr(pagetable_t pagetable, uint64 psz, char *dst, uint64 srcva, uint64 max)
  {
    uint64 n, va0, pa0;
    int got_null = 0;

    while (got_null == 0 && max > 0)
    {
      va0 = PGROUNDDOWN(srcva);
      pa0 = walkaddr(pagetable, va0);
      if (pa0 == 0)
      {
        if ((pa0 = vmfault(pagetable, psz, va0, 1)) == 0)
        {
          return -1;
        }
      }
      n = PGSIZE - (srcva - va0);
      if (n > max)
      {
        n = max;
      }

      char *p = (char *)(pa0 + (srcva - va0));
      while (n > 0)
      {
        if (*p == '\0')
        {
          *dst = '\0';
          got_null = 1;
          break;
        }
        else
        {
          *dst = *p;
        }
        --n;
        --max;
        p++;
        dst++;
      }

      srcva = va0 + PGSIZE;
    }
    if (got_null)
    {
      return 0;
    }
    else
    {
      return -1;
    }
  }

  // allocate and map user memory if process is referencing a page
  // that was lazily allocated in sys_sbrk().
  // returns 0 if va is invalid or already mapped, or if
  // out of physical memory, and physical address if successful.
  uint64 vmfault(pagetable_t pagetable, uint64 psz, uint64 va, int read)
  {
    uint64 mem;

    if (va >= psz)
    {
      return 0;
    }
    va = PGROUNDDOWN(va);
    if (ismapped(pagetable, va))
    {
      return 0;
    }
    mem = (uint64)xv::memory::allocator().allocate();
    if (mem == 0)
    {
      return 0;
    }
    memset((void *)mem, 0, PGSIZE);
    if (mappages(pagetable, va, PGSIZE, mem, PTE_W | PTE_U | PTE_R) != 0)
    {
      xv::memory::allocator().deallocate((void *)mem);
      return 0;
    }
    return mem;
  }

  int ismapped(pagetable_t pagetable, uint64 va)
  {
    pte_t *pte = walk(pagetable, va, 0);
    if (pte == 0)
    {
      return 0;
    }
    if (*pte & PTE_V)
    {
      return 1;
    }
    return 0;
  }
}
