// Create a zombie process that
// must be reparented at exit.

#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"

extern "C" {

int
main(void)
{
  if (fork() > 0)
    pause(5); // Let child exit before parent.
  exit(0);
}
}
