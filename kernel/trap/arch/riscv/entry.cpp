import xv.trap;
#include "kernel/trap/arch/riscv/entry.h"

extern "C" UserTrapReturn usertrap()
{
  auto token = xv::trap::controller().handle_user();
  return {token, xv::trap::user_frame_address()};
}

extern "C" void kerneltrap()
{
  xv::trap::controller().handle_kernel();
}
