#include "application.h"
#include "user/runtime/user.h"
#include "user/runtime/ui.h"

extern "C" int
app_main(int argc, char **argv) {
  if (argc != 1)
  {
    bool help = argc == 2 && strcmp(argv[1], "--help") == 0;
    printf("Usage: poweroff\nSync mounted filesystems and power off (root required).\n");
    return help ? 0 : 1;
  }
  if (poweroff() < 0)
  {
    if (errno == EPERM)
    {
      ui::error("poweroff: root required\n");
    }
    else if (errno == ENODEV)
    {
      ui::error("poweroff: no supported power controller\n");
    }
    else
    {
      ui::error("poweroff: filesystem sync failed; system remains running\n");
    }
    return 1;
  }
  return 0;
}
