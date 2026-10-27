#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"

extern "C" int main(int argc, char **argv)
{
  if (argc != 2 || account::group_add(argv[1]) < 0)
  {
    fprintf(2, "groupadd: failed (root required, valid account/group)\n");
    exit(1);
  }
  printf("groupadd: done\n");
  exit(0);
}
