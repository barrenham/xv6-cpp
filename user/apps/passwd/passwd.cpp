#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"

extern "C" int main(int argc, char **argv)
{
  if (geteuid() != 0 || argc > 2)
  {
    fprintf(2, "passwd: root required; usage: passwd [user]\n");
    exit(1);
  }
  account::User user{};
  if (!account::find(argc == 2 ? argv[1] : "root", user))
  {
    fprintf(2, "passwd: unknown user\n");
    exit(1);
  }
  if (!account::change_password(user.name))
  {
    exit(1);
  }
  printf("passwd: password updated for %s\n", user.name);
  exit(0);
}
