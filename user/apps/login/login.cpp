#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"
#include "user/runtime/password_hash.h"

extern "C" int main()
{
  if (geteuid() != 0)
  {
    fprintf(2, "login: must be started by init\n");
    exit(1);
  }
  sigdisposition(SIGINT, SIGNAL_IGNORE);
  sigdisposition(SIGTSTP, SIGNAL_IGNORE);
  for (;;)
  {
    char name[64], password[64]{};
    write(1, "login: ", 7);
    if (!gets(name, sizeof(name)) || !name[0])
    {
      continue;
    }
    unsigned size = strlen(name);
    if (size && name[size - 1] == '\n')
    {
      name[--size] = 0;
    }
    account::User user{};
    bool known = account::find(name, user);
    bool good = account::read_password("Password: ", password, sizeof(password));
    good = good && known && account::verify(name, password);
    password_hash::wipe(password, sizeof(password));
    if (good)
    {
      sigdisposition(SIGINT, SIGNAL_DEFAULT);
      sigdisposition(SIGTSTP, SIGNAL_DEFAULT);
      account::enter(user);
    }
    fprintf(2, "Login incorrect\n");
    pause(10);
  }
}
