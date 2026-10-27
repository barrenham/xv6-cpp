#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"

extern "C" int main(int argc, char **argv)
{
  if (argc != 2 || account::remove(argv[1]) < 0)
  {
    fprintf(2, "userdel: failed (root required, valid account/group)\n");
    exit(1);
  }
  printf("userdel: done\n");
  exit(0);
}
