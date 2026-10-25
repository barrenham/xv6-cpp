module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

module xv.memory;

import xv.process;

extern "C" char etext[];
extern "C" char trampoline[];

namespace xv::memory
{
namespace
{
constinit KernelPageTable shared_kernel_page_table;

  // Make a direct-map page table for the kernel.
  pagetable_t make_kernel_page_table()
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
    xv::process::map_kernel_stacks(kpgtbl);

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

} // namespace

void KernelPageTable::initialize()
{
  if (root_)
  {
    panic("kernel page table: already initialized");
  }
  root_ = make_kernel_page_table();
}

void KernelPageTable::activate_hart() const
{
  if (!root_)
  {
    panic("kernel page table: not initialized");
  }
  sfence_vma();
  w_satp(MAKE_SATP(root_));
  sfence_vma();
}

const void *KernelPageTable::root() const
{
  return root_;
}

KernelPageTable &kernel_page_table()
{
  return shared_kernel_page_table;
}
} // namespace xv::memory
