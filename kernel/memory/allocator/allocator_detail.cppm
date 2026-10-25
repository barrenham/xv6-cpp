module;

#include "kernel/memory/layout/memlayout.h"

module xv.memory:detail;

import :allocator;

namespace xv::memory::detail
{
inline constexpr PhysicalAddress KernelLoadAddress(KERNBASE);
// Identity-mapped RAM and two virtual pages per potential stack frame must
// fit together below the trampoline in the lower Sv39 address window.
inline constexpr PhysicalAddress DirectMapLimit(((1UL << 38) - 4 * PageSize) / 3 /
                                                PageSize * PageSize);

inline constexpr MemorySize block_size(unsigned order)
{
  return MemorySize(PageSize) << order;
}

void fill(void *address, int value, MemorySize length);
[[noreturn]] void fail(const char *message);
} // namespace xv::memory::detail
