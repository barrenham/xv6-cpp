#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "user/runtime/user.h"

extern "C" int
app_main(int argc, char **argv) {
  if (argc != 2)
  {
    ui::usage("usage: umount /directory\n");
    exit(1);
  }
  if (umount(argv[1]) < 0)
  {
    ui::error("umount: failed (busy, protected mount or insufficient permission)\n");
    exit(1);
  }
  exit(0);
}
