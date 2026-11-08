#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"

extern "C" int
app_main(int argc, char **argv) {
  if (argc != 2)
  {
    ui::usage("usage: useradd NAME\n");
    exit(1);
  }
  int uid = account::add(argv[1]);
  if (uid < 0)
  {
    ui::error(
        "useradd: root required, duplicate user, busy database or creation failed\n");
    exit(1);
  }
  ui::success("useradd: %s uid=%d gid=%d\n", argv[1], uid, uid);
  exit(0);
}
