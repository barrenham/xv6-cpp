module;

#include "kernel/support/types.h"
#include "kernel/support/flow.h"
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
  unmap_device_pages();
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
  auto device = device_pages_;
  device_pages_ = other.device_pages_;
  other.device_pages_ = device;
  auto identity = device_identity_;
  device_identity_ = other.device_identity_;
  other.device_identity_ = identity;
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
  if (context.result && device_pages_ &&
      !child.map_device(nullptr, device_pages_, this))
  {
    child.reset();
    return false;
  }
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

namespace
{
constexpr unsigned long DeviceBase = UTOP - 32UL * 1024 * 1024;
constexpr unsigned DeviceMaxPages = 4096;
} // namespace

unsigned long AddressSpace::map_device_pages(void *const *pages, unsigned count)
{
  if (device_pages_)
  {
    return device_mapping(pages, count) ? DeviceBase : 0;
  }
  return map_device(pages, count, nullptr);
}

bool AddressSpace::device_mapping(void *const *pages, unsigned count) const
{
  if (!pages || !count || count != device_pages_)
  {
    return false;
  }
  return device_identity_ == reinterpret_cast<unsigned long>(pages[0]);
}

bool AddressSpace::unmap_device_pages()
{
  if (!device_pages_)
  {
    return false;
  }
  xv::memory::unmap_user_pages(table_, DeviceBase, device_pages_, 1);
  device_pages_ = 0;
  device_identity_ = 0;
  regions_.release(DeviceBase);
  // The current process owns all page-table edits; fork builds an inactive table.
  // Returning through the trampoline also fences the user page table.
  return true;
}

struct AddressSpace::DeviceMapFlow
{
  struct Context
  {
    AddressSpace &owner;
    void *const *pages;
    unsigned count, index = 0;
    const AddressSpace *inherited;
    unsigned long physical = 0, result = 0;
  };

  static bool reserve(Context &c) noexcept
  {
    if (!c.owner.table_ || !c.count || c.count > DeviceMaxPages ||
        (!c.pages && !c.inherited) || c.owner.device_pages_)
    {
      return false;
    }
    return c.inherited || c.owner.regions_.reserve(
                              DeviceBase, DeviceBase + c.count * PGSIZE,
                              PTE_R | PTE_W | PTE_U, xv::memory::RegionKind::Special);
  }

  static bool map(Context &c) noexcept
  {
    c.physical = c.inherited ? c.inherited->translate(DeviceBase + c.index * PGSIZE)
                             : reinterpret_cast<unsigned long>(c.pages[c.index]);
    return c.physical && !(c.physical % PGSIZE) &&
           xv::memory::map_user_pages(c.owner.table_, DeviceBase + c.index * PGSIZE,
                                      PGSIZE, c.physical, PTE_R | PTE_W | PTE_U) >= 0;
  }

  static void retain(Context &c) noexcept
  {
    xv::memory::allocator().retain_page(reinterpret_cast<void *>(c.physical));
    if (!c.index)
    {
      c.owner.device_identity_ = c.physical;
    }
    c.owner.device_pages_ = ++c.index;
  }

  static bool finished(Context &c) noexcept
  {
    return c.index == c.count;
  }

  static void commit(Context &c) noexcept
  {
    c.result = DeviceBase;
  }

  static void rollback(Context &c) noexcept
  {
    if (!c.owner.unmap_device_pages())
    {
      c.owner.regions_.release(DeviceBase);
    }
  }

  struct Reserve;
  struct Map;
  struct Retain;
  struct Commit;
  struct Rollback;
  using Graph = xv::flow::graph<
      Reserve,
      xv::flow::types<
          xv::flow::node<Reserve, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&reserve>,
                                          xv::flow::go<Map>, xv::flow::stop>>,
          xv::flow::node<
              Map, xv::flow::noop,
              xv::flow::branch<xv::flow::condition<&map>, xv::flow::go<Retain>,
                               xv::flow::go<Rollback>>>,
          xv::flow::node<Retain, xv::flow::action<&retain>,
                         xv::flow::branch<xv::flow::condition<&finished>,
                                          xv::flow::go<Commit>, xv::flow::go<Map>>>,
          xv::flow::node<Commit, xv::flow::action<&commit>, xv::flow::stop>,
          xv::flow::node<Rollback, xv::flow::action<&rollback>, xv::flow::stop>>>;
};

unsigned long AddressSpace::map_device(void *const *pages, unsigned count,
                                       const AddressSpace *inherited)
{
  DeviceMapFlow::Context context{*this, pages, count, 0, inherited};
  (void)DeviceMapFlow::Graph::run<0>(context);
  return context.result;
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
