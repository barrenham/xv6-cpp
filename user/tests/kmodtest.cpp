#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/kmodule.h"
#include "user/runtime/user.h"
#include "user/runtime/proc.h"

namespace
{
unsigned char reference[4096], buffer[4096];

void
check(bool okay, const char *why) {
  if (!okay)
  {
    fprintf(2, "kmodtest: FAIL: %s\n", why);
    exit(1);
  }
}

void
boundaries() {
  check(kmod(99, 0, 0) < 0, "unknown command");
  check(kmod(XV_KMOD_LOAD, 0, 0) < 0 &&
            kmod(XV_KMOD_LOAD, 0, XV_KMOD_MAX_FILE + 1) < 0 &&
            kmod(XV_KMOD_LOAD, ~0UL - 8, 64) < 0,
        "invalid load buffers");
  check(kmod(XV_KMOD_INFO, 0, 0) < 0 &&
            kmod(XV_KMOD_INFO, reinterpret_cast<unsigned long>(buffer), ~0UL) < 0 &&
            kmod(XV_KMOD_DEVICE, 0, 0) < 0 &&
            kmod(XV_KMOD_DEVICE, reinterpret_cast<unsigned long>(buffer), 1UL << 32) <
                0 &&
            kmod(XV_KMOD_BIND, 1UL << 32, 0) < 0 &&
            kmod(XV_KMOD_BIND, 0, 1UL << 32) < 0,
        "query and binding bounds");
  int pid = fork();
  check(pid >= 0, "credential test fork");
  if (!pid)
  {
    check(setuid(1000) == 0, "drop root privilege");
    check(kmod(XV_KMOD_INFO, reinterpret_cast<unsigned long>(buffer), 0) < 0 &&
              kmod(XV_KMOD_DEVICE, reinterpret_cast<unsigned long>(buffer), 0) < 0 &&
              kmod(XV_KMOD_LOAD, reinterpret_cast<unsigned long>(buffer), 64) < 0 &&
              kmod(XV_KMOD_BIND, 0, 0) < 0,
          "root-only module management");
    exit(0);
  }
  int status;
  check(wait(&status) == pid && !status, "credential child succeeded");
}
} // namespace

extern "C" int
main(int argc, char **argv) {
  if (argc == 4 && !strcmp(argv[1], "--failed-bind"))
  {
    unsigned module = atoi(argv[2]);
    unsigned device = atoi(argv[3]);
    check(kmod(XV_KMOD_BIND, module, device) < 0, "warm failed bind");
    char memory[512];
    check(proc::read("/proc/meminfo", memory, sizeof(memory)) >= 0,
          "read memory before retries");
    int before = proc::number(memory, "MemFree");
    for (unsigned i = 0; i < 64; ++i)
    {
      check(kmod(XV_KMOD_BIND, module, device) < 0, "failed bind retry");
    }
    check(proc::read("/proc/meminfo", memory, sizeof(memory)) >= 0,
          "read memory after retries");
    check(before > 0 && before - proc::number(memory, "MemFree") <= 64,
          "failed bind reclaims DMA pages and locks");
    printf("kmodtest: PASS: failed bind retries reclaim resources\n");
    return 0;
  }
  boundaries();
  int fd = open("/dev/vdb", O_RDONLY);
  check(fd >= 0 && read(fd, reference, sizeof(reference)) == sizeof(reference),
        "reference raw read");
  close(fd);
  int gate[2];
  check(pipe(gate) == 0, "concurrent reader gate");
  for (unsigned worker = 0; worker < 4; ++worker)
  {
    int pid = fork();
    check(pid >= 0, "reader fork");
    if (pid)
    {
      continue;
    }
    close(gate[1]);
    char token;
    check(read(gate[0], &token, 1) == 1, "start reader");
    close(gate[0]);
    int disk = open("/dev/vdb", O_RDONLY);
    check(disk >= 0, "reader device open");
    for (unsigned i = 0; i < 64; ++i)
    {
      unsigned offset = ((i * 131 + worker * 37) % 3000);
      check(lseek(disk, offset, SEEK_SET) == offset && read(disk, buffer, 777) == 777 &&
                !memcmp(buffer, reference + offset, 777),
            "concurrent unaligned read contents");
    }
    close(disk);
    exit(0);
  }
  close(gate[0]);
  check(write(gate[1], "XXXX", 4) == 4, "release all readers");
  close(gate[1]);
  for (unsigned i = 0; i < 4; ++i)
  {
    int status;
    check(wait(&status) > 0 && !status, "reader completed");
  }
  printf("kmodtest: PASS: bounds, credentials, concurrent IRQ-driven block reads\n");
  return 0;
}
