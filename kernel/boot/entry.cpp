import xv.boot;

#include "kernel/support/param.h"

// Symbols referenced by entry.S retain C linkage.
extern "C"
{
alignas(16) char stack0[4096 * NCPU];

[[noreturn]] void start(unsigned long hart_id, unsigned long device_tree)
{
  xv::boot::enter(hart_id, device_tree);
}
}
