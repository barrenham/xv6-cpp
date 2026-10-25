#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"

extern "C" {

int
main(int argc, char *argv[])
{
  int i;

  for (i = 1; i < argc; i++) {
    write(1, argv[i], strlen(argv[i]));
    if (i + 1 < argc) {
      write(1, " ", 1);
    } else {
      write(1, "\n", 1);
    }
  }
  exit(0);
}
}
