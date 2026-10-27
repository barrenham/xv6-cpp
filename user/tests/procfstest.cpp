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
void descriptor_path(int fd, char *path)
{
  strcpy(path, "/proc/self/fd/"); unsigned used = strlen(path);
  char digits[12]; unsigned count = 0;
  do { digits[count++] = '0' + fd % 10; fd /= 10; } while (fd);
  while (count) path[used++] = digits[--count];
  path[used] = 0;
}
void descriptors()
{
  char data[512], path[64];
  check(proc::read("/proc/self/status", data, sizeof(data)) > 0 && proc::number(data, "Pid") == getpid(), "self alias");
  int file = open("/tmp/fd-target", O_CREATE | O_RDWR);
  check(file >= 0 && fcntl(file, F_SETFL, O_NONBLOCK) == 0, "descriptor fixture");
  int alias = dup(file); check(alias >= 0, "descriptor duplicate");
  descriptor_path(file, path);
  check(proc::read(path, data, sizeof(data)) > 0 && proc::number(data, "Descriptor") == file &&
        (proc::number(data, "Flags") & O_NONBLOCK), "descriptor flags");
  int identity = proc::number(data, "Identity");
  descriptor_path(alias, path);
  check(proc::read(path, data, sizeof(data)) > 0 && proc::number(data, "Identity") == identity, "dup identity");
  descriptor_path(file, path); int snapshot = open(path, O_RDONLY); check(snapshot >= 0, "descriptor content snapshot");
  close(file);
  int replacement = open("/tmp/fd-other", O_CREATE | O_RDWR); check(replacement == file, "descriptor slot reuse");
  check(proc::read(path, data, sizeof(data)) > 0 && proc::number(data, "Identity") != identity, "reused slot new identity");
  check(read_all(snapshot, data, sizeof(data), 1) > 0 && proc::number(data, "Identity") == identity, "old snapshot retains identity");
  close(snapshot); close(alias); close(replacement);
  int fds[2]; check(pipe(fds) == 0, "pipe observation"); descriptor_path(fds[0], path);
  check(proc::read(path, data, sizeof(data)) > 0 && !memcmp(proc::field(data, "Type"), "pipe", 4), "pipe kind");
  close(fds[0]); close(fds[1]);
  int socket_fd = socket(AF_INET, SOCK_DGRAM, 0); check(socket_fd >= 0, "socket observation"); descriptor_path(socket_fd, path);
  check(proc::read(path, data, sizeof(data)) > 0 && !memcmp(proc::field(data, "Type"), "socket", 6), "socket kind"); close(socket_fd);
  check(unlink("/tmp/fd-target") == 0 && unlink("/tmp/fd-other") == 0, "descriptor fixture cleanup");
}
void descriptor_race()
{
  int child = fork(); check(child >= 0, "descriptor race fork");
  if (!child)
  {
    for (int i = 0; i < 500; ++i)
    {
      int fd = open("/tmp/fd-race", O_CREATE | O_RDWR);
      check(fd >= 0 && write(fd, "x", 1) == 1 && unlink("/tmp/fd-race") == 0, "descriptor race writer");
      close(fd);
    }
    exit(0);
  }
  char path[64]; proc::path(child, path); strcpy(path + strlen(path) - 6, "fd");
  for (int i = 0; i < 200; ++i)
  {
    int fd = open(path, O_RDONLY);
    if (fd < 0) break;
    dirent entry; while (read(fd, &entry, sizeof(entry)) == sizeof(entry)) {}
    close(fd);
  }
  int status; check(wait(&status) == child && status == 0, "descriptor race child");
}
void large_directory()
{
  constexpr int Children = 300;
  int gate[2]; check(pipe(gate) == 0, "large process gate");
  for (int i = 0; i < Children; ++i)
  {
    int child = fork(); check(child >= 0, "large process fork");
    if (!child) { close(gate[1]); char byte; read(gate[0], &byte, 1); close(gate[0]); exit(0); }
  }
  close(gate[0]);
  int directory = open("/proc", O_RDONLY); check(directory >= 0, "large proc open");
  struct stat metadata{}; check(fstat(directory, &metadata) == 0 && metadata.size > 4096, "dynamic proc snapshot size");
  dirent entry{}; int records = 0;
  while (read(directory, &entry, sizeof(entry)) == sizeof(entry)) if (entry.name[0] >= '1' && entry.name[0] <= '9') ++records;
  close(directory); check(records >= Children + 2, "all live processes listed");
  close(gate[1]);
  for (int i = 0; i < Children; ++i) { int status; check(wait(&status) > 0 && status == 0, "large process reap"); }
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
  descriptors(); descriptor_race(); large_directory();
  printf("procfstest: all tests succeeded\n"); exit(0);
}
