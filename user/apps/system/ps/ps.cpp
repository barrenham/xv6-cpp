#include "application.h"
#include "kernel/support/types.h"
#include "user/fs/backends/xv6/format.h"
#include "user/runtime/proc.h"
#include "user/runtime/directory.h"

extern "C" int
app_main() {
  int directory = open("/proc", O_RDONLY);
  if (directory < 0)
  {
    fprintf(2, "ps: cannot open /proc\n");
    exit(1);
  }
  printf("%6s %6s %6s %6s %-5s %8s %s\n", "PID", "PPID", "PGID", "SID", "STATE", "KiB",
         "NAME");
  DirectoryReader reader(directory);
  const linux_dirent64 *entry;
  while (reader.next(entry) == 1)
  {
    if (!entry->d_ino || entry->d_name[0] < '1' || entry->d_name[0] > '9')
    {
      continue;
    }
    int pid = atoi(entry->d_name);
    char path[48], data[512];
    proc::path(pid, path);
    if (proc::read(path, data, sizeof(data)) < 0)
    {
      continue;
    }
    char *name = proc::field(data, "Name"), *state = proc::field(data, "State");
    if (!name || !state)
    {
      continue;
    }
    int parent = proc::number(data, "PPid"), group = proc::number(data, "Pgid");
    int session = proc::number(data, "Sid"), bytes = proc::number(data, "VmSize");
    char *end = strchr(name, '\n');
    if (end)
    {
      *end = 0;
    }
    printf("%6d %6d %6d %6d %-5c %8d %s\n", pid, parent, group, session, *state, bytes,
           name);
  }
  close(directory);
  exit(0);
}
