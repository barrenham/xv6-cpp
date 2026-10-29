module;

// Boot integration and allocator platform operations. All state belongs to the selected
// PhysicalPageAllocator implementation; these functions only forward calls.
#include "kernel/memory/layout/memlayout.h"
#include "kernel/support/types.h"
#include "kernel/support/flow.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.memory;

import xv.print;

import xv.runtime;

import :detail;

extern "C" char end[];
static_assert(PGSIZE == xv::memory::PageSize);

namespace xv::memory
{
AllocatorKind configured_allocator_kind()
{
#if XV_DEFAULT_ALLOCATOR_FREELIST
  return AllocatorKind::FreeList;
#else
  return AllocatorKind::Buddy;
#endif
}

namespace
{
namespace flow = xv::flow;

struct BootInitializeFlow
{
  struct Context
  {
    AllocatorKind kind;
    PhysicalAddress tree;
    BootMemoryInfo &memory;
    FdtMemorySource &source;
    bool ok = false;
  };

  static void discover(Context &c) noexcept
  {
    c.ok = c.source.discover(c.memory);
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void invalid_tree(Context &) noexcept
  {
    detail::fail("memory: invalid or unsupported device tree");
  }

  static bool resident(Context &c) noexcept
  {
    return c.memory.ram().contains(c.tree);
  }

  static void outside(Context &) noexcept
  {
    detail::fail("memory: DTB outside RAM");
  }

  static void initialize(Context &c) noexcept
  {
    c.ok =
        xv::memory::initialize(c.kind, c.memory, PhysicalAddress::from_pointer(::end));
  }

  static void invalid_layout(Context &) noexcept
  {
    detail::fail("memory: invalid RAM layout or insufficient metadata space");
  }

  static void report(Context &c) noexcept
  {
    const auto &layout = allocator().layout();
    xv::print::printk(
        "RAM: base=%p bytes=%lu metadata=%lu usable_pages=%lu source=%s\n",
        layout.ram().begin().pointer(), layout.ram().size(), layout.metadata().size(),
        layout.usable_pages(), c.source.name());
  }
  struct Discover;
  struct InvalidTree;
  struct Resident;
  struct Outside;
  struct Initialize;
  struct InvalidLayout;
  struct Report;
  using Graph = flow::graph<
      Discover,
      flow::types<flow::node<Discover, flow::action<&discover>,
                             flow::branch<flow::condition<&ok>, flow::go<Resident>,
                                          flow::go<InvalidTree>>>,
                  flow::node<InvalidTree, flow::action<&invalid_tree>, flow::stop>,
                  flow::node<Resident, flow::noop,
                             flow::branch<flow::condition<&resident>,
                                          flow::go<Initialize>, flow::go<Outside>>>,
                  flow::node<Outside, flow::action<&outside>, flow::stop>,
                  flow::node<Initialize, flow::action<&initialize>,
                             flow::branch<flow::condition<&ok>, flow::go<Report>,
                                          flow::go<InvalidLayout>>>,
                  flow::node<InvalidLayout, flow::action<&invalid_layout>, flow::stop>,
                  flow::node<Report, flow::action<&report>, flow::stop>>>;
};
} // namespace

void boot_initialize(AllocatorKind kind, PhysicalAddress device_tree)
{
  BootMemoryInfo memory;
  FdtMemorySource source(device_tree.pointer(), FdtMemorySource::MaxBlobSize);
  BootInitializeFlow::Context context{kind, device_tree, memory, source};
  (void)BootInitializeFlow::Graph::run<0>(context);
}

namespace detail
{
void fill(void *address, int value, MemorySize length)
{
  xv::runtime::fill(address, value, length);
}

[[noreturn]] void fail(const char *message)
{
  xv::print::panic(message);
}
} // namespace detail
} // namespace xv::memory
