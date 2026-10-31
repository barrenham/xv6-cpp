#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"

extern "C" int main(int argc, char **argv)
{
  static char path[MAXPATH];
  int n = argc == 2 ? readlink(argv[1], path, sizeof(path) - 1) : -1;
  if (n < 0)
  {
    fprintf(2, "readlink: failed\n");
    exit(1);
  }
  path[n] = 0;
  printf("%s\n", path);
  exit(0);
}
