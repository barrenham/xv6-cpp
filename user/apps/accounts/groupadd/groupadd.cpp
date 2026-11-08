#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"

extern "C" int
app_main(int argc, char **argv) {
  if (argc != 2 || account::group_add(argv[1]) < 0)
  {
    ui::error("groupadd: failed (root required, valid account/group)\n");
    exit(1);
  }
  ui::success("groupadd: done\n");
  exit(0);
}
