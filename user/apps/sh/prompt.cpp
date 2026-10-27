#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"

namespace shell
{

void prompt()
{
  char output[MAXPATH + 32], path[MAXPATH];
  account::User user{};
  unsigned uid = geteuid();
  if (account::find(uid, user))
  {
    strcpy(output, user.name);
  }
  else
  {
    char digits[12];
    unsigned count = 0, used = 0;
    do
    {
      digits[count++] = '0' + uid % 10;
      uid /= 10;
    } while (uid);
    while (count)
    {
      output[used++] = digits[--count];
    }
    output[used] = 0;
  }
  unsigned used = strlen(output);
  output[used++] = ':';
  if (!getcwd(path, sizeof(path)))
  {
    strcpy(path, "?");
  }
  strcpy(output + used, path);
  used += strlen(path);
  strcpy(output + used, "$ ");
  used += 2;
  write(2, output, used);
}
} // namespace shell
