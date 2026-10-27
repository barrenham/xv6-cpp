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
void map_initial_range(VirtualRegions &regions, pagetable_t table, unsigned long va,
                       unsigned long pa, unsigned long bytes, int permissions)
{
  auto kind = va == TRAMPOLINE ? RegionKind::Special :
              va >= KERNBASE ? RegionKind::DirectMap : RegionKind::Device;
  if (!regions.reserve(va, va + bytes, permissions, kind))
    xv::print::panic("kernel mapping: virtual region collision");
  xv::memory::map_kernel_range(table, va, pa, bytes, permissions);
}
pagetable_t make_kernel_page_table(VirtualRegions &regions)
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
  map_initial_range(regions, kpgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W);

  // virtio mmio disk interface
  map_initial_range(regions, kpgtbl, VIRTIO0, VIRTIO0, 2 * PGSIZE, PTE_R | PTE_W);

#ifdef LAB_NET
  // PCI-E ECAM (configuration space), for pci.c
  map_initial_range(regions, kpgtbl, 0x30000000L, 0x30000000L, 0x10000000,
                               PTE_R | PTE_W);

  // pci.c maps the e1000's registers here.
  map_initial_range(regions, kpgtbl, 0x40000000L, 0x40000000L, 0x80000,
                               PTE_R | PTE_W);
#endif

#ifdef LAB_LOCK
  // CLINT
  map_initial_range(regions, kpgtbl, CLINT_BASE, CLINT_BASE, PGSIZE, PTE_R | PTE_W);
#endif

  // PLIC
  map_initial_range(regions, kpgtbl, PLIC, PLIC, 0x4000000, PTE_R | PTE_W);

  // map kernel text executable and read-only.
  map_initial_range(regions, kpgtbl, KERNBASE, KERNBASE, (uint64)etext - KERNBASE,
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
      map_initial_range(regions, kpgtbl, begin, begin, region.end().raw() - begin,
                                   PTE_R | PTE_W);
    }
  }

  // map the trampoline for trap entry/exit to
  // the highest virtual address in the kernel.
  map_initial_range(regions, kpgtbl, TRAMPOLINE, (uint64)trampoline, PGSIZE,
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
  root_ = make_kernel_page_table(regions_);
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
  if (!regions_.reserve(address - KSTACK_BYTES, address, 0, RegionKind::Guard)) return 0;
  if (!regions_.reserve(address, address + KSTACK_BYTES, PTE_R | PTE_W, RegionKind::KernelStack, true))
  { regions_.erase(address - KSTACK_BYTES, address); return 0; }
  // Every leaf table is prepared before publication. A later stack fault only
  // needs data frames and can complete without acquiring any VM/allocator lock.
  bool prepared = true;
  for (auto cursor = address; cursor < address + KSTACK_BYTES; cursor += 512 * PageSize)
    if (!walk_entry(static_cast<PageTable>(root_), cursor, 1)) { prepared = false; break; }
  if (!prepared || !commit_stack(address, address + KSTACK_BYTES - KSTACK_INITIAL_PAGES * PageSize))
  {
    release_stack(address);
    regions_.erase(address - KSTACK_BYTES, address + KSTACK_BYTES);
    return 0;
  }
  __atomic_fetch_or(&stack_bitmap_[slot / 64], 1UL << (slot % 64), __ATOMIC_RELEASE);
  return address;
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
  release_stack(address);
  regions_.erase(address - KSTACK_BYTES, address + KSTACK_BYTES);
  __atomic_fetch_and(&stack_bitmap_[slot / 64], ~(1UL << (slot % 64)), __ATOMIC_RELEASE);
}

FaultResult KernelPageTable::handle_stack_fault(unsigned long base, unsigned long stack,
                                               unsigned long fault, FaultAccess access, void *(*frame)())
{
  if (base > KSTACK(0UL) || (KSTACK(0UL) - base) % (2 * KSTACK_BYTES) ||
      fault < base || fault >= base + KSTACK_BYTES || stack < base || stack > base + KSTACK_BYTES)
    return FaultResult::OutsideRegion;
  auto slot = (KSTACK(0UL) - base) / (2 * KSTACK_BYTES);
  if (slot >= stack_slots_ || !(__atomic_load_n(&stack_bitmap_[slot / 64], __ATOMIC_ACQUIRE) &
                               (1UL << (slot % 64)))) return FaultResult::OutsideRegion;
  if (access == FaultAccess::Execute) return FaultResult::Permission;
  auto *pte = walk_entry(static_cast<PageTable>(root_), fault, 0);
  if (!pte) return FaultResult::InvalidMapping;
  if (*pte & PTE_V)
  {
    if (!PTE_LEAF(*pte) || ((*pte & PTE_W) && !(*pte & PTE_R))) return FaultResult::InvalidMapping;
    auto permission = access == FaultAccess::Write ? PTE_W : PTE_R;
    return (*pte & permission) ? FaultResult::AlreadyMapped : FaultResult::Permission;
  }
  auto lowest = (fault < stack ? fault : stack) & ~(PageSize - 1);
  for (auto cursor = base + KSTACK_BYTES; cursor > lowest;)
  {
    cursor -= PageSize;
    auto *entry = walk_entry(static_cast<PageTable>(root_), cursor, 0);
    if (!entry) { xv::arch::sfence_vma(); return FaultResult::InvalidMapping; }
    if (*entry & PTE_V) continue;
    auto *page = frame();
    if (!page) { xv::arch::sfence_vma(); return FaultResult::OutOfMemory; }
    __atomic_store_n(entry, PA2PTE(page) | PTE_R | PTE_W | PTE_V, __ATOMIC_RELEASE);
  }
  xv::arch::sfence_vma();
  return FaultResult::Resolved;
}

bool KernelPageTable::region(unsigned long address, VirtualRegion &result)
{
  xv::sync::ScopedLock guard(stacks_);
  auto *found = regions_.find(address);
  if (!found) return false;
  result = *found;
  return true;
}

PageTableStatistics KernelPageTable::statistics()
{
  // Serializes against publication and reclamation of stack leaf tables.
  // Lock-free stack faults may add leaf mappings, so leaf counts are advisory.
  xv::sync::ScopedLock guard(stacks_);
  LargestPagePolicy policy;
  Sv39PageTable view(static_cast<PageTable>(root_), allocator(), policy);
  return view.statistics();
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
