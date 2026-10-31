#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"

extern "C" int main()
{
  static char path[MAXPATH];
  if (!getcwd(path, sizeof(path)))
  {
    fprintf(2, "pwd: cannot resolve current directory\n");
    exit(1);
  }
  printf("%s\n", path);
  exit(0);
}
