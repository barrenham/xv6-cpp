#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"

extern "C" {

int
main(int argc, char *argv[])
{
  sync();
  exit(0);
}
}
