#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"
#include "user/runtime/terminal_style.h"
#include "editor.h"

namespace shell
{

namespace
{
char output[MAXPATH + 32], path[MAXPATH], styled[MAXPATH + 64];
}

void
refresh_prompt() {
  account::User user{};
  unsigned uid = geteuid(), effective_uid = uid;
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
  if (!terminal_style::enabled(2))
  {
    strcpy(styled, output);
    return;
  }
  const char *color = effective_uid == 0 ? "\033[33m" : "\033[1;32m";
  strcpy(styled, color);
  strcpy(styled + strlen(styled), output);
  strcpy(styled + strlen(styled), terminal_style::reset);
}

const char *
prompt_text() {
  return output;
}

const char *
prompt_display() {
  return styled;
}

void
prompt() {
  const char *text = prompt_display();
  write(2, text, strlen(text));
}
} // namespace shell
