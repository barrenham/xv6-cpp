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
  const char *type = nullptr, *source = nullptr, *options = nullptr, *path = nullptr;
  for (int i = 1; i < argc; ++i)
  {
    const char **value = !strcmp(argv[i], "-t") ? &type :
                         !strcmp(argv[i], "-s") ? &source :
                         !strcmp(argv[i], "-o") ? &options : nullptr;
    if (value)
    {
      if (*value || ++i == argc) goto usage;
      *value = argv[i];
    }
    else if (path || argv[i][0] != '/') goto usage;
    else path = argv[i];
  }
  if (!type || !path) goto usage;
  if ((source || options ? mountfs(type, path, source, options) : mount(type, path)) < 0)
  {
    fprintf(2,
            "mount: failed (root required, valid unused directory, available slot)\n");
    exit(1);
  }
  exit(0);
usage:
  fprintf(2, "usage: mount [-t type [-s source] [-o options] /directory]\n");
  exit(1);
}
