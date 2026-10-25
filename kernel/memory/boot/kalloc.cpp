module;

// Boot integration and the legacy C ABI. All state belongs to the selected
// PhysicalPageAllocator implementation; these functions only forward calls.
#include "kernel/memory/layout/memlayout.h"
#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

module xv.memory;

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

void boot_initialize(AllocatorKind kind, PhysicalAddress device_tree)
{
  // Firmware owns the DTB pointer; its initial header/span must be accessible.
  BootMemoryInfo memory;
  FdtMemorySource source(device_tree.pointer(), FdtMemorySource::MaxBlobSize);
  if (!source.discover(memory))
  {
    detail::fail("memory: invalid or unsupported device tree");
  }
  if (!memory.ram().contains(device_tree))
  {
    detail::fail("memory: DTB outside RAM");
  }
  if (!initialize(kind, memory, PhysicalAddress::from_pointer(end)))
  {
    detail::fail("memory: invalid RAM layout or insufficient metadata space");
  }
  const auto &layout = allocator().layout();
  printk("RAM: base=%p bytes=%lu metadata=%lu usable_pages=%lu source=%s\n",
         layout.ram().begin().pointer(), layout.ram().size(), layout.metadata().size(),
         layout.usable_pages(), source.name());
}

namespace detail
{
void fill(void *address, int value, MemorySize length)
{
  memset(address, value, static_cast<uint>(length));
}

[[noreturn]] void fail(const char *message)
{
  panic(const_cast<char *>(message));
}
} // namespace detail
} // namespace xv::memory

extern "C"
{
  void *kalloc()
  {
    return xv::memory::allocator().allocate();
  }

  void kfree(void *pa)
  {
    xv::memory::allocator().deallocate(pa);
  }

  void *superalloc()
  {
    return xv::memory::allocator().allocate(xv::memory::SuperpageOrder);
  }

  void superfree(void *pa)
  {
    xv::memory::allocator().deallocate(pa, xv::memory::SuperpageOrder);
  }

  void supersplit(void *pa)
  {
    if (!xv::memory::allocator().split_to_pages(pa, xv::memory::SuperpageOrder))
    {
      xv::memory::detail::fail("physical allocator: unsupported superpage split");
    }
  }
}
