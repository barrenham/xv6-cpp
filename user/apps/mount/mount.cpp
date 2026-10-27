#include "kernel/support/types.h"
#include "kernel/fs/mount.h"
#include "user/runtime/user.h"

extern "C" int main(int argc, char **argv)
{
  if (argc == 1)
  {
    mount_info info;
    int result;
    for (int i = 0; (result = mountinfo(i, &info)) > 0; ++i)
    {
      printf("%s on %s\n", info.type, info.path);
    }
    exit(result < 0);
  }
  if (argc != 4 || strcmp(argv[1], "-t"))
  {
    fprintf(2, "usage: mount [-t ramfs /directory]\n");
    exit(1);
  }
  if (mount(argv[2], argv[3]) < 0)
  {
    fprintf(2,
            "mount: failed (root required, valid unused directory, available slot)\n");
    exit(1);
  }
  exit(0);
}
