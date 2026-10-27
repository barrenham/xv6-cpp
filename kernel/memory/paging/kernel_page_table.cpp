module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.memory;

import xv.arch;

import xv.print;
import xv.runtime;

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
  xv::memory::map_kernel_range(kpgtbl, VIRTIO0, VIRTIO0, 2 * PGSIZE, PTE_R | PTE_W);

#ifdef LAB_NET
  // PCI-E ECAM (configuration space), for pci.c
  xv::memory::map_kernel_range(kpgtbl, 0x30000000L, 0x30000000L, 0x10000000,
                               PTE_R | PTE_W);

  // pci.c maps the e1000's registers here.
  xv::memory::map_kernel_range(kpgtbl, 0x40000000L, 0x40000000L, 0x80000,
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
  stacks_.init("kernel stack regions");
  // Upper-level tables are shared and stable. Each 2 MiB stack owns its leaf
  // tables, which can be reclaimed after its process has stopped running.
  auto *table = static_cast<PageTable>(root_);
  for (unsigned i = PX(2, KSTACK_BOTTOM); i <= PX(2, KSTACK_TOP - 1); ++i)
  {
    if (!(table[i] & PTE_V))
    {
      auto *page = allocator().allocate();
      if (!page) xv::print::panic("kernel stack directory: out of memory");
      xv::runtime::fill(page, 0, PageSize); table[i] = PA2PTE(page) | PTE_V;
    }
  }
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



bool KernelPageTable::commit_stack(unsigned long address, unsigned long lowest)
{
  lowest &= ~(PageSize - 1);
  for (auto cursor = address + KSTACK_BYTES; cursor > lowest;)
  {
    cursor -= PageSize;
    auto *entry = walk_entry(static_cast<PageTable>(root_), cursor, 1);
    if (!entry) { xv::arch::sfence_vma(); return false; }
    if (*entry & PTE_V) continue;
    auto *page = allocator().allocate();
    if (!page) { xv::arch::sfence_vma(); return false; }
    *entry = PA2PTE(page) | PTE_R | PTE_W | PTE_V;
  }
  xv::arch::sfence_vma(); return true;
}
void KernelPageTable::release_stack(unsigned long address)
{
  constexpr unsigned long span = 512 * PageSize;
  auto *root = static_cast<PageTable>(root_);
  for (auto cursor = address; cursor < address + KSTACK_BYTES; cursor += span)
  {
    auto *middle = reinterpret_cast<PageTable>(PTE2PA(root[PX(2, cursor)]));
    auto &directory = middle[PX(1, cursor)];
    if (!(directory & PTE_V)) continue;
    auto *leaf = reinterpret_cast<PageTable>(PTE2PA(directory)); directory = 0;
    xv::arch::sfence_vma();
    for (unsigned i = 0; i < 512; ++i) if (leaf[i] & PTE_V) allocator().deallocate(reinterpret_cast<void *>(PTE2PA(leaf[i])));
    allocator().deallocate(leaf);
  }
}
unsigned long KernelPageTable::allocate_process_stack()
{
  xv::sync::ScopedLock guard(stacks_);
  unsigned long slot = 0;
  while (slot < stack_slots_ && (stack_bitmap_[slot / 64] & (1UL << (slot % 64)))) ++slot;
  if (slot == stack_slots_) return 0;
  auto address = KSTACK(slot);
  if (!commit_stack(address, address + KSTACK_BYTES - KSTACK_INITIAL_PAGES * PageSize))
  { release_stack(address); return 0; }
  stack_bitmap_[slot / 64] |= 1UL << (slot % 64); return address;
}
bool KernelPageTable::grow_process_stack(unsigned long address, unsigned long lowest)
{
  if (address > KSTACK(0UL) || (KSTACK(0UL) - address) % (2 * KSTACK_BYTES) || lowest < address || lowest >= address + KSTACK_BYTES) return false;
  xv::sync::ScopedLock guard(stacks_);
  auto slot = (KSTACK(0UL) - address) / (2 * KSTACK_BYTES);
  if (slot >= stack_slots_ || !(stack_bitmap_[slot / 64] & (1UL << (slot % 64)))) return false;
  return commit_stack(address, lowest);
}
void KernelPageTable::free_process_stack(unsigned long address)
{
  xv::sync::ScopedLock guard(stacks_);
  if (address > KSTACK(0UL) || (KSTACK(0UL) - address) % (2 * KSTACK_BYTES)) xv::print::panic("kernel stack: invalid region");
  auto slot = (KSTACK(0UL) - address) / (2 * KSTACK_BYTES);
  if (slot >= stack_slots_ || !(stack_bitmap_[slot / 64] & (1UL << (slot % 64)))) xv::print::panic("kernel stack: unowned region");
  release_stack(address); stack_bitmap_[slot / 64] &= ~(1UL << (slot % 64));
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
