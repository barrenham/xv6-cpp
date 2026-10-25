import xv.print;

#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"

// Required by abstract vtables; this is the only added C++ ABI hook.
extern "C" [[noreturn]] void __cxa_pure_virtual()
{
  xv::print::panic("pure virtual call");
}
