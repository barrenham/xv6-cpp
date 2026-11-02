#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"

namespace
{
void check(bool okay, const char *message)
{
  if (!okay) { fprintf(2, "ramfsservicetest: FAIL: %s\n", message); exit(1); }
}
}

extern "C" int main()
{
  constexpr char path[] = "/tmp/ramfs-service";
  constexpr char marker[] = "/tmp/ramfs-service/covered";
  check(mkdir(path) == 0, "create covered directory");
  int file = open(marker, O_CREATE | O_WRONLY);
  check(file >= 0 && write(file, "base", 4) == 4, "covered marker");
  close(file);
  struct stat original{};
  check(stat(path, &original) == 0, "covered metadata");
  for (unsigned round = 0; round < 3; ++round)
  {
    int daemon = fork();
    check(daemon >= 0, "fork independent ramfs daemon");
    if (!daemon)
    {
      char *args[] = {const_cast<char *>("/bin/ramfsd"), const_cast<char *>("--mount"),
                      const_cast<char *>("ramfs-layer"), const_cast<char *>(path), nullptr};
      exec(args[0], args);
      check(false, "exec ramfs daemon");
    }
    bool mounted = false;
    for (unsigned attempt = 0; attempt < 100; ++attempt)
    {
      struct stat layer{};
      if (stat(path, &layer) == 0 && layer.dev != original.dev) { mounted = true; break; }
      pause(1);
    }
    check(mounted && open(marker, O_RDONLY) < 0, "registered daemon provides isolated overlay");
    file = open("/tmp/ramfs-service/live", O_CREATE | O_RDWR);
    check(file >= 0 && write(file, "layer", 5) == 5, "write new layer");
    check(umount(path) < 0, "open descriptor prevents unmount");
    close(file);
    check(umount(path) == 0, "unmount idle service");
    int status;
    check(wait(&status) == daemon && status == 0, "unmount sends EOF and daemon exits");
    char data[5]{};
    file = open(marker, O_RDONLY);
    check(file >= 0 && read(file, data, 4) == 4 && !strcmp(data, "base"), "covered data restored");
    close(file);
  }
  check(unlink(marker) == 0 && unlink(path) == 0, "remove fixtures");
  printf("ramfsservicetest: independent daemon, overlay and lifecycle passed\n");
  exit(0);
}
