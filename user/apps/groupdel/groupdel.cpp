#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"

extern "C" int main(int argc, char **argv)
{
  if (argc != 2 || account::group_remove(argv[1]) < 0)
  {
    fprintf(2, "groupdel: failed (root required, valid account/group)\n");
    exit(1);
  }
  printf("groupdel: done\n");
  exit(0);
}
