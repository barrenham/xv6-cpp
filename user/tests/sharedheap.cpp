#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/support/placement_new.hpp"
import xv.userfs.shared;

namespace
{
void
check(bool okay, const char *why) {
  if (!okay)
  {
    fprintf(2, "sharedheap: FAIL: %s errno=%d\n", why, errno);
    exit(1);
  }
}

unsigned long
free_kib() {
  static char bytes[2048];
  int fd = open("/proc/meminfo", O_RDONLY);
  check(fd >= 0, "memory information");
  int count = read(fd, bytes, sizeof(bytes) - 1);
  close(fd);
  check(count > 0, "memory information contents");
  bytes[count] = 0;
  for (char *line = bytes; *line;)
  {
    if (strlen(line) >= 8 && !memcmp(line, "MemFree:", 8))
    {
      line += 8;
      while (*line == ' ')
      {
        ++line;
      }
      unsigned long value = 0;
      while (*line >= '0' && *line <= '9')
      {
        value = value * 10 + *line++ - '0';
      }
      return value;
    }
    while (*line && *line != '\n')
    {
      ++line;
    }
    if (*line)
    {
      ++line;
    }
  }
  check(false, "free physical memory field");
  return 0;
}

struct Probe
{
  char *data = nullptr;
  unsigned completed[2]{};
};
} // namespace

extern "C" int
main() {
  auto before = free_kib();
  userfs::SharedStorage storage;
  check(storage.ready(), "initialize shared arena and capability queue");
  auto after = free_kib();
  check(after >= before || before - after < 4096,
        "reserving 16 MiB virtual capacity does not commit it physically");
  void *memory = storage.allocate(sizeof(Probe));
  check(memory, "shared probe");
  auto *probe = new (memory) Probe{};
  int ready[2], resume[2];
  check(pipe(ready) == 0 && pipe(resume) == 0, "handoff pipes");
  int child = fork();
  check(child >= 0, "allocation owner fork");
  if (!child)
  {
    close(ready[0]);
    close(resume[1]);
    probe->data = static_cast<char *>(storage.allocate(128 * 1024));
    check(probe->data, "child grows shared object");
    memset(probe->data, 'C', 128 * 1024);
    char token = 0;
    check(write(ready[1], "R", 1) == 1 && read(resume[0], &token, 1) == 1,
          "allocation handoff");
    check(probe->data[0] == 'P' && probe->data[128 * 1024 - 1] == 'P',
          "parent reuse is visible in child");
    exit(0);
  }
  close(ready[1]);
  close(resume[0]);
  char token = 0;
  check(read(ready[0], &token, 1) == 1 && token == 'R' && probe->data &&
            probe->data[0] == 'C' && probe->data[128 * 1024 - 1] == 'C',
        "child allocation metadata and bytes are visible to parent");
  auto *original = probe->data;
  storage.release(original);
  probe->data = static_cast<char *>(storage.allocate(128 * 1024));
  check(probe->data == original,
        "another process can free and reuse the same shared block");
  memset(probe->data, 'P', 128 * 1024);
  check(write(resume[1], "G", 1) == 1, "resume allocation owner");
  int status = -1;
  check(waitpid(child, &status, 0) == child && status == 0, "handoff child completed");
  close(ready[0]);
  close(resume[1]);
  storage.release(probe->data);
  int children[2];
  for (unsigned worker = 0; worker < 2; ++worker)
  {
    children[worker] = fork();
    check(children[worker] >= 0, "allocator worker fork");
    if (!children[worker])
    {
      for (unsigned round = 0; round < 64; ++round)
      {
        unsigned bytes = 4112 + (round % 7) * 16;
        auto *data = static_cast<unsigned char *>(storage.allocate(bytes));
        check(data, "concurrent allocation");
        memset(data, worker + 1, bytes);
        sched_yield();
        check(data[0] == worker + 1 && data[bytes - 1] == worker + 1,
              "concurrent allocation contents do not overlap");
        storage.release(data);
        ++probe->completed[worker];
      }
      exit(0);
    }
  }
  for (int pid : children)
  {
    check(waitpid(pid, &status, 0) == pid && status == 0, "allocator worker completed");
  }
  check(probe->completed[0] == 64 && probe->completed[1] == 64,
        "all concurrent allocations observed");
  probe->~Probe();
  storage.release(probe);
  printf(
      "sharedheap: PASS: sparse capacity, cross-process growth, ownership transfer and concurrent reuse\n");
  return 0;
}
