#include "user/runtime/user.h"
#include "user/runtime/proc.h"
#include "kernel/fs/fcntl.h"
#include "kernel/process/wait/wait.h"

namespace
{
constexpr unsigned Clients = 2, Rounds = 256, Bytes = 4096;

struct Results
{
  unsigned long total[Clients], maximum[Clients];
} *results;

void
check(bool ok, const char *why) {
  if (!ok)
  {
    fprintf(2, "fspoolbench: FAIL: %s errno=%d\n", why, errno);
    exit(1);
  }
}

unsigned long
decimal(const char *&text) {
  while (*text == ' ' || *text == '\t')
  {
    ++text;
  }
  unsigned long value = 0;
  while (*text >= '0' && *text <= '9')
  {
    value = value * 10 + *text++ - '0';
  }
  return value;
}

unsigned long
cpu_ticks() {
  static char text[4096];
  check(proc::read("/proc/stat", text, sizeof(text)) > 0, "read CPU statistics");
  const char *at = text + 4;
  auto user = decimal(at);
  decimal(at);
  auto system = decimal(at);
  return user + system;
}

int
free_kib() {
  static char text[2048];
  check(proc::read("/proc/meminfo", text, sizeof(text)) > 0, "read memory statistics");
  return proc::number(text, "MemFree");
}
} // namespace

extern "C" int
main() {
  results = static_cast<Results *>(
      mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
  check(results != MAP_FAILED, "shared client measurements");
  int before = free_kib();
  check(!mkdir("/tmp/poolbench") &&
            !mountfs("ramfs", "/tmp/poolbench", nullptr, "workers=2"),
        "mount backend");
  int files[Clients], children[Clients];
  static char buffer[Bytes];
  memset(buffer, 'Q', Bytes);
  for (unsigned i = 0; i < Clients; ++i)
  {
    char path[] = "/tmp/poolbench/a";
    path[15] += i;
    files[i] = open(path, O_CREATE | O_RDWR);
    check(files[i] >= 0, "create independent file");
    for (unsigned n = 0; n < 128; ++n)
    {
      check(write(files[i], buffer, Bytes) == Bytes, "prepare file pages");
    }
  }
  int warmed = free_kib();
  auto cpu_start = cpu_ticks(), start = clockticks();
  for (unsigned i = 0; i < Clients; ++i)
  {
    children[i] = fork();
    check(children[i] >= 0, "fork clients");
    if (!children[i])
    {
      unsigned long total = 0, maximum = 0;
      for (unsigned n = 0; n < Rounds; ++n)
      {
        auto began = clockticks();
        check(lseek(files[i], (n % 128) * Bytes, SEEK_SET) >= 0 &&
                  write(files[i], buffer, Bytes) == Bytes,
              "write file");
        check(lseek(files[i], (n % 128) * Bytes, SEEK_SET) >= 0 &&
                  read(files[i], buffer, Bytes) == Bytes,
              "read file");
        for (char byte : buffer)
        {
          check(byte == 'Q', "readback integrity");
        }
        auto elapsed = clockticks() - began;
        total += elapsed;
        if (elapsed > maximum)
        {
          maximum = elapsed;
        }
      }
      results->total[i] = total;
      results->maximum[i] = maximum;
      exit(0);
    }
  }
  for (int child : children)
  {
    int status;
    check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
              !WEXITSTATUS(status),
          "client completion");
  }
  auto elapsed = clockticks() - start, cpu = cpu_ticks() - cpu_start;
  unsigned long total = 0, maximum = 0;
  for (unsigned i = 0; i < Clients; ++i)
  {
    total += results->total[i];
    if (results->maximum[i] > maximum)
    {
      maximum = results->maximum[i];
    }
    close(files[i]);
  }
  printf(
      "fspoolbench: ops=%u bytes=%u elapsed_us=%lu mean_pair_us=%lu max_pair_us=%lu cpu_ticks=%lu mount_and_file_kib=%d PASS\n",
      Clients * Rounds * 2, Clients * Rounds * Bytes * 2, elapsed / 10,
      total / (Clients * Rounds * 10), maximum / 10, cpu, before - warmed);
  check(!umount("/tmp/poolbench") && !unlink("/tmp/poolbench"), "unmount");
  munmap(results, 4096);
  return 0;
}
