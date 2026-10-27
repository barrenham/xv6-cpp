#include "kernel/support/types.h"
#include "user/runtime/user.h"

extern "C" int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(2, "usage: umount /directory\n");
    exit(1);
  }
  if (umount(argv[1]) < 0)
  {
    fprintf(2, "umount: failed (busy, protected mount or insufficient permission)\n");
    exit(1);
  }
  exit(0);
}
