module;
#include "kernel/support/flow.h"
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"
module xv.process:address_space_flow;
import :address_space;
import xv.memory;

namespace xv::process
{
namespace flow = xv::flow;
extern "C" char trampoline[];

struct AddressSpace::CreateFlow
{
  struct Context
  {
    AddressSpace &owner;
    void *trap_frame;
    void *user_info;
    PageTable table = nullptr;
    bool result = false;
  };

  static bool allocate_table(Context &c) noexcept
  {
    c.table = xv::memory::create_user_table();
    return c.table != nullptr;
  }

  static bool map_trampoline(Context &c) noexcept
  {
    return xv::memory::map_user_pages(c.table, TRAMPOLINE, PGSIZE,
                                      reinterpret_cast<unsigned long>(trampoline),
                                      PTE_R | PTE_X) >= 0;
  }

  static bool map_frame(Context &c) noexcept
  {
    return xv::memory::map_user_pages(c.table, TRAPFRAME, PGSIZE,
                                      reinterpret_cast<unsigned long>(c.trap_frame),
                                      PTE_R | PTE_W) >= 0;
  }

  static bool map_info(Context &c) noexcept
  {
#ifdef LAB_PGTBL
    return xv::memory::map_user_pages(c.table, USYSCALL, PGSIZE,
                                      reinterpret_cast<unsigned long>(c.user_info),
                                      PTE_R | PTE_U) >= 0;
#else
    (void)c;
    return true;
#endif
  }

  static bool describe(Context &c) noexcept
  {
    c.owner.table_ = c.table;
    return c.owner.regions_.reserve(TRAMPOLINE, TRAMPOLINE + PGSIZE, PTE_R | PTE_X,
                                    xv::memory::RegionKind::Special) &&
           c.owner.regions_.reserve(TRAPFRAME, TRAPFRAME + PGSIZE, PTE_R | PTE_W,
                                    xv::memory::RegionKind::Special)
#ifdef LAB_PGTBL
           && c.owner.regions_.reserve(USYSCALL, USYSCALL + PGSIZE, PTE_R | PTE_U,
                                       xv::memory::RegionKind::Special)
#endif
        ;
  }

  static void commit(Context &c) noexcept
  {
    c.owner.size_ = 0;
    __atomic_store_n(&c.owner.observed_size_, c.owner.size_, __ATOMIC_RELEASE);
    c.result = true;
  }

  static void reset_owner(Context &c) noexcept
  {
    c.owner.reset();
  }

  static void unmap_frame(Context &c) noexcept
  {
    xv::memory::unmap_user_pages(c.table, TRAPFRAME, 1, 0);
  }

  static void unmap_trampoline(Context &c) noexcept
  {
    xv::memory::unmap_user_pages(c.table, TRAMPOLINE, 1, 0);
  }

  static void free_table(Context &c) noexcept
  {
    xv::memory::free_user_table(c.table, 0);
  }
  struct Allocate;
  struct Trampoline;
  struct Frame;
  struct Info;
  struct Describe;
  struct Commit;
  struct Reset;
  struct UnmapFrame;
  struct UnmapTrampoline;
  struct Free;
  using Graph = flow::graph<
      Allocate,
      flow::types<
          flow::node<Allocate, flow::noop,
                     flow::branch<flow::condition<&allocate_table>,
                                  flow::go<Trampoline>, flow::stop>>,
          flow::node<Trampoline, flow::noop,
                     flow::branch<flow::condition<&map_trampoline>, flow::go<Frame>,
                                  flow::go<Free>>>,
          flow::node<Frame, flow::noop,
                     flow::branch<flow::condition<&map_frame>, flow::go<Info>,
                                  flow::go<UnmapTrampoline>>>,
          flow::node<Info, flow::noop,
                     flow::branch<flow::condition<&map_info>, flow::go<Describe>,
                                  flow::go<UnmapFrame>>>,
          flow::node<Describe, flow::noop,
                     flow::branch<flow::condition<&describe>, flow::go<Commit>,
                                  flow::go<Reset>>>,
          flow::node<Commit, flow::action<&commit>, flow::stop>,
          flow::node<Reset, flow::action<&reset_owner>, flow::stop>,
          flow::node<UnmapFrame, flow::action<&unmap_frame>, flow::go<UnmapTrampoline>>,
          flow::node<UnmapTrampoline, flow::action<&unmap_trampoline>, flow::go<Free>>,
          flow::node<Free, flow::action<&free_table>, flow::stop>>>;
};

struct AddressSpace::ResizeFlow
{
  struct Context
  {
    AddressSpace &owner;
    int bytes;
    bool eager;
    unsigned long next, begin = 0, end = 0;
    int result = -1;
  };

  static bool positive(Context &c) noexcept
  {
    return c.bytes > 0;
  }

  static bool negative(Context &c) noexcept
  {
    return c.bytes < 0;
  }

  static bool plan_growth(Context &c) noexcept
  {
    if (static_cast<unsigned long>(c.bytes) > UTOP - c.next)
    {
      return false;
    }
    c.next += c.bytes;
    c.begin = PGROUNDUP(c.owner.size_);
    c.end = PGROUNDUP(c.next);
    return true;
  }

  static bool reserve(Context &c) noexcept
  {
    return c.begin == c.end ||
           c.owner.regions_.reserve(c.begin, c.end, PTE_R | PTE_W | PTE_U,
                                    xv::memory::RegionKind::Heap, true);
  }

  static bool allocate(Context &c) noexcept
  {
    return !c.eager || xv::memory::allocate_user_pages(c.owner.table_, c.owner.size_,
                                                       c.next, PTE_W);
  }

  static void undo_reservation(Context &c) noexcept
  {
    if (c.begin != c.end)
    {
      c.owner.regions_.erase(c.begin, c.end);
    }
  }

  static bool plan_shrink(Context &c) noexcept
  {
    auto shrink = static_cast<unsigned long>(-static_cast<long>(c.bytes));
    if (shrink > c.owner.size_ || c.owner.size_ - shrink < c.owner.heap_base_)
    {
      return false;
    }
    c.next -= shrink;
    c.begin = PGROUNDUP(c.next);
    c.end = PGROUNDUP(c.owner.size_);
    return true;
  }

  static bool erase(Context &c) noexcept
  {
    return c.begin == c.end || c.owner.regions_.erase(c.begin, c.end);
  }

  static void deallocate(Context &c) noexcept
  {
    xv::memory::deallocate_user_pages(c.owner.table_, c.owner.size_, c.next);
  }

  static void commit(Context &c) noexcept
  {
    c.owner.size_ = c.next;
    __atomic_store_n(&c.owner.observed_size_, c.owner.size_, __ATOMIC_RELEASE);
    c.result = 0;
  }
  struct Direction;
  struct ShrinkDirection;
  struct PlanGrowth;
  struct Reserve;
  struct Allocate;
  struct UndoReservation;
  struct PlanShrink;
  struct Erase;
  struct Deallocate;
  struct Commit;
  using Graph = flow::graph<
      Direction,
      flow::types<
          flow::node<Direction, flow::noop,
                     flow::branch<flow::condition<&positive>, flow::go<PlanGrowth>,
                                  flow::go<ShrinkDirection>>>,
          flow::node<ShrinkDirection, flow::noop,
                     flow::branch<flow::condition<&negative>, flow::go<PlanShrink>,
                                  flow::go<Commit>>>,
          flow::node<PlanGrowth, flow::noop,
                     flow::branch<flow::condition<&plan_growth>, flow::go<Reserve>,
                                  flow::stop>>,
          flow::node<
              Reserve, flow::noop,
              flow::branch<flow::condition<&reserve>, flow::go<Allocate>, flow::stop>>,
          flow::node<Allocate, flow::noop,
                     flow::branch<flow::condition<&allocate>, flow::go<Commit>,
                                  flow::go<UndoReservation>>>,
          flow::node<UndoReservation, flow::action<&undo_reservation>, flow::stop>,
          flow::node<
              PlanShrink, flow::noop,
              flow::branch<flow::condition<&plan_shrink>, flow::go<Erase>, flow::stop>>,
          flow::node<Erase, flow::noop,
                     flow::branch<flow::condition<&erase>, flow::go<Deallocate>,
                                  flow::stop>>,
          flow::node<Deallocate, flow::action<&deallocate>, flow::go<Commit>>,
          flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

struct AddressSpace::GrowFlow
{
  struct Context
  {
    AddressSpace &owner;
    unsigned long end;
    int permissions;
    bool stack;
    unsigned long begin = 0, limit = 0, next = 0;
    bool result = false;
  };

  static bool plan(Context &c) noexcept
  {
    if (c.end < c.owner.size_ || c.end > UTOP)
    {
      return false;
    }
    c.begin = PGROUNDUP(c.owner.size_);
    c.limit = PGROUNDUP(c.end);
    return true;
  }

  static bool reserve(Context &c) noexcept
  {
    return c.begin == c.limit ||
           c.owner.regions_.reserve(c.begin, c.limit, c.permissions | PTE_R | PTE_U,
                                    c.stack ? xv::memory::RegionKind::UserStack
                                            : xv::memory::RegionKind::Image);
  }

  static bool allocate(Context &c) noexcept
  {
    c.next = xv::memory::allocate_user_pages(c.owner.table_, c.owner.size_, c.end,
                                             c.permissions);
    return c.next != 0;
  }

  static void undo_reservation(Context &c) noexcept
  {
    if (c.begin != c.limit)
    {
      c.owner.regions_.erase(c.begin, c.limit);
    }
  }

  static void commit(Context &c) noexcept
  {
    c.owner.size_ = c.next;
    c.owner.heap_base_ = c.next;
    __atomic_store_n(&c.owner.observed_size_, c.owner.size_, __ATOMIC_RELEASE);
    c.result = true;
  }
  struct Plan;
  struct Reserve;
  struct Allocate;
  struct UndoReservation;
  struct Commit;
  using Graph = flow::graph<
      Plan,
      flow::types<
          flow::node<
              Plan, flow::noop,
              flow::branch<flow::condition<&plan>, flow::go<Reserve>, flow::stop>>,
          flow::node<
              Reserve, flow::noop,
              flow::branch<flow::condition<&reserve>, flow::go<Allocate>, flow::stop>>,
          flow::node<Allocate, flow::noop,
                     flow::branch<flow::condition<&allocate>, flow::go<Commit>,
                                  flow::go<UndoReservation>>>,
          flow::node<UndoReservation, flow::action<&undo_reservation>, flow::stop>,
          flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

struct AddressSpace::CopyFlow
{
  struct Context
  {
    const AddressSpace &owner;
    AddressSpace &child;
    bool result = false;
  };

  static bool copy_regions(Context &c) noexcept
  {
    return c.owner.regions_.copy_to(c.child.regions_);
  }

  static bool clone_pages(Context &c) noexcept
  {
    return xv::memory::clone_user_pages(c.owner.table_, c.child.table_,
                                        c.owner.size_) >= 0;
  }

  static void commit(Context &c) noexcept
  {
    c.child.size_ = c.owner.size_;
    c.child.heap_base_ = c.owner.heap_base_;
    __atomic_store_n(&c.child.observed_size_, c.child.size_, __ATOMIC_RELEASE);
    c.result = true;
  }
  struct Regions;
  struct Pages;
  struct Commit;
  using Graph =
      flow::graph<Regions,
                  flow::types<flow::node<Regions, flow::noop,
                                         flow::branch<flow::condition<&copy_regions>,
                                                      flow::go<Pages>, flow::stop>>,
                              flow::node<Pages, flow::noop,
                                         flow::branch<flow::condition<&clone_pages>,
                                                      flow::go<Commit>, flow::stop>>,
                              flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

struct AddressSpace::GuardFlow
{
  struct Context
  {
    AddressSpace &owner;
    unsigned long address;
    bool result = false;
  };

  static bool erase(Context &c) noexcept
  {
    return c.owner.regions_.erase(c.address, c.address + PGSIZE);
  }

  static bool reserve(Context &c) noexcept
  {
    return c.owner.regions_.reserve(c.address, c.address + PGSIZE, 0,
                                    xv::memory::RegionKind::Guard);
  }

  static void protect(Context &c) noexcept
  {
    xv::memory::protect_user_guard(c.owner.table_, c.address);
    c.result = true;
  }
  struct Erase;
  struct Reserve;
  struct Protect;
  using Graph =
      flow::graph<Erase,
                  flow::types<flow::node<Erase, flow::noop,
                                         flow::branch<flow::condition<&erase>,
                                                      flow::go<Reserve>, flow::stop>>,
                              flow::node<Reserve, flow::noop,
                                         flow::branch<flow::condition<&reserve>,
                                                      flow::go<Protect>, flow::stop>>,
                              flow::node<Protect, flow::action<&protect>, flow::stop>>>;
};

} // namespace xv::process
