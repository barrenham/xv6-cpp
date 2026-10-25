import xv.trap;
#include "kernel/trap/arch/riscv/entry.h"

extern "C" unsigned long usertrap()
{
  return xv::trap::controller().handle_user();
}

extern "C" void kerneltrap()
{
  xv::trap::controller().handle_kernel();
}
