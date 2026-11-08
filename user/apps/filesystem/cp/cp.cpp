#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"
#include "user/runtime/file_copy.h"

extern "C" int
app_main(int argc, char **argv) {
  static char target[MAXPATH];
  if (argc != 3)
  {
    ui::usage("usage: cp source destination\n");
    exit(1);
  }
  if (!fileutil::destination(argv[1], argv[2], target, sizeof(target)) ||
      fileutil::copy(argv[1], target) < 0)
  {
    ui::error("cp: cannot copy %s to %s\n", argv[1], argv[2]);
    exit(1);
  }
  exit(0);
}
