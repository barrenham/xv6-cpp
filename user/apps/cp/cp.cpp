#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"
#include "user/runtime/file_copy.h"

extern "C" int main(int argc, char **argv)
{
  char target[MAXPATH];
  if (argc != 3)
  {
    fprintf(2, "usage: cp source destination\n");
    exit(1);
  }
  if (!fileutil::destination(argv[1], argv[2], target, sizeof(target)) ||
      fileutil::copy(argv[1], target) < 0)
  {
    fprintf(2, "cp: cannot copy %s to %s\n", argv[1], argv[2]);
    exit(1);
  }
  exit(0);
}
