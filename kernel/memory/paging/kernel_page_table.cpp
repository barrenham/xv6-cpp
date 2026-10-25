module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.memory;

import xv.arch;

import xv.print;

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
    xv::print::panic("kvmmake: no root page");
  }

  // uart registers
  xv::memory::map_kernel_range(kpgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W);

  // virtio mmio disk interface
  xv::memory::map_kernel_range(kpgtbl, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

#ifdef LAB_NET
  // PCI-E ECAM (configuration space), for pci.c
  xv::memory::map_kernel_range(kpgtbl, 0x30000000L, 0x30000000L, 0x10000000,
                               PTE_R | PTE_W);

  // pci.c maps the e1000's registers here.
  xv::memory::map_kernel_range(kpgtbl, 0x40000000L, 0x40000000L, 0x20000,
                               PTE_R | PTE_W);
#endif

#ifdef LAB_LOCK
  // CLINT
  xv::memory::map_kernel_range(kpgtbl, CLINT_BASE, CLINT_BASE, PGSIZE, PTE_R | PTE_W);
#endif

  // PLIC
  xv::memory::map_kernel_range(kpgtbl, PLIC, PLIC, 0x4000000, PTE_R | PTE_W);

  // map kernel text executable and read-only.
  xv::memory::map_kernel_range(kpgtbl, KERNBASE, KERNBASE, (uint64)etext - KERNBASE,
                               PTE_R | PTE_X);

  // map kernel data and the physical RAM we'll make use of.
  const auto &layout = xv::memory::allocator().layout();
  for (unsigned i = 0; i < layout.direct_count(); ++i)
  {
    const auto &region = layout.direct(i);
    uint64 begin =
        region.begin().raw() < (uint64)etext ? (uint64)etext : region.begin().raw();
    if (begin < region.end().raw())
    {
      xv::memory::map_kernel_range(kpgtbl, begin, begin, region.end().raw() - begin,
                                   PTE_R | PTE_W);
    }
  }

  // map the trampoline for trap entry/exit to
  // the highest virtual address in the kernel.
  xv::memory::map_kernel_range(kpgtbl, TRAMPOLINE, (uint64)trampoline, PGSIZE,
                               PTE_R | PTE_X);

  // allocate and map a kernel stack for each process.
  xv::process::map_kernel_stacks(kpgtbl);

  xv::memory::PageTranslation translated;
  if (!owned.view().translate((uint64)etext - 1, translated) ||
      (translated.permissions & PTE_W) || !(translated.permissions & PTE_X))
  {
    xv::print::panic("direct map: text permissions");
  }
  if (!owned.view().translate((uint64)etext, translated) ||
      !(translated.permissions & PTE_W) || (translated.permissions & PTE_X))
  {
    xv::print::panic("direct map: data permissions");
  }
  for (unsigned i = 0; i < layout.direct_count(); ++i)
  {
    const auto &range = layout.direct(i);
    uint64 boundaries[2] = {range.begin().raw(), range.end().raw() - 1};
    for (uint64 va : boundaries)
    {
      if (!owned.view().translate(va, translated) || translated.physical.raw() != va)
      {
        xv::print::panic("direct map: region boundary");
      }
    }
  }
  if (owned.view().translate(layout.ram().end().raw(), translated))
  {
    xv::print::panic("direct map: beyond RAM");
  }
  auto counts = owned.view().statistics();
  xv::print::printk("kernel mappings: 1GiB=%lu 2MiB=%lu 4KiB=%lu table_pages=%lu\n",
                    counts.leaves_1g, counts.leaves_2m, counts.leaves_4k,
                    counts.table_pages);
  xv::print::printk("direct map self-check: OK\n");
  return static_cast<pagetable_t>(owned.release());
}

} // namespace

void KernelPageTable::initialize()
{
  if (root_)
  {
    xv::print::panic("kernel page table: already initialized");
  }
  root_ = make_kernel_page_table();
}

void KernelPageTable::activate_hart() const
{
  if (!root_)
  {
    xv::print::panic("kernel page table: not initialized");
  }
  xv::arch::sfence_vma();
  xv::arch::w_satp(MAKE_SATP(root_));
  xv::arch::sfence_vma();
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
