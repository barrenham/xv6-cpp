module;

#include "kernel/memory/layout/memlayout.h"
#include "kernel/support/param.h"

module xv.memory:detail;

import :allocator;

namespace xv::memory::detail
{
inline constexpr PhysicalAddress KernelLoadAddress(KERNBASE);
// Leave all kernel stacks, their bottom guard page, and the trampoline outside
// the RAM identity window in the lower Sv39 half.
inline constexpr PhysicalAddress DirectMapLimit((1UL << 38) -
                                                (2 * NPROC + 3) * PageSize);

inline constexpr MemorySize block_size(unsigned order)
{
  return MemorySize(PageSize) << order;
}

void fill(void *address, int value, MemorySize length);
[[noreturn]] void fail(const char *message);
} // namespace xv::memory::detail
