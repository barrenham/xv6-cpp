#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"

extern "C" int
app_main() {
  static char path[MAXPATH];
  if (!getcwd(path, sizeof(path)))
  {
    ui::error("pwd: cannot resolve current directory\n");
    exit(1);
  }
  printf("%s\n", path);
  exit(0);
}
