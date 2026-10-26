#include "kernel/support/types.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/proc.h"
namespace
{
void check(bool ok, const char *message)
{ if (!ok) { fprintf(2, "procfstest: FAIL: %s\n", message); exit(1); } }
int read_all(int fd, char *data, int capacity, int step)
{
  int used = 0;
  while (used + 1 < capacity)
  { int n = read(fd, data + used, step); if (n < 0) return -1; if (!n) break; used += n; }
  data[used] = 0; return used;
}
}
extern "C" int main()
{
  char first[512], second[512], path[48];
  check(proc::read("/proc/meminfo", first, sizeof(first)) > 0, "meminfo");
  int total = proc::number(first, "MemTotal"), free = proc::number(first, "MemFree");
  check(total > 0 && free >= 0 && free <= total && proc::number(first, "MemUsed") + free == total, "memory counters");
  int fd = open("/proc/uptime", O_RDONLY); check(fd >= 0, "uptime open");
  check(read(fd, first, 1) == 1, "short read"); pause(2);
  check(read_all(fd, first + 1, sizeof(first) - 1, 1) >= 0 && strchr(first, '.'), "snapshot remainder"); close(fd);
  proc::path(getpid(), path);
  check(proc::read(path, first, sizeof(first)) > 0 && proc::number(first, "Pid") == getpid() &&
        proc::number(first, "PPid") > 0 && proc::number(first, "VmSize") > 0, "own process status");
  check(open(path, O_WRONLY) < 0 && open(path, O_RDONLY | O_TRUNC) < 0 &&
        open("/proc/new", O_CREATE | O_RDWR) < 0 && mkdir("/proc/new") < 0 &&
        unlink("/proc/meminfo") < 0 && unlink("/proc") < 0, "read-only filesystem and mount protection");
  int sync[2]; check(pipe(sync) == 0, "synchronization pipe");
  int child = fork(); check(child >= 0, "fork");
  if (!child) { close(sync[1]); char byte; read(sync[0], &byte, 1); exit(0); }
  close(sync[0]); proc::path(child, path);
  fd = open(path, O_RDONLY); check(fd >= 0, "child snapshot");
  check(write(sync[1], "x", 1) == 1, "release child"); close(sync[1]);
  int status; check(wait(&status) == child && status == 0, "reap child");
  check(open(path, O_RDONLY) < 0, "reaped process disappears");
  check(read_all(fd, first, sizeof(first), 1) > 0 && proc::number(first, "Pid") == child,
        "open snapshot survives process exit"); close(fd);
  fd = open("/proc/meminfo", O_RDONLY); check(fd >= 0, "snapshot for dup");
  check(read(fd, reinterpret_cast<void *>(~0UL), 1) == -1, "bad snapshot destination keeps offset");
  int alias = dup(fd); check(alias >= 0 && read(fd, first, 1) == 1, "shared offset first byte");
  close(fd); check(read_all(alias, first + 1, sizeof(first) - 1, 1) > 0 && !memcmp(first, "MemTotal:", 9), "shared offset remainder"); close(alias);
  check(chdir("/proc") == 0 && proc::read("meminfo", second, sizeof(second)) > 0 && chdir("..") == 0,
        "relative paths and mounted parent");
  fd = open("/proc", O_RDONLY); check(fd >= 0, "process directory"); dirent entry; bool found = false;
  while (read(fd, &entry, sizeof(entry)) == sizeof(entry)) if (atoi(entry.name) == getpid()) found = true;
  close(fd); check(found, "process enumeration");
  for (int i = 0; i < 200; ++i) { fd = open("/proc/meminfo", O_RDONLY); check(fd >= 0, "repeat snapshot"); close(fd); }
  printf("procfstest: all tests succeeded\n"); exit(0);
}
