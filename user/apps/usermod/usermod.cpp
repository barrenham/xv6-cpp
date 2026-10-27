#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"

extern "C" int main(int argc, char **argv)
{
  if (argc != 4 || (strcmp(argv[1], "-G") && strcmp(argv[1], "-aG")) ||
      account::memberships(argv[3], argv[2], !strcmp(argv[1], "-aG")) < 0)
  {
    fprintf(2, "usermod: usage: usermod -G|-aG groups user (root required)\n");
    exit(1);
  }
  exit(0);
}
