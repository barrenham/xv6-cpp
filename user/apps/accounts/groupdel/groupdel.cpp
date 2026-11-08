#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"

extern "C" int
app_main(int argc, char **argv) {
  if (argc != 2 || account::group_remove(argv[1]) < 0)
  {
    ui::error("groupdel: failed (root required, valid account/group)\n");
    exit(1);
  }
  ui::success("groupdel: done\n");
  exit(0);
}
