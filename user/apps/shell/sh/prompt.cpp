#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"
#include "user/runtime/terminal_style.h"
#include "editor.h"

namespace shell
{

const char *
prompt_text() {
  static char output[MAXPATH + 32], path[MAXPATH];
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
  return output;
}

void
prompt() {
  const char *text = prompt_display(prompt_text());
  unsigned length = strlen(text);
  write(2, text, length);
}

const char *
prompt_display(const char *text) {
  if (!terminal_style::enabled(2))
  {
    return text;
  }
  static char output[MAXPATH + 64];
  // Keep the plain prompt available separately: SGR bytes occupy no columns.
  // Each prompt resets its attributes before the user starts typing.
  const char *color = geteuid() == 0 ? "\033[33m" : "\033[1;32m";
  strcpy(output, color);
  strcpy(output + strlen(output), text);
  strcpy(output + strlen(output), terminal_style::reset);
  return output;
}
} // namespace shell
