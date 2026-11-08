#include "application.h"
#include "user/runtime/user.h"
#include "user/runtime/ui.h"

extern "C" int
app_main(int argc, char **argv) {
  if (argc != 1)
  {
    bool help = argc == 2 && strcmp(argv[1], "--help") == 0;
    printf("Usage: reboot\nSync mounted filesystems and reboot (root required).\n");
    return help ? 0 : 1;
  }
  if (reboot() < 0)
  {
    if (errno == EPERM)
    {
      ui::error("reboot: root required\n");
    }
    else if (errno == ENODEV)
    {
      ui::error("reboot: no supported power controller\n");
    }
    else
    {
      ui::error("reboot: filesystem sync failed; system remains running\n");
    }
    return 1;
  }
  return 0;
}
