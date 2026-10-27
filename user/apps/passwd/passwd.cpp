#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"
#include "user/runtime/password_hash.h"

extern "C" int main(int argc, char **argv)
{
  unsigned caller = getuid();
  for (int fd = 3; fd < 16; ++fd)
  {
    close(fd);
  }
  clearenv();
  umask(0077);
  account::User user{};
  bool known = argc == 2 ? account::find(argv[1], user) : account::find(caller, user);
  if (argc > 2 || geteuid() != 0 || !known || (caller != 0 && user.uid != caller))
  {
    fprintf(2, "passwd: permission denied; usage: passwd [user]\n");
    exit(1);
  }
  if (caller)
  {
    char current[64]{};
    bool valid =
        account::read_password("Current password: ", current, sizeof(current)) &&
        account::verify(user.name, current);
    password_hash::wipe(current, sizeof(current));
    if (!valid)
    {
      fprintf(2, "passwd: authentication failed\n");
      exit(1);
    }
  }
  if (!account::change_password(user.name))
  {
    exit(1);
  }
  printf("passwd: password updated for %s\n", user.name);
  exit(0);
}
