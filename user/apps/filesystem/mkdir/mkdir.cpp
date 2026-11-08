#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"

extern "C"
{

  int
  app_main(int argc, char *argv[]) {
    int i;

    if (argc < 2)
    {
      ui::usage("Usage: mkdir files...\n");
      exit(1);
    }

    for (i = 1; i < argc; i++)
    {
      if (mkdir(argv[i]) < 0)
      {
        ui::error("mkdir: %s failed to create\n", argv[i]);
        break;
      }
    }

    exit(0);
  }
}
