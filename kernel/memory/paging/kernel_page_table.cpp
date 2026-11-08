module;

#include "kernel/support/types.h"
#include "kernel/hardware/device_tree.h"
#include "kernel/support/flow.h"
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
// RSW bit 9 on the first non-leaf stack directory publishes a complete stack.
// Fault callers pin their running stack, including its upper directory pages.
constexpr unsigned long StackPublished = 1UL << 9;

// Make a direct-map page table for the kernel.
void
map_initial_range(VirtualAddressPool &regions, pagetable_t table, unsigned long va,
                  unsigned long pa, unsigned long bytes, int permissions) {
  auto kind = va == TRAMPOLINE ? RegionKind::Special
              : va >= KERNBASE ? RegionKind::DirectMap
                               : RegionKind::Device;
  if (!regions.reserve(va, va + bytes, permissions, kind))
  {
    xv::print::panic("kernel mapping: virtual region collision");
  }
  xv::memory::map_kernel_range(table, va, pa, bytes, permissions);
}

namespace flow = xv::flow;

struct KernelBuildFlow
{
  struct Context
  {
    VirtualAddressPool &regions;
    OwnedSv39PageTable &owned;
    pagetable_t table;
    pagetable_t result = nullptr;
  };

  static bool
  root(Context &c) noexcept {
    return c.table != nullptr;
  }

  static void
  fail(Context &) noexcept {
    xv::print::panic("kvmmake: no root page");
  }

  static void
  devices_text(Context &c) noexcept {
    for (unsigned i = 0; i < xv::hardware::description().regions_count; ++i)
    {
      const auto &region = xv::hardware::description().regions[i];
      map_initial_range(c.regions, c.table, region.address, region.address,
                        region.bytes, PTE_R | PTE_W);
    }

    // map kernel text executable and read-only.
    map_initial_range(c.regions, c.table, KERNBASE, KERNBASE, (uint64)etext - KERNBASE,
                      PTE_R | PTE_X);
  }

  static void
  direct(Context &c) noexcept {
    // map kernel data and the physical RAM we'll make use of.
    const auto &layout = xv::memory::allocator().layout();
    for (unsigned i = 0; i < layout.direct_count(); ++i)
    {
      const auto &region = layout.direct(i);
      uint64 begin =
          region.begin().raw() < (uint64)etext ? (uint64)etext : region.begin().raw();
      if (begin < region.end().raw())
      {
        map_initial_range(c.regions, c.table, begin, begin, region.end().raw() - begin,
                          PTE_R | PTE_W);
      }
    }
  }

  static void
  map_trampoline(Context &c) noexcept {
    // map the trampoline for trap entry/exit to
    // the highest virtual address in the kernel.
    map_initial_range(c.regions, c.table, TRAMPOLINE, (uint64)trampoline, PGSIZE,
                      PTE_R | PTE_X);
  }

  static void
  check(Context &c) noexcept {
    const auto &layout = xv::memory::allocator().layout();
    xv::memory::PageTranslation translated;
    if (!c.owned.view().translate((uint64)etext - 1, translated) ||
        (translated.permissions & PTE_W) || !(translated.permissions & PTE_X))
    {
      xv::print::panic("direct map: text permissions");
    }
    if (!c.owned.view().translate((uint64)etext, translated) ||
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
        if (!c.owned.view().translate(va, translated) ||
            translated.physical.raw() != va)
        {
          xv::print::panic("direct map: region boundary");
        }
      }
    }
    if (c.owned.view().translate(layout.ram().end().raw(), translated))
    {
      xv::print::panic("direct map: beyond RAM");
    }
  }

  static void
  report(Context &c) noexcept {
    auto counts = c.owned.view().statistics();
    xv::print::printk("kernel mappings: 1GiB=%lu 2MiB=%lu 4KiB=%lu table_pages=%lu\n",
                      counts.leaves_1g, counts.leaves_2m, counts.leaves_4k,
                      counts.table_pages);
    xv::print::printk("direct map self-check: OK\n");
  }

  static void
  release(Context &c) noexcept {
    c.result = static_cast<pagetable_t>(c.owned.release());
  }
  struct Root;
  struct Fail;
  struct DevicesText;
  struct Direct;
  struct Trampoline;
  struct Check;
  struct Report;
  struct Release;
  using Graph = flow::graph<
      Root, flow::types<
                flow::node<Root, flow::noop,
                           flow::branch<flow::condition<&root>, flow::go<DevicesText>,
                                        flow::go<Fail>>>,
                flow::node<Fail, flow::action<&fail>, flow::stop>,
                flow::node<DevicesText, flow::action<&devices_text>, flow::go<Direct>>,
                flow::node<Direct, flow::action<&direct>, flow::go<Trampoline>>,
                flow::node<Trampoline, flow::action<&map_trampoline>, flow::go<Check>>,
                flow::node<Check, flow::action<&check>, flow::go<Report>>,
                flow::node<Report, flow::action<&report>, flow::go<Release>>,
                flow::node<Release, flow::action<&release>, flow::stop>>>;
};

pagetable_t
make_kernel_page_table(VirtualAddressPool &regions) {
  LargestPagePolicy policy;
  OwnedSv39PageTable owned(allocator(), policy);
  KernelBuildFlow::Context context{regions, owned,
                                   static_cast<pagetable_t>(owned.root())};
  (void)KernelBuildFlow::Graph::run<0>(context);
  return context.result;
}

} // namespace

namespace flow = xv::flow;

struct KernelPageTable::InitializeFlow
{
  struct Context
  {
    KernelPageTable &owner;
  };

  static void
  mappings(Context &c) noexcept {
    if (!c.owner.regions_.initialize())
    {
      xv::print::panic("kernel address space: initialization failed");
    }
    c.owner.root_ = make_kernel_page_table(c.owner.regions_);
  }

  static void
  pool(Context &c) noexcept {
    c.owner.stacks_.init("kernel stack regions");
    c.owner.pool_begin_ = allocator().layout().ram().end().align_up(KSTACK_BYTES).raw();
  }
  struct Mappings;
  struct Pool;
  using Graph = flow::graph<
      Mappings,
      flow::types<flow::node<Mappings, flow::action<&mappings>, flow::go<Pool>>,
                  flow::node<Pool, flow::action<&pool>, flow::stop>>>;
};

struct KernelPageTable::CommitFlow
{
  struct Context
  {
    KernelPageTable &owner;
    unsigned long cursor, lowest;
    pte_t *entry = nullptr;
    void *page = nullptr;
    bool result = false;
  };

  static bool
  remaining(Context &c) noexcept {
    return c.cursor > c.lowest;
  }

  static void
  walk(Context &c) noexcept {
    c.cursor -= PageSize;
    c.entry = walk_entry(static_cast<PageTable>(c.owner.root_), c.cursor, 1);
  }

  static bool
  entry(Context &c) noexcept {
    return c.entry != nullptr;
  }

  static bool
  mapped(Context &c) noexcept {
    return *c.entry & PTE_V;
  }

  static void
  allocate(Context &c) noexcept {
    c.page = allocator().allocate();
  }

  static bool
  allocated(Context &c) noexcept {
    return c.page != nullptr;
  }

  static void
  publish(Context &c) noexcept {
    *c.entry = PA2PTE(c.page) | PTE_R | PTE_W | PTE_V;
  }

  static void
  success(Context &c) noexcept {
    c.result = true;
  }

  static void
  fence(Context &) noexcept {
    xv::arch::sfence_vma();
  }
  struct Remaining;
  struct Walk;
  struct Mapped;
  struct Allocate;
  struct Publish;
  struct Success;
  struct Fence;
  using Graph = flow::graph<
      Remaining,
      flow::types<flow::node<Remaining, flow::noop,
                             flow::branch<flow::condition<&remaining>, flow::go<Walk>,
                                          flow::go<Success>>>,
                  flow::node<Walk, flow::action<&walk>,
                             flow::branch<flow::condition<&entry>, flow::go<Mapped>,
                                          flow::go<Fence>>>,
                  flow::node<Mapped, flow::noop,
                             flow::branch<flow::condition<&mapped>, flow::go<Remaining>,
                                          flow::go<Allocate>>>,
                  flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&allocated>,
                                          flow::go<Publish>, flow::go<Fence>>>,
                  flow::node<Publish, flow::action<&publish>, flow::go<Remaining>>,
                  flow::node<Success, flow::action<&success>, flow::go<Fence>>,
                  flow::node<Fence, flow::action<&fence>, flow::stop>>>;
};

struct KernelPageTable::ReleaseFlow
{
  struct Context
  {
    KernelPageTable &owner;
    unsigned long cursor, end;
    PageTable leaf = nullptr;
    PageTable middle = nullptr;
  };

  static void
  detach(Context &c) noexcept {
    c.leaf = nullptr;
    c.middle = nullptr;
    auto *root = static_cast<PageTable>(c.owner.root_);
    for (; c.cursor < c.end; c.cursor += 512 * PageSize)
    {
      auto upper = __atomic_load_n(&root[PX(2, c.cursor)], __ATOMIC_ACQUIRE);
      if (!(upper & PTE_V))
      {
        continue;
      }
      auto *middle = reinterpret_cast<PageTable>(PTE2PA(upper));
      auto &directory = middle[PX(1, c.cursor)];
      if (directory & PTE_V)
      {
        c.leaf = reinterpret_cast<PageTable>(PTE2PA(directory));
        __atomic_store_n(&directory, 0UL, __ATOMIC_RELEASE);
      }
      bool empty = true;
      for (unsigned i = 0; i < 512; ++i)
      {
        if (middle[i] & PTE_V)
        {
          empty = false;
          break;
        }
      }
      // A running stack keeps a nonempty child here; its owner pins both
      // levels. Empty directories have no remaining fault-path readers.
      if (empty)
      {
        __atomic_store_n(&root[PX(2, c.cursor)], 0UL, __ATOMIC_RELEASE);
        c.middle = middle;
      }
      if (c.leaf || c.middle)
      {
        xv::arch::sfence_vma();
        return;
      }
    }
  }

  static bool
  detached(Context &c) noexcept {
    return c.leaf || c.middle;
  }

  static void
  free(Context &c) noexcept {
    for (unsigned i = 0; c.leaf && i < 512; ++i)
    {
      if (c.leaf[i] & PTE_V)
      {
        allocator().deallocate(reinterpret_cast<void *>(PTE2PA(c.leaf[i])));
      }
    }
    if (c.leaf)
    {
      allocator().deallocate(c.leaf);
    }
    if (c.middle)
    {
      allocator().deallocate(c.middle);
    }
    c.cursor += 512 * PageSize;
  }
  struct Detach;
  struct Free;
  using Graph =
      flow::graph<Detach,
                  flow::types<flow::node<Detach, flow::action<&detach>,
                                         flow::branch<flow::condition<&detached>,
                                                      flow::go<Free>, flow::stop>>,
                              flow::node<Free, flow::action<&free>, flow::go<Detach>>>>;
};

struct KernelPageTable::AllocateFlow
{
  struct Context
  {
    KernelPageTable &owner;
    unsigned long reservation = 0, address = 0, result = 0;
    bool ok = false;
  };

  static void
  find(Context &c) noexcept {
    c.reservation = c.owner.regions_.allocate_in(2 * KSTACK_BYTES, c.owner.pool_begin_,
                                                 KSTACK_TOP, KSTACK_BYTES);
    if (c.reservation)
    {
      c.address = c.reservation + KSTACK_BYTES;
    }
  }

  static bool
  available(Context &c) noexcept {
    return c.reservation != 0;
  }

  static void
  unreserve(Context &c) noexcept {
    if (!c.owner.regions_.erase(c.reservation, c.reservation + 2 * KSTACK_BYTES))
    {
      xv::print::panic("kernel virtual address pool: lost reservation");
    }
  }

  static bool
  ok(Context &c) noexcept {
    return c.ok;
  }

  static void
  region(Context &c) noexcept {
    c.ok = c.owner.regions_.protect(c.address, c.address + KSTACK_BYTES, PTE_R | PTE_W,
                                    RegionKind::KernelStack, true);
  }

  static void
  erase_guard(Context &c) noexcept {
    unreserve(c);
  }

  static void
  prepare(Context &c) noexcept {
    c.ok = true;
    for (auto cursor = c.address; cursor < c.address + KSTACK_BYTES;
         cursor += 512 * PageSize)
    {
      if (!walk_entry(static_cast<PageTable>(c.owner.root_), cursor, 1))
      {
        c.ok = false;
        break;
      }
    }
  }

  static void
  commit(Context &c) noexcept {
    constexpr unsigned pages = KSTACK_INITIAL_PAGES;
    static_assert(pages && !(pages & (pages - 1)));
    constexpr unsigned order = __builtin_ctz(pages);
    // The stack directory is prepared, but not published. Obtain its initial
    // suffix in one buddy allocation, then transfer ownership to independent
    // pages so trimming/freeing retains its existing per-page contract.
    auto &physical = allocator();
    void *block = physical.supports(order) ? physical.allocate(order) : nullptr;
    if (block)
    {
      if (!physical.split_to_pages(block, order))
      {
        physical.deallocate(block, order);
      }
      else
      {
        auto base = c.address + KSTACK_BYTES - pages * PageSize;
        for (unsigned index = 0; index < pages; ++index)
        {
          auto *entry = walk_entry(static_cast<PageTable>(c.owner.root_),
                                   base + index * PageSize, 0);
          if (!entry || (*entry & PTE_V))
          {
            xv::print::panic("kernel stack: initial directory");
          }
          *entry = PA2PTE(reinterpret_cast<unsigned long>(block) + index * PageSize) |
                   PTE_R | PTE_W | PTE_V;
        }
        xv::arch::sfence_vma();
        c.ok = true;
        return;
      }
    }
    // Fragmentation and allocators supporting only single pages must not
    // turn an otherwise possible stack allocation into a failure.
    c.ok = c.owner.commit_stack(c.address, c.address + KSTACK_BYTES -
                                               pages * PageSize);
  }

  static void
  rollback(Context &c) noexcept {
    c.owner.release_stack(c.address);
    unreserve(c);
  }

  static void
  publish(Context &c) noexcept {
    __atomic_fetch_or(c.owner.stack_directory(c.address), StackPublished,
                      __ATOMIC_RELEASE);
    c.result = c.address;
  }
  struct Find;
  struct Region;
  struct EraseGuard;
  struct Prepare;
  struct Commit;
  struct Rollback;
  struct Publish;
  using Graph = flow::graph<
      Find, flow::types<flow::node<Find, flow::action<&find>,
                                   flow::branch<flow::condition<&available>,
                                                flow::go<Region>, flow::stop>>,
                        flow::node<Region, flow::action<&region>,
                                   flow::branch<flow::condition<&ok>, flow::go<Prepare>,
                                                flow::go<EraseGuard>>>,
                        flow::node<EraseGuard, flow::action<&erase_guard>, flow::stop>,
                        flow::node<Prepare, flow::action<&prepare>,
                                   flow::branch<flow::condition<&ok>, flow::go<Commit>,
                                                flow::go<Rollback>>>,
                        flow::node<Commit, flow::action<&commit>,
                                   flow::branch<flow::condition<&ok>, flow::go<Publish>,
                                                flow::go<Rollback>>>,
                        flow::node<Rollback, flow::action<&rollback>, flow::stop>,
                        flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct KernelPageTable::FreeFlow
{
  struct Context
  {
    KernelPageTable &owner;
    unsigned long address;
  };

  static void
  release(Context &c) noexcept {
    c.owner.release_stack(c.address);
  }

  static void
  erase(Context &c) noexcept {
    c.owner.regions_.erase(c.address - KSTACK_BYTES, c.address + KSTACK_BYTES);
  }

  struct Release;
  struct Erase;
  using Graph = flow::graph<
      Release, flow::types<flow::node<Release, flow::action<&release>, flow::go<Erase>>,
                           flow::node<Erase, flow::action<&erase>, flow::stop>>>;
};

struct KernelPageTable::FaultFlow
{
  struct Context
  {
    KernelPageTable &owner;
    unsigned long base, stack, fault;
    FaultAccess access;
    void *(*frame)();
    unsigned long cursor = 0, lowest = 0;
    pte_t *entry = nullptr;
    void *page = nullptr;
    FaultResult result = FaultResult::OutsideRegion;
  };

  static bool
  valid(Context &c) noexcept {
    return c.owner.in_stack_window(c.base) && c.base % KSTACK_BYTES == 0 &&
           c.base <= KSTACK_TOP - KSTACK_BYTES && c.fault >= c.base &&
           c.fault < c.base + KSTACK_BYTES && c.stack >= c.base &&
           c.stack <= c.base + KSTACK_BYTES;
  }

  static bool
  owned(Context &c) noexcept {
    return c.owner.owns_stack(c.base);
  }

  static bool
  data(Context &c) noexcept {
    return c.access != FaultAccess::Execute;
  }

  static void
  permission(Context &c) noexcept {
    c.result = FaultResult::Permission;
  }

  static void
  lookup(Context &c) noexcept {
    c.entry = walk_entry(static_cast<PageTable>(c.owner.root_), c.fault, 0);
  }

  static bool
  entry(Context &c) noexcept {
    return c.entry != nullptr;
  }

  static bool
  mapped(Context &c) noexcept {
    return *c.entry & PTE_V;
  }

  static void
  existing(Context &c) noexcept {
    if (!PTE_LEAF(*c.entry) || ((*c.entry & PTE_W) && !(*c.entry & PTE_R)))
    {
      c.result = FaultResult::InvalidMapping;
    }
    else
    {
      auto permission = c.access == FaultAccess::Write ? PTE_W : PTE_R;
      c.result = (*c.entry & permission) ? FaultResult::AlreadyMapped
                                         : FaultResult::Permission;
    }
  }

  static void
  start(Context &c) noexcept {
    c.lowest = (c.fault < c.stack ? c.fault : c.stack) & ~(PageSize - 1);
    c.cursor = c.base + KSTACK_BYTES;
  }

  static bool
  remaining(Context &c) noexcept {
    return c.cursor > c.lowest;
  }

  static void
  walk(Context &c) noexcept {
    c.cursor -= PageSize;
    c.entry = walk_entry(static_cast<PageTable>(c.owner.root_), c.cursor, 0);
  }

  static void
  allocate(Context &c) noexcept {
    c.page = c.frame();
  }

  static bool
  allocated(Context &c) noexcept {
    return c.page != nullptr;
  }

  static void
  publish(Context &c) noexcept {
    __atomic_store_n(c.entry, PA2PTE(c.page) | PTE_R | PTE_W | PTE_V, __ATOMIC_RELEASE);
  }

  static void
  invalid(Context &c) noexcept {
    c.result = FaultResult::InvalidMapping;
  }

  static void
  out_of_memory(Context &c) noexcept {
    c.result = FaultResult::OutOfMemory;
  }

  static void
  resolved(Context &c) noexcept {
    c.result = FaultResult::Resolved;
  }

  static void
  fence(Context &) noexcept {
    xv::arch::sfence_vma();
  }
  struct Valid;
  struct Owned;
  struct Data;
  struct Permission;
  struct Lookup;
  struct Mapped;
  struct Existing;
  struct Start;
  struct Remaining;
  struct Walk;
  struct PageMapped;
  struct Allocate;
  struct Publish;
  struct Invalid;
  struct InvalidFence;
  struct OutOfMemory;
  struct Resolved;
  struct Fence;
  using Graph = flow::graph<
      Valid,
      flow::types<
          flow::node<
              Valid, flow::noop,
              flow::branch<flow::condition<&valid>, flow::go<Owned>, flow::stop>>,
          flow::node<Owned, flow::noop,
                     flow::branch<flow::condition<&owned>, flow::go<Data>, flow::stop>>,
          flow::node<Data, flow::noop,
                     flow::branch<flow::condition<&data>, flow::go<Lookup>,
                                  flow::go<Permission>>>,
          flow::node<Permission, flow::action<&permission>, flow::stop>,
          flow::node<Lookup, flow::action<&lookup>,
                     flow::branch<flow::condition<&entry>, flow::go<Mapped>,
                                  flow::go<Invalid>>>,
          flow::node<Mapped, flow::noop,
                     flow::branch<flow::condition<&mapped>, flow::go<Existing>,
                                  flow::go<Start>>>,
          flow::node<Existing, flow::action<&existing>, flow::stop>,
          flow::node<Start, flow::action<&start>, flow::go<Remaining>>,
          flow::node<Remaining, flow::noop,
                     flow::branch<flow::condition<&remaining>, flow::go<Walk>,
                                  flow::go<Resolved>>>,
          flow::node<Walk, flow::action<&walk>,
                     flow::branch<flow::condition<&entry>, flow::go<PageMapped>,
                                  flow::go<InvalidFence>>>,
          flow::node<PageMapped, flow::noop,
                     flow::branch<flow::condition<&mapped>, flow::go<Remaining>,
                                  flow::go<Allocate>>>,
          flow::node<Allocate, flow::action<&allocate>,
                     flow::branch<flow::condition<&allocated>, flow::go<Publish>,
                                  flow::go<OutOfMemory>>>,
          flow::node<Publish, flow::action<&publish>, flow::go<Remaining>>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>,
          flow::node<InvalidFence, flow::action<&invalid>, flow::go<Fence>>,
          flow::node<OutOfMemory, flow::action<&out_of_memory>, flow::go<Fence>>,
          flow::node<Resolved, flow::action<&resolved>, flow::go<Fence>>,
          flow::node<Fence, flow::action<&fence>, flow::stop>>>;
};

void
KernelPageTable::initialize() {
  if (root_)
  {
    xv::print::panic("kernel page table: already initialized");
  }
  InitializeFlow::Context context{*this};
  (void)InitializeFlow::Graph::run<0>(context);
}

void
KernelPageTable::activate_hart() const {
  if (!root_)
  {
    xv::print::panic("kernel page table: not initialized");
  }
  xv::arch::sfence_vma();
  xv::arch::w_satp(MAKE_SATP(root_));
  xv::arch::sfence_vma();
}

bool
KernelPageTable::commit_stack(unsigned long address, unsigned long lowest) {
  lowest &= ~(PageSize - 1);
  CommitFlow::Context context{*this, address + KSTACK_BYTES, lowest};
  (void)CommitFlow::Graph::run<0>(context);
  return context.result;
}

void
KernelPageTable::release_stack(unsigned long address) {
  ReleaseFlow::Context context{*this, address, address + KSTACK_BYTES};
  (void)ReleaseFlow::Graph::run<0>(context);
}

unsigned long
KernelPageTable::allocate_process_stack() {
  xv::sync::ScopedLock guard(stacks_);
  AllocateFlow::Context context{*this};
  (void)AllocateFlow::Graph::run<0>(context);
  return context.result;
}

bool
KernelPageTable::grow_process_stack(unsigned long address, unsigned long lowest) {
  if (!in_stack_window(address) || address % KSTACK_BYTES ||
      address > KSTACK_TOP - KSTACK_BYTES || lowest < address ||
      lowest >= address + KSTACK_BYTES)
  {
    return false;
  }
  xv::sync::ScopedLock guard(stacks_);
  const auto *stack = regions_.find(address);
  const auto *guard_region = regions_.find(address - KSTACK_BYTES);
  if (address < pool_begin_ + KSTACK_BYTES || !stack || !guard_region ||
      stack->begin != address || stack->end != address + KSTACK_BYTES ||
      stack->kind != RegionKind::KernelStack ||
      guard_region->begin != address - KSTACK_BYTES || guard_region->end != address ||
      guard_region->kind != RegionKind::Guard || !owns_stack(address))
  {
    return false;
  }
  return commit_stack(address, lowest);
}

unsigned
KernelPageTable::trim_process_stack(unsigned long address, unsigned long lowest) {
  if (!in_stack_window(address) || address % KSTACK_BYTES ||
      address > KSTACK_TOP - KSTACK_BYTES || lowest < address ||
      lowest >= address + KSTACK_BYTES || !owns_stack(address))
  {
    return 0;
  }
  auto initial = address + KSTACK_BYTES - KSTACK_INITIAL_PAGES * PageSize;
  if (lowest > initial)
  {
    lowest = initial;
  }
  lowest &= ~(PageSize - 1);
  unsigned released = 0;
  // Committed stacks are contiguous suffixes. Stop at the first absent page,
  // so an ordinary user return checks only the page below the retained suffix.
  // Ownership pins the page-table levels; no region or table is removed here.
  while (lowest > address)
  {
    lowest -= PageSize;
    auto *entry = walk_entry(static_cast<PageTable>(root_), lowest, 0);
    if (!entry || !(__atomic_load_n(entry, __ATOMIC_ACQUIRE) & PTE_V))
    {
      break;
    }
    auto previous = __atomic_exchange_n(entry, 0UL, __ATOMIC_ACQ_REL);
    // Flush before returning the frame. Another hart cannot run this process
    // concurrently; a later user/kernel transition flushes its local TLB.
    xv::arch::sfence_vma();
    allocator().deallocate(reinterpret_cast<void *>(PTE2PA(previous)));
    ++released;
  }
  return released;
}

void
KernelPageTable::free_process_stack(unsigned long address) {
  xv::sync::ScopedLock guard(stacks_);
  const auto *stack = regions_.find(address);
  const auto *guard_region = regions_.find(address - KSTACK_BYTES);
  if (address < pool_begin_ + KSTACK_BYTES || address > KSTACK_TOP - KSTACK_BYTES ||
      !stack || !guard_region || stack->begin != address ||
      stack->end != address + KSTACK_BYTES || stack->kind != RegionKind::KernelStack ||
      guard_region->begin != address - KSTACK_BYTES || guard_region->end != address ||
      guard_region->kind != RegionKind::Guard || !owns_stack(address))
  {
    xv::print::panic("kernel stack: unowned region");
  }
  FreeFlow::Context context{*this, address};
  (void)FreeFlow::Graph::run<0>(context);
}

FaultResult
KernelPageTable::handle_stack_fault(unsigned long base, unsigned long stack,
                                    unsigned long fault, FaultAccess access,
                                    void *(*frame)()) {
  FaultFlow::Context context{*this, base, stack, fault, access, frame};
  (void)FaultFlow::Graph::run<0>(context);
  return context.result;
}

bool
KernelPageTable::in_stack_window(unsigned long address) const {
  return pool_begin_ && address >= pool_begin_ && address < KSTACK_TOP;
}

unsigned long *
KernelPageTable::stack_directory(unsigned long address) const {
  if (!root_ || !in_stack_window(address))
  {
    return nullptr;
  }
  auto *table = static_cast<PageTable>(root_);
  auto upper = __atomic_load_n(&table[PX(2, address)], __ATOMIC_ACQUIRE);
  if (!(upper & PTE_V) || PTE_LEAF(upper))
  {
    return nullptr;
  }
  auto *middle = reinterpret_cast<PageTable>(PTE2PA(upper));
  return &middle[PX(1, address)];
}

bool
KernelPageTable::owns_stack(unsigned long address) const {
  auto *directory = stack_directory(address);
  if (!directory)
  {
    return false;
  }
  auto entry = __atomic_load_n(directory, __ATOMIC_ACQUIRE);
  return (entry & (PTE_V | StackPublished)) == (PTE_V | StackPublished) &&
         !PTE_LEAF(entry);
}

bool
KernelPageTable::region(unsigned long address, VirtualRegion &result) {
  xv::sync::ScopedLock guard(stacks_);
  auto *found = regions_.find(address);
  if (!found)
  {
    return false;
  }
  result = *found;
  return true;
}

PageTableStatistics
KernelPageTable::statistics() {
  // Serializes against publication and reclamation of stack leaf tables.
  // Lock-free stack faults may add leaf mappings, so leaf counts are advisory.
  xv::sync::ScopedLock guard(stacks_);
  LargestPagePolicy policy;
  Sv39PageTable view(static_cast<PageTable>(root_), allocator(), policy);
  return view.statistics();
}

const void *
KernelPageTable::root() const {
  return root_;
}

unsigned long
KernelPageTable::allocate_module(unsigned long bytes) {
  if (!bytes || bytes % PageSize || bytes > 1024 * 1024)
  {
    return 0;
  }
  xv::sync::ScopedLock guard(stacks_);
  auto base = regions_.allocate_in(bytes, 1UL << 36, (1UL << 36) + (1UL << 30),
                                   PageSize, PTE_R | PTE_W, RegionKind::Image);
  if (!base)
  {
    return 0;
  }
  unsigned long done = 0;
  for (; done < bytes; done += PageSize)
  {
    auto *entry = walk_entry(static_cast<PageTable>(root_), base + done, 1);
    auto *page = entry ? allocator().allocate() : nullptr;
    if (!page)
    {
      break;
    }
    xv::runtime::fill(page, 0, PageSize);
    __atomic_store_n(entry, PA2PTE(page) | PTE_R | PTE_W | PTE_V, __ATOMIC_RELEASE);
  }
  if (done != bytes)
  {
    while (done)
    {
      done -= PageSize;
      auto *entry = walk_entry(static_cast<PageTable>(root_), base + done, 0);
      auto old = __atomic_exchange_n(entry, 0UL, __ATOMIC_ACQ_REL);
      allocator().deallocate(reinterpret_cast<void *>(PTE2PA(old)));
    }
    regions_.release(base);
    base = 0;
  }
  xv::arch::sfence_vma();
  return base;
}

bool
KernelPageTable::protect_module(unsigned long base, unsigned long offset,
                                unsigned permissions) {
  if (offset % PageSize || !(permissions & PTE_R) ||
      (permissions & ~(PTE_R | PTE_W | PTE_X)) ||
      ((permissions & PTE_W) && (permissions & PTE_X)))
  {
    return false;
  }
  xv::sync::ScopedLock guard(stacks_);
  auto *region = regions_.find(base);
  if (!region || region->begin != base || region->kind != RegionKind::Image ||
      offset >= region->end - base)
  {
    return false;
  }
  auto *entry = walk_entry(static_cast<PageTable>(root_), base + offset, 0);
  if (!entry || !(*entry & PTE_V))
  {
    return false;
  }
  auto old = *entry;
  __atomic_store_n(entry, (old & ~(PTE_R | PTE_W | PTE_X)) | permissions,
                   __ATOMIC_RELEASE);
  xv::arch::sfence_vma();
  return true;
}

void
KernelPageTable::discard_module(unsigned long base) {
  xv::sync::ScopedLock guard(stacks_);
  auto *region = regions_.find(base);
  if (!region || region->begin != base || region->kind != RegionKind::Image)
  {
    return;
  }
  for (auto at = base; at < region->end; at += PageSize)
  {
    auto *entry = walk_entry(static_cast<PageTable>(root_), at, 0);
    if (entry && (*entry & PTE_V))
    {
      auto old = __atomic_exchange_n(entry, 0UL, __ATOMIC_ACQ_REL);
      xv::arch::sfence_vma();
      allocator().deallocate(reinterpret_cast<void *>(PTE2PA(old)));
    }
  }
  regions_.release(base);
}

KernelPageTable &
kernel_page_table() {
  return shared_kernel_page_table;
}
} // namespace xv::memory
