#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/proc.h"

namespace
{
constexpr unsigned Blocks = 128, Size = 1024, Operations = 2048;
char buffer[Size];

void path(unsigned file, char *name)
{
  strcpy(name, "/cachebench0.data");
  name[11] = '0' + file;
}

unsigned long counter(char *text, const char *key)
{
  // bcache fields use whitespace, not colon, so find them line by line.
  unsigned n = strlen(key);
  for (char *line = text; line && *line;)
  {
    if (!memcmp(line, key, n) && line[n] == ' ')
    {
      unsigned long result = 0;
      for (char *digit = line + n + 1; *digit >= '0' && *digit <= '9'; ++digit)
      {
        result = result * 10 + *digit - '0';
      }
      return result;
    }
    auto next = strchr(line, '\n');
    line = next ? next + 1 : nullptr;
  }
  return 0;
}

int prepare()
{
  for (unsigned file = 0; file < 4; ++file)
  {
    char name[32];
    path(file, name);
    int fd = open(name, O_CREATE | O_WRONLY | O_TRUNC);
    if (fd < 0)
    {
      return 1;
    }
    for (unsigned block = 0; block < Blocks; ++block)
    {
      for (unsigned offset = 0; offset < Size; ++offset)
      {
        buffer[offset] = (file * 17 + block * 13 + offset) % 251;
      }
      if (write(fd, buffer, Size) != Size)
      {
        return 1;
      }
    }
    close(fd);
  }
  sync();
  return 0;
}

int reads(const char *pattern, unsigned file, unsigned operations)
{
  char name[32];
  path(file, name);
  int fd = open(name, O_RDONLY);
  if (fd < 0)
  {
    return 1;
  }
  unsigned random = 12345 + file;
  for (unsigned i = 0; i < operations; ++i)
  {
    random = random * 1664525U + 1013904223U;
    unsigned block = !strcmp(pattern, "repeat")       ? i % 16
                     : !strcmp(pattern, "sequential") ? i % Blocks
                     : !strcmp(pattern, "random")     ? (random >> 16) % Blocks
                                                      : i % 4;
    if (lseek(fd, block * Size, SEEK_SET) < 0 || read(fd, buffer, Size) != Size ||
        static_cast<unsigned char>(buffer[0]) != (file * 17 + block * 13) % 251 ||
        static_cast<unsigned char>(buffer[Size - 1]) !=
            (file * 17 + block * 13 + Size - 1) % 251)
    {
      return 1;
    }
  }
  close(fd);
  return 0;
}
} // namespace

extern "C" int main(int argc, char **argv)
{
  if (argc != 2)
  {
    exit(1);
  }
  if (!strcmp(argv[1], "prepare"))
  {
    if (prepare())
    {
      exit(1);
    }
    printf("CACHEBENCH prepared\n");
    exit(0);
  }
  bool parallel = !strcmp(argv[1], "parallel");
  if (!parallel && strcmp(argv[1], "repeat") && strcmp(argv[1], "sequential") &&
      strcmp(argv[1], "random"))
  {
    exit(1);
  }
  char before[1536], after[1536];
  if (proc::read("/proc/bcache", before, sizeof(before)) < 0)
  {
    exit(1);
  }
  auto begin = clockticks();
  int failed = 0;
  if (parallel)
  {
    unsigned children = 0;
    for (unsigned file = 0; file < 4; ++file)
    {
      int pid = fork();
      if (pid < 0)
      {
        failed = 1;
        break;
      }
      if (!pid)
      {
        exit(reads(argv[1], file, Operations / 4));
      }
      ++children;
    }
    for (unsigned child = 0; child < children; ++child)
    {
      int status = 1;
      if (wait(&status) < 0 || status)
      {
        failed = 1;
      }
    }
  }
  else
  {
    failed = reads(argv[1], 0, Operations);
  }
  auto ticks = clockticks() - begin;
  if (proc::read("/proc/bcache", after, sizeof(after)) < 0 || failed)
  {
    printf("CACHEBENCH FAIL\n");
    exit(1);
  }
  printf("CACHEBENCH %s ticks=%lu ops=%d", argv[1], ticks, Operations);
  const char *keys[] = {"hits",  "misses",     "evictions",     "lookup_steps",
                        "reads", "writes",     "locks",         "contentions",
                        "spins", "wait_ticks", "eviction_steps"};
  for (auto key : keys)
  {
    printf(" %s=%lu", key, counter(after, key) - counter(before, key));
  }
  printf("\nCACHEBENCH done\n");
  exit(0);
}
