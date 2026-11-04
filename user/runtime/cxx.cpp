#include "user/runtime/user.h"

extern "C" [[noreturn]] void __cxa_pure_virtual()
{
  fprintf(2, "runtime: abstract virtual function called\n");
  exit(1);
}
