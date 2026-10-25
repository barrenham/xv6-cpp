#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

// Required by abstract vtables; this is the only added C++ ABI hook.
extern "C" [[noreturn]] void __cxa_pure_virtual()
{
  panic("pure virtual call");
}
