module;

#include "kernel/support/param.h"
#include "kernel/support/types.h"
#include "kernel/support/flow.h"
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
    auto entry = __atomic_load_n(pte, __ATOMIC_ACQUIRE);
    if (entry & PTE_V)
    {
      if (entry & (PTE_R | PTE_W | PTE_X))
      {
        if (leaflevel)
        {
          *leaflevel = level;
        }
        return pte;
      }
      pagetable = (pagetable_t)PTE2PA(entry);
    }
    else
    {
      if (!alloc || (pagetable = (pde_t *)xv::memory::allocator().allocate()) == 0)
      {
        return 0;
      }
      xv::runtime::fill(pagetable, 0, PGSIZE);
      // Kernel upper directories can now be installed lazily. Publish only
      // after clearing the child; fault walkers acquire the same PTE.
      __atomic_store_n(pte, PA2PTE(pagetable) | PTE_V, __ATOMIC_RELEASE);
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
namespace
{
namespace flow = xv::flow;

struct SuperMapFlow
{
  struct Context
  {
    pagetable_t table;
    uint64 address, physical;
    int permissions;
    int level = 0, result = -1;
    pte_t *pte = nullptr;
  };

  static void walk(Context &c) noexcept
  {
    c.pte = walkto(c.table, c.address, 1, 1, &c.level);
  }

  static bool found(Context &c) noexcept
  {
    return c.pte != nullptr;
  }

  static bool level(Context &c) noexcept
  {
    return c.level == 1;
  }

  static void invalid(Context &) noexcept
  {
    xv::print::panic("supermap: level");
  }

  static bool existing(Context &c) noexcept
  {
    return *c.pte & PTE_V;
  }

  static void reclaim(Context &c) noexcept
  {
    if (PTE_LEAF(*c.pte))
    {
      xv::print::panic("supermap: remap");
    }
    auto *child = reinterpret_cast<pagetable_t>(PTE2PA(*c.pte));
    for (int i = 0; i < 512; ++i)
    {
      if (child[i] & PTE_V)
      {
        xv::print::panic("supermap: occupied");
      }
    }
    allocator().deallocate(child);
  }

  static void publish(Context &c) noexcept
  {
    *c.pte = PA2PTE(c.physical) | c.permissions | PTE_V;
    c.result = 0;
  }
  struct Walk;
  struct Level;
  struct Invalid;
  struct Existing;
  struct Reclaim;
  struct Publish;
  using Graph = flow::graph<
      Walk, flow::types<flow::node<Walk, flow::action<&walk>,
                                   flow::branch<flow::condition<&found>,
                                                flow::go<Level>, flow::stop>>,
                        flow::node<Level, flow::noop,
                                   flow::branch<flow::condition<&level>,
                                                flow::go<Existing>, flow::go<Invalid>>>,
                        flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                        flow::node<Existing, flow::noop,
                                   flow::branch<flow::condition<&existing>,
                                                flow::go<Reclaim>, flow::go<Publish>>>,
                        flow::node<Reclaim, flow::action<&reclaim>, flow::go<Publish>>,
                        flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

} // namespace

static int supermap(pagetable_t pagetable, uint64 va, uint64 pa, int perm)
{
  if ((va | pa) % SUPERPGSIZE != 0)
  {
    xv::print::panic("supermap: alignment");
  }
  SuperMapFlow::Context context{pagetable, va, pa, perm};
  (void)SuperMapFlow::Graph::run<0>(context);
  return context.result;
}

// Split without copying data. Under memory pressure, use a page that this
// unmap is about to discard as the new page table instead of failing shrink.
namespace
{
namespace flow = xv::flow;

struct SuperDemoteFlow
{
  struct Context
  {
    pte_t *pte;
    uint64 address, physical;
    uint flags;
    int do_free;
    pagetable_t child = nullptr;
    int recycled = -1, result = -1;
  };

  static void allocate(Context &c) noexcept
  {
    c.child = static_cast<pagetable_t>(allocator().allocate());
  }

  static bool allocated(Context &c) noexcept
  {
    return c.child != nullptr;
  }

  static bool may_recycle(Context &c) noexcept
  {
    return c.do_free != 0;
  }

  static void recycle(Context &c) noexcept
  {
    c.recycled = PX(0, c.address);
    c.child = reinterpret_cast<pagetable_t>(c.physical +
                                            static_cast<uint64>(c.recycled) * PGSIZE);
  }

  static void split(Context &c) noexcept
  {
    if (!allocator().split_to_pages(reinterpret_cast<void *>(c.physical),
                                    SuperpageOrder))
    {
      xv::print::panic("unsupported page split");
    }
  }

  static void fill(Context &c) noexcept
  {
    for (int i = 0; i < 512; ++i)
    {
      c.child[i] = i == c.recycled
                       ? 0
                       : PA2PTE(c.physical + static_cast<uint64>(i) * PGSIZE) | c.flags;
    }
  }

  static void publish(Context &c) noexcept
  {
    *c.pte = PA2PTE(c.child) | PTE_V;
    c.result = 0;
  }
  struct Allocate;
  struct RecycleCheck;
  struct Recycle;
  struct Split;
  struct Fill;
  struct Publish;
  using Graph = flow::graph<
      Allocate,
      flow::types<flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&allocated>, flow::go<Split>,
                                          flow::go<RecycleCheck>>>,
                  flow::node<RecycleCheck, flow::noop,
                             flow::branch<flow::condition<&may_recycle>,
                                          flow::go<Recycle>, flow::stop>>,
                  flow::node<Recycle, flow::action<&recycle>, flow::go<Split>>,
                  flow::node<Split, flow::action<&split>, flow::go<Fill>>,
                  flow::node<Fill, flow::action<&fill>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

} // namespace

static int superdemote(pte_t *pte, uint64 va, int do_free)
{
  SuperDemoteFlow::Context context{pte, va, PTE2PA(*pte),
                                   static_cast<uint>(PTE_FLAGS(*pte)), do_free};
  (void)SuperDemoteFlow::Graph::run<0>(context);
  return context.result;
}

// pgaccess needs independent access bits for each 4 KiB page. A superpage
// has only one A bit, so split any covered superpages before sampling.
namespace
{
namespace flow = xv::flow;

struct DemoteUserFlow
{
  struct Context
  {
    pagetable_t table;
    uint64 address, end;
    pte_t *pte = nullptr;
    int level = 0, result = -1;
    bool ok = false;
  };

  static bool remaining(Context &c) noexcept
  {
    return c.address < c.end;
  }

  static void walk(Context &c) noexcept
  {
    c.pte = walkto(c.table, c.address, 0, 0, &c.level);
  }

  static bool mapped(Context &c) noexcept
  {
    return c.pte && (*c.pte & PTE_V);
  }

  static bool huge(Context &c) noexcept
  {
    return c.level == 1;
  }

  static void split(Context &c) noexcept
  {
    c.ok = superdemote(c.pte, c.address, 0) >= 0;
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void advance(Context &c) noexcept
  {
    c.address = SUPERPGROUNDDOWN(c.address) + SUPERPGSIZE;
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Remaining;
  struct Walk;
  struct Huge;
  struct Split;
  struct Advance;
  struct Success;
  using Graph = flow::graph<
      Remaining,
      flow::types<
          flow::node<Remaining, flow::noop,
                     flow::branch<flow::condition<&remaining>, flow::go<Walk>,
                                  flow::go<Success>>>,
          flow::node<
              Walk, flow::action<&walk>,
              flow::branch<flow::condition<&mapped>, flow::go<Huge>, flow::stop>>,
          flow::node<
              Huge, flow::noop,
              flow::branch<flow::condition<&huge>, flow::go<Split>, flow::go<Advance>>>,
          flow::node<Split, flow::action<&split>,
                     flow::branch<flow::condition<&ok>, flow::go<Advance>, flow::stop>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Remaining>>,
          flow::node<Success, flow::action<&success>, flow::stop>>>;
};

} // namespace

int demote_user_pages(pagetable_t pagetable, uint64 va, uint64 npages)
{
  if (va % PGSIZE != 0 || va >= MAXVA || npages > (MAXVA - va) / PGSIZE)
  {
    return -1;
  }
  DemoteUserFlow::Context context{pagetable, va, va + npages * PGSIZE};
  (void)DemoteUserFlow::Graph::run<0>(context);
  return context.result;
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

namespace
{
namespace flow = xv::flow;

struct MapUserFlow
{
  struct Context
  {
    pagetable_t table;
    uint64 address, last, physical;
    int permissions;
    pte_t *pte = nullptr;
    int result = -1;
  };

  static void walk(Context &c) noexcept
  {
    c.pte = walk_entry(c.table, c.address, 1);
  }

  static bool found(Context &c) noexcept
  {
    return c.pte != nullptr;
  }

  static void publish(Context &c) noexcept
  {
    if (*c.pte & PTE_V)
    {
      xv::print::panic("map_user_pages: remap");
    }
    *c.pte = PA2PTE(c.physical) | c.permissions | PTE_V;
  }

  static bool last(Context &c) noexcept
  {
    return c.address == c.last;
  }

  static void advance(Context &c) noexcept
  {
    c.address += PGSIZE;
    c.physical += PGSIZE;
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Walk;
  struct Publish;
  struct Advance;
  struct Success;
  using Graph = flow::graph<
      Walk, flow::types<flow::node<Walk, flow::action<&walk>,
                                   flow::branch<flow::condition<&found>,
                                                flow::go<Publish>, flow::stop>>,
                        flow::node<Publish, flow::action<&publish>,
                                   flow::branch<flow::condition<&last>,
                                                flow::go<Success>, flow::go<Advance>>>,
                        flow::node<Advance, flow::action<&advance>, flow::go<Walk>>,
                        flow::node<Success, flow::action<&success>, flow::stop>>>;
};

} // namespace

int map_user_pages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
  if (va % PGSIZE)
  {
    xv::print::panic("map_user_pages: va not aligned");
  }
  if (size % PGSIZE)
  {
    xv::print::panic("map_user_pages: size not aligned");
  }
  if (!size)
  {
    xv::print::panic("map_user_pages: size");
  }
  MapUserFlow::Context context{pagetable, va, va + size - PGSIZE, pa, perm};
  (void)MapUserFlow::Graph::run<0>(context);
  return context.result;
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
namespace
{
namespace flow = xv::flow;

struct UnmapUserFlow
{
  struct Context
  {
    pagetable_t table;
    uint64 address, end;
    int do_free;
    pte_t *pte = nullptr;
    int step = PGSIZE, level = 0;
  };

  static void scan(Context &c) noexcept
  {
    for (; c.address < c.end; c.address += c.step)
    {
      c.step = PGSIZE;
      c.pte = walkto(c.table, c.address, 0, 0, &c.level);
      if (c.pte && (*c.pte & PTE_V))
      {
        return;
      }
    }
    c.pte = nullptr;
  }

  static bool found(Context &c) noexcept
  {
    return c.pte != nullptr;
  }
#ifdef LAB_PGTBL
  static bool huge(Context &c) noexcept
  {
    return c.level == 1;
  }

  static bool whole(Context &c) noexcept
  {
    return !(c.address % SUPERPGSIZE) && c.end - c.address >= SUPERPGSIZE;
  }

  static void remove_huge(Context &c) noexcept
  {
    if (c.do_free)
    {
      allocator().deallocate(reinterpret_cast<void *>(PTE2PA(*c.pte)), SuperpageOrder);
    }
    *c.pte = 0;
    c.step = SUPERPGSIZE;
  }

  static void split(Context &c) noexcept
  {
    if (superdemote(c.pte, c.address, c.do_free) < 0)
    {
      xv::print::panic("unmap_user_pages: split");
    }
    c.pte = walk_entry(c.table, c.address, 0);
  }

  static bool mapped(Context &c) noexcept
  {
    return *c.pte & PTE_V;
  }
#endif
  static void remove_leaf(Context &c) noexcept
  {
    if (PTE_FLAGS(*c.pte) == PTE_V)
    {
      xv::print::panic("unmap_user_pages: not a leaf");
    }
    if (c.do_free)
    {
      allocator().release_page(reinterpret_cast<void *>(PTE2PA(*c.pte)));
    }
    *c.pte = 0;
  }

  static void advance(Context &c) noexcept
  {
    c.address += c.step;
  }
#ifdef LAB_PGTBL
  struct Scan;
  struct Huge;
  struct Whole;
  struct RemoveHuge;
  struct Split;
  struct Leaf;
  struct Advance;
  using Graph = flow::graph<
      Scan,
      flow::types<
          flow::node<Scan, flow::action<&scan>,
                     flow::branch<flow::condition<&found>, flow::go<Huge>, flow::stop>>,
          flow::node<
              Huge, flow::noop,
              flow::branch<flow::condition<&huge>, flow::go<Whole>, flow::go<Leaf>>>,
          flow::node<Whole, flow::noop,
                     flow::branch<flow::condition<&whole>, flow::go<RemoveHuge>,
                                  flow::go<Split>>>,
          flow::node<RemoveHuge, flow::action<&remove_huge>, flow::go<Advance>>,
          flow::node<Split, flow::action<&split>,
                     flow::branch<flow::condition<&mapped>, flow::go<Leaf>,
                                  flow::go<Advance>>>,
          flow::node<Leaf, flow::action<&remove_leaf>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Scan>>>>;

#else
  struct Scan;
  struct Leaf;
  struct Advance;
  using Graph = flow::graph<
      Scan,
      flow::types<
          flow::node<Scan, flow::action<&scan>,
                     flow::branch<flow::condition<&found>, flow::go<Leaf>, flow::stop>>,
          flow::node<Leaf, flow::action<&remove_leaf>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Scan>>>>;

#endif
};

} // namespace

void unmap_user_pages(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
{
  if (va % PGSIZE)
  {
    xv::print::panic("unmap_user_pages: not aligned");
  }
  UnmapUserFlow::Context context{pagetable, va, va + npages * PGSIZE, do_free};
  (void)UnmapUserFlow::Graph::run<0>(context);
}

// Allocate PTEs and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
namespace
{
namespace flow = xv::flow;

struct AllocateUserFlow
{
  struct Context
  {
    pagetable_t table;
    uint64 old_size, new_size, address;
    int permissions, step = PGSIZE;
    char *memory = nullptr;
    bool ok = false;
    uint64 result = 0;
  };

  static bool remaining(Context &c) noexcept
  {
    return c.address < c.new_size;
  }

  static void choose(Context &c) noexcept
  {
    c.step = PGSIZE;
#ifdef LAB_PGTBL
    c.memory = nullptr;
    if (!(c.address % SUPERPGSIZE) && c.new_size - c.address >= SUPERPGSIZE)
    {
      c.memory = static_cast<char *>(allocator().allocate(SuperpageOrder));
    }
#endif
  }

  static bool allocated(Context &c) noexcept
  {
    return c.memory != nullptr;
  }
#ifdef LAB_PGTBL
  static void map_huge(Context &c) noexcept
  {
    xv::runtime::fill(c.memory, 0, SUPERPGSIZE);
    c.ok = supermap(c.table, c.address, reinterpret_cast<uint64>(c.memory),
                    PTE_R | PTE_U | c.permissions) >= 0;
  }

  static void huge_step(Context &c) noexcept
  {
    c.step = SUPERPGSIZE;
  }

  static void drop_huge(Context &c) noexcept
  {
    allocator().deallocate(c.memory, SuperpageOrder);
  }
#endif
  static void page(Context &c) noexcept
  {
    c.memory = static_cast<char *>(allocator().allocate());
  }

  static void zero(Context &c) noexcept
  {
#ifndef LAB_SYSCALL
    xv::runtime::fill(c.memory, 0, c.step);
#else
    (void)c;
#endif
  }

  static void map(Context &c) noexcept
  {
    c.ok =
        map_user_pages(c.table, c.address, c.step, reinterpret_cast<uint64>(c.memory),
                       PTE_R | PTE_U | c.permissions) == 0;
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void drop(Context &c) noexcept
  {
    allocator().deallocate(c.memory);
  }

  static void rollback(Context &c) noexcept
  {
    deallocate_user_pages(c.table, c.address, c.old_size);
  }

  static void advance(Context &c) noexcept
  {
    c.address += c.step;
  }

  static void success(Context &c) noexcept
  {
    c.result = c.new_size;
  }
#ifdef LAB_PGTBL
  struct Remaining;
  struct Choose;
  struct HugeMap;
  struct HugeStep;
  struct DropHuge;
  struct Page;
  struct Zero;
  struct Map;
  struct Drop;
  struct Rollback;
  struct Advance;
  struct Success;
  using Graph = flow::graph<
      Remaining,
      flow::types<flow::node<Remaining, flow::noop,
                             flow::branch<flow::condition<&remaining>, flow::go<Choose>,
                                          flow::go<Success>>>,
                  flow::node<Choose, flow::action<&choose>,
                             flow::branch<flow::condition<&allocated>,
                                          flow::go<HugeMap>, flow::go<Page>>>,
                  flow::node<HugeMap, flow::action<&map_huge>,
                             flow::branch<flow::condition<&ok>, flow::go<HugeStep>,
                                          flow::go<DropHuge>>>,
                  flow::node<HugeStep, flow::action<&huge_step>, flow::go<Advance>>,
                  flow::node<DropHuge, flow::action<&drop_huge>, flow::go<Rollback>>,
                  flow::node<Page, flow::action<&page>,
                             flow::branch<flow::condition<&allocated>, flow::go<Zero>,
                                          flow::go<Rollback>>>,
                  flow::node<Zero, flow::action<&zero>, flow::go<Map>>,
                  flow::node<Map, flow::action<&map>,
                             flow::branch<flow::condition<&ok>, flow::go<Advance>,
                                          flow::go<Drop>>>,
                  flow::node<Drop, flow::action<&drop>, flow::go<Rollback>>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Remaining>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;

#else
  struct Remaining;
  struct Choose;
  struct Page;
  struct Zero;
  struct Map;
  struct Drop;
  struct Rollback;
  struct Advance;
  struct Success;
  using Graph = flow::graph<
      Remaining,
      flow::types<flow::node<Remaining, flow::noop,
                             flow::branch<flow::condition<&remaining>, flow::go<Choose>,
                                          flow::go<Success>>>,
                  flow::node<Choose, flow::action<&choose>, flow::go<Page>>,
                  flow::node<Page, flow::action<&page>,
                             flow::branch<flow::condition<&allocated>, flow::go<Zero>,
                                          flow::go<Rollback>>>,
                  flow::node<Zero, flow::action<&zero>, flow::go<Map>>,
                  flow::node<Map, flow::action<&map>,
                             flow::branch<flow::condition<&ok>, flow::go<Advance>,
                                          flow::go<Drop>>>,
                  flow::node<Drop, flow::action<&drop>, flow::go<Rollback>>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Remaining>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;

#endif
};

} // namespace

uint64 allocate_user_pages(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm)
{
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
  AllocateUserFlow::Context context{pagetable, oldsz, newsz, oldsz, xperm};
  (void)AllocateUserFlow::Graph::run<0>(context);
  return context.result;
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
namespace
{
namespace flow = xv::flow;

struct CloneUserFlow
{
  struct Context
  {
    pagetable_t source, target;
    uint64 size, address = 0, physical = 0;
    uint flags = 0;
    pte_t *pte = nullptr;
    int level = 0, result = -1;
    bool ok = false;
  };

  static void scan(Context &c) noexcept
  {
    for (; c.address < c.size; c.address += PGSIZE)
    {
      c.pte = walkto(c.source, c.address, 0, 0, &c.level);
      if (c.pte && (*c.pte & PTE_V))
      {
        return;
      }
    }
    c.pte = nullptr;
  }

  static bool found(Context &c) noexcept
  {
    return c.pte != nullptr;
  }
#ifdef LAB_PGTBL
  static bool huge(Context &c) noexcept
  {
    return c.level == 1;
  }

  static void split(Context &c) noexcept
  {
    c.ok = superdemote(c.pte, c.address, 0) >= 0;
    if (c.ok)
    {
      c.pte = walk_entry(c.source, c.address, 0);
      c.level = 0;
    }
  }
#endif
  static bool level(Context &c) noexcept
  {
    return c.level == 0;
  }

  static void share(Context &c) noexcept
  {
    c.physical = PTE2PA(*c.pte);
    c.flags = PTE_FLAGS(*c.pte);
    if ((c.flags & (PTE_U | PTE_W)) == (PTE_U | PTE_W))
    {
      c.flags = (c.flags & ~PTE_W) | PTE_COW;
    }
  }

  static void map(Context &c) noexcept
  {
    c.ok = map_user_pages(c.target, c.address, PGSIZE, c.physical, c.flags) >= 0;
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void retain(Context &c) noexcept
  {
    allocator().retain_page(reinterpret_cast<void *>(c.physical));
    *c.pte = PA2PTE(c.physical) | c.flags;
  }

  static void advance(Context &c) noexcept
  {
    c.address += PGSIZE;
  }

  static void failure(Context &c) noexcept
  {
    unmap_user_pages(c.target, 0, c.address / PGSIZE, 1);
    xv::arch::sfence_vma();
  }

  static void success(Context &c) noexcept
  {
    xv::arch::sfence_vma();
    c.result = 0;
  }
#ifdef LAB_PGTBL
  struct Scan;
  struct Huge;
  struct Split;
  struct Level;
  struct Share;
  struct Map;
  struct Retain;
  struct Advance;
  struct Failure;
  struct Success;
  using Graph = flow::graph<
      Scan, flow::types<flow::node<Scan, flow::action<&scan>,
                                   flow::branch<flow::condition<&found>, flow::go<Huge>,
                                                flow::go<Success>>>,
                        flow::node<Huge, flow::noop,
                                   flow::branch<flow::condition<&huge>, flow::go<Split>,
                                                flow::go<Level>>>,
                        flow::node<Split, flow::action<&split>,
                                   flow::branch<flow::condition<&ok>, flow::go<Level>,
                                                flow::go<Failure>>>,
                        flow::node<Level, flow::noop,
                                   flow::branch<flow::condition<&level>,
                                                flow::go<Share>, flow::go<Failure>>>,
                        flow::node<Share, flow::action<&share>, flow::go<Map>>,
                        flow::node<Map, flow::action<&map>,
                                   flow::branch<flow::condition<&ok>, flow::go<Retain>,
                                                flow::go<Failure>>>,
                        flow::node<Retain, flow::action<&retain>, flow::go<Advance>>,
                        flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
                        flow::node<Failure, flow::action<&failure>, flow::stop>,
                        flow::node<Success, flow::action<&success>, flow::stop>>>;

#else
  struct Scan;
  struct Level;
  struct Share;
  struct Map;
  struct Retain;
  struct Advance;
  struct Failure;
  struct Success;
  using Graph = flow::graph<
      Scan, flow::types<flow::node<Scan, flow::action<&scan>,
                                   flow::branch<flow::condition<&found>,
                                                flow::go<Level>, flow::go<Success>>>,
                        flow::node<Level, flow::noop,
                                   flow::branch<flow::condition<&level>,
                                                flow::go<Share>, flow::go<Failure>>>,
                        flow::node<Share, flow::action<&share>, flow::go<Map>>,
                        flow::node<Map, flow::action<&map>,
                                   flow::branch<flow::condition<&ok>, flow::go<Retain>,
                                                flow::go<Failure>>>,
                        flow::node<Retain, flow::action<&retain>, flow::go<Advance>>,
                        flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
                        flow::node<Failure, flow::action<&failure>, flow::stop>,
                        flow::node<Success, flow::action<&success>, flow::stop>>>;

#endif
};

} // namespace

int clone_user_pages(pagetable_t old, pagetable_t new_table, uint64 sz)
{
  CloneUserFlow::Context context{old, new_table, sz};
  (void)CloneUserFlow::Graph::run<0>(context);
  return context.result;
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
  return access == FaultAccess::Write     ? PTE_W
         : access == FaultAccess::Execute ? PTE_X
                                          : PTE_R;
}

const VirtualRegion *accessible_region(const VirtualAddressPool &regions, uint64 size,
                                       uint64 address, FaultAccess access)
{
  auto *region = regions.find(address);
  auto permissions = permission_for(access) | PTE_U;
  if (!region || (region->permissions & permissions) != permissions ||
      (address >= size && region->kind != RegionKind::Special))
  {
    return nullptr;
  }
  return region;
}
} // namespace

namespace
{
namespace flow = xv::flow;

struct FaultUserFlow
{
  struct Context
  {
    pagetable_t table;
    uint64 size;
    const VirtualAddressPool &regions;
    uint64 address;
    FaultAccess access;
    const VirtualRegion *region = nullptr;
    pte_t *pte = nullptr;
    int level = 0;
    uint64 physical = 0;
    uint flags = 0;
    void *page = nullptr;
    bool ok = false;
    FaultOutcome result{FaultResult::OutsideRegion};
  };

  static void region(Context &c) noexcept
  {
    c.region = c.regions.find(c.address);
  }

  static bool inside(Context &c) noexcept
  {
    return c.region && (c.address < c.size || c.region->kind == RegionKind::Special);
  }

  static bool accessible(Context &c) noexcept
  {
    return accessible_region(c.regions, c.size, c.address, c.access) != nullptr;
  }

  static void permission(Context &c) noexcept
  {
    c.result = {FaultResult::Permission};
  }

  static void walk(Context &c) noexcept
  {
    c.pte = walkto(c.table, c.address, 0, 0, &c.level);
  }

  static bool mapped(Context &c) noexcept
  {
    return c.pte && (*c.pte & PTE_V);
  }

  static bool leaf(Context &c) noexcept
  {
    return PTE_LEAF(*c.pte);
  }

  static void invalid(Context &c) noexcept
  {
    c.result = {FaultResult::InvalidMapping};
  }

  static bool user(Context &c) noexcept
  {
    return *c.pte & PTE_U;
  }

  static bool cow(Context &c) noexcept
  {
    return c.access == FaultAccess::Write && (*c.pte & PTE_COW) && !(*c.pte & PTE_W);
  }

  static void existing(Context &c) noexcept
  {
    c.result = {(*c.pte & permission_for(c.access)) ? FaultResult::AlreadyMapped
                                                    : FaultResult::Permission};
  }

  static bool small(Context &c) noexcept
  {
    return c.level == 0;
  }

  static void shared(Context &c) noexcept
  {
    c.physical = PTE2PA(*c.pte);
    c.flags = (PTE_FLAGS(*c.pte) | PTE_W) & ~PTE_COW;
  }

  static bool sole(Context &c) noexcept
  {
    return allocator().page_references(reinterpret_cast<void *>(c.physical)) == 1;
  }

  static void promote(Context &c) noexcept
  {
    *c.pte = PA2PTE(c.physical) | c.flags;
    xv::arch::sfence_vma();
    c.result = {FaultResult::Resolved, c.physical};
  }

  static void allocate(Context &c) noexcept
  {
    c.page = allocator().allocate();
  }

  static bool allocated(Context &c) noexcept
  {
    return c.page != nullptr;
  }

  static void oom(Context &c) noexcept
  {
    c.result = {FaultResult::OutOfMemory};
  }

  static void copy(Context &c) noexcept
  {
    xv::runtime::move(c.page, reinterpret_cast<void *>(c.physical), PGSIZE);
  }

  static void publish_cow(Context &c) noexcept
  {
    *c.pte = PA2PTE(c.page) | c.flags;
    xv::arch::sfence_vma();
  }

  static void release(Context &c) noexcept
  {
    allocator().release_page(reinterpret_cast<void *>(c.physical));
    c.result = {FaultResult::Resolved, reinterpret_cast<uint64>(c.page)};
  }

  static bool demand(Context &c) noexcept
  {
    return c.region->demand && c.access != FaultAccess::Execute;
  }

  static void zero(Context &c) noexcept
  {
    xv::runtime::fill(c.page, 0, PGSIZE);
  }

  static void map(Context &c) noexcept
  {
    c.ok = map_user_pages(c.table, PGROUNDDOWN(c.address), PGSIZE,
                          reinterpret_cast<uint64>(c.page), c.region->permissions) >= 0;
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void rollback(Context &c) noexcept
  {
    allocator().deallocate(c.page);
  }

  static void resolved(Context &c) noexcept
  {
    xv::arch::sfence_vma();
    c.result = {FaultResult::Resolved, reinterpret_cast<uint64>(c.page)};
  }
  struct Region;
  struct Access;
  struct Permission;
  struct Walk;
  struct Leaf;
  struct Invalid;
  struct User;
  struct Cow;
  struct Existing;
  struct Small;
  struct Shared;
  struct Promote;
  struct AllocateCow;
  struct Copy;
  struct PublishCow;
  struct Release;
  struct Demand;
  struct AllocateDemand;
  struct Zero;
  struct Map;
  struct Rollback;
  struct Oom;
  struct Resolved;
  using Graph = flow::graph<
      Region,
      flow::types<
          flow::node<
              Region, flow::action<&region>,
              flow::branch<flow::condition<&inside>, flow::go<Access>, flow::stop>>,
          flow::node<Access, flow::noop,
                     flow::branch<flow::condition<&accessible>, flow::go<Walk>,
                                  flow::go<Permission>>>,
          flow::node<Permission, flow::action<&permission>, flow::stop>,
          flow::node<
              Walk, flow::action<&walk>,
              flow::branch<flow::condition<&mapped>, flow::go<Leaf>, flow::go<Demand>>>,
          flow::node<
              Leaf, flow::noop,
              flow::branch<flow::condition<&leaf>, flow::go<User>, flow::go<Invalid>>>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>,
          flow::node<User, flow::noop,
                     flow::branch<flow::condition<&user>, flow::go<Cow>,
                                  flow::go<Permission>>>,
          flow::node<
              Cow, flow::noop,
              flow::branch<flow::condition<&cow>, flow::go<Small>, flow::go<Existing>>>,
          flow::node<Existing, flow::action<&existing>, flow::stop>,
          flow::node<Small, flow::noop,
                     flow::branch<flow::condition<&small>, flow::go<Shared>,
                                  flow::go<Invalid>>>,
          flow::node<Shared, flow::action<&shared>,
                     flow::branch<flow::condition<&sole>, flow::go<Promote>,
                                  flow::go<AllocateCow>>>,
          flow::node<Promote, flow::action<&promote>, flow::stop>,
          flow::node<
              AllocateCow, flow::action<&allocate>,
              flow::branch<flow::condition<&allocated>, flow::go<Copy>, flow::go<Oom>>>,
          flow::node<Copy, flow::action<&copy>, flow::go<PublishCow>>,
          flow::node<PublishCow, flow::action<&publish_cow>, flow::go<Release>>,
          flow::node<Release, flow::action<&release>, flow::stop>,
          flow::node<Demand, flow::noop,
                     flow::branch<flow::condition<&demand>, flow::go<AllocateDemand>,
                                  flow::stop>>,
          flow::node<
              AllocateDemand, flow::action<&allocate>,
              flow::branch<flow::condition<&allocated>, flow::go<Zero>, flow::go<Oom>>>,
          flow::node<Zero, flow::action<&zero>, flow::go<Map>>,
          flow::node<Map, flow::action<&map>,
                     flow::branch<flow::condition<&ok>, flow::go<Resolved>,
                                  flow::go<Rollback>>>,
          flow::node<Rollback, flow::action<&rollback>, flow::go<Oom>>,
          flow::node<Oom, flow::action<&oom>, flow::stop>,
          flow::node<Resolved, flow::action<&resolved>, flow::stop>>>;
};

} // namespace

FaultOutcome resolve_user_fault(pagetable_t table, uint64 size,
                                const VirtualAddressPool &regions, uint64 address,
                                FaultAccess access)
{
  if (address >= MAXVA)
  {
    return {FaultResult::OutsideRegion};
  }
  FaultUserFlow::Context context{table, size, regions, address, access};
  (void)FaultUserFlow::Graph::run<0>(context);
  return context.result;
}

namespace
{
uint64 user_access(pagetable_t table, uint64 size, const VirtualAddressPool &regions,
                   uint64 address, FaultAccess access)
{
  if (!accessible_region(regions, size, address, access))
  {
    return 0;
  }
  int level;
  auto *pte = walkto(table, address, 0, 0, &level);
  auto permission = static_cast<unsigned long>(permission_for(access) | PTE_U);
  if (pte && (*pte & PTE_V) && (*pte & permission) == permission)
  {
    return PTE2PA(*pte) + (PGROUNDDOWN(address) & ((1UL << PXSHIFT(level)) - 1));
  }
  auto result = resolve_user_fault(table, size, regions, address, access);
  return result.result == FaultResult::Resolved ? result.physical : 0;
}

uint64 accessible_bytes(const VirtualAddressPool &regions, uint64 size, uint64 address)
{
  auto *region = regions.find(address);
  auto end = region->end;
  if (region->kind != RegionKind::Special && size < end)
  {
    end = size;
  }
  auto page_end = PGROUNDDOWN(address) + PGSIZE;
  if (page_end < end)
  {
    end = page_end;
  }
  return end - address;
}
} // namespace

int copy_to_user(pagetable_t table, uint64 size, const VirtualAddressPool &regions,
                 uint64 destination, const char *source, uint64 bytes)
{
  if (bytes > MAXVA || destination >= MAXVA || bytes > MAXVA - destination)
  {
    return -1;
  }
  while (bytes)
  {
    auto physical = user_access(table, size, regions, destination, FaultAccess::Write);
    if (!physical)
    {
      return -1;
    }
    auto count = accessible_bytes(regions, size, destination);
    if (bytes < count)
    {
      count = bytes;
    }
    xv::runtime::move(reinterpret_cast<void *>(physical + (destination % PGSIZE)),
                      source, count);
    bytes -= count;
    source += count;
    destination += count;
  }
  return 0;
}

int copy_from_user(pagetable_t table, uint64 size, const VirtualAddressPool &regions,
                   char *destination, uint64 source, uint64 bytes)
{
  if (bytes > MAXVA || source >= MAXVA || bytes > MAXVA - source)
  {
    return -1;
  }
  while (bytes)
  {
    auto physical = user_access(table, size, regions, source, FaultAccess::Read);
    if (!physical)
    {
      return -1;
    }
    auto count = accessible_bytes(regions, size, source);
    if (bytes < count)
    {
      count = bytes;
    }
    xv::runtime::move(destination,
                      reinterpret_cast<void *>(physical + (source % PGSIZE)), count);
    bytes -= count;
    destination += count;
    source += count;
  }
  return 0;
}

int copy_user_string(pagetable_t table, uint64 size, const VirtualAddressPool &regions,
                     char *destination, uint64 source, uint64 maximum)
{
  while (maximum)
  {
    auto physical = user_access(table, size, regions, source, FaultAccess::Read);
    if (!physical)
    {
      return -1;
    }
    auto count = accessible_bytes(regions, size, source);
    if (maximum < count)
    {
      count = maximum;
    }
    auto *input = reinterpret_cast<char *>(physical + source % PGSIZE);
    for (uint64 i = 0; i < count; ++i)
    {
      *destination++ = input[i];
      if (!input[i])
      {
        return 0;
      }
    }
    source += count;
    maximum -= count;
  }
  return -1;
}

int is_user_mapped(pagetable_t table, uint64 address)
{
  if (address >= MAXVA)
  {
    return 0;
  }
  auto *pte = walk_entry(table, address, 0);
  return pte && (*pte & PTE_V);
}
} // namespace xv::memory
