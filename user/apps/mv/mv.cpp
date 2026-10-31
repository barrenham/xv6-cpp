#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"
#include "user/runtime/file_copy.h"

extern "C" int main(int argc, char **argv)
{
  static char target[MAXPATH];
  if (argc != 3 || !fileutil::destination(argv[1], argv[2], target, sizeof(target)))
  {
    fprintf(2, "usage: mv source destination\n");
    exit(1);
  }
  if (rename(argv[1], target) == 0)
  {
    exit(0);
  }
  struct stat src, parent;
  static char directory[MAXPATH];
  strcpy(directory, target);
  char *last = nullptr;
  for (char *p = directory; *p; ++p)
  {
    if (*p == '/')
    {
      last = p;
    }
  }
  if (last)
  {
    if (last == directory)
    {
      last[1] = 0;
    }
    else
    {
      *last = 0;
    }
  }
  else
  {
    strcpy(directory, ".");
  }
  if (stat(argv[1], &src) == 0 && stat(directory, &parent) == 0 &&
      src.dev != parent.dev && src.type == T_FILE &&
      fileutil::copy(argv[1], target) == 0 && unlink(argv[1]) == 0)
  {
    exit(0);
  }
  fprintf(2, "mv: cannot move %s to %s\n", argv[1], argv[2]);
  exit(1);
}
