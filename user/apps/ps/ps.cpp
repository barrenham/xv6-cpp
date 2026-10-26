#include "kernel/support/types.h"
#include "kernel/fs/format/layout.h"
#include "user/runtime/proc.h"
extern "C" int main()
{
  int directory = open("/proc", O_RDONLY);
  if (directory < 0) { fprintf(2, "ps: cannot open /proc\n"); exit(1); }
  printf("PID PPID PGID SID STATE KiB NAME\n");
  dirent entry{};
  while (read(directory, &entry, sizeof(entry)) == sizeof(entry))
  {
    if (!entry.inum || entry.name[0] < '1' || entry.name[0] > '9') continue;
    int pid = atoi(entry.name); char path[48], data[512]; proc::path(pid, path);
    if (proc::read(path, data, sizeof(data)) < 0) continue;
    char *name = proc::field(data, "Name"), *state = proc::field(data, "State");
    if (!name || !state) continue;
    int parent = proc::number(data, "PPid"), group = proc::number(data, "Pgid");
    int session = proc::number(data, "Sid"), bytes = proc::number(data, "VmSize");
    char *end = strchr(name, '\n'); if (end) *end = 0;
    printf("%d %d %d %d %c %d %s\n", pid, parent, group, session, *state, bytes, name);
  }
  close(directory); exit(0);
}
