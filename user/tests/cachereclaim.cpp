#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"

namespace
{
constexpr unsigned Page = 4096, Pages = 256, Files = 6;
char *views[Files];
char paths[Files][32];
char page[Page];

void
check(bool okay, const char *why) {
  if (!okay)
  {
    fprintf(2, "cachereclaim: FAIL: %s errno=%d\n", why, errno);
    exit(1);
  }
}

unsigned long
statistic(const char *name) {
  char data[2048];
  int fd = open("/proc/meminfo", O_RDONLY);
  check(fd >= 0, "open memory statistics");
  int count = read(fd, data, sizeof(data) - 1);
  close(fd);
  check(count > 0, "read memory statistics");
  data[count] = 0;
  unsigned length = strlen(name);
  for (char *line = data; *line;)
  {
    if (strlen(line) > length && !memcmp(line, name, length) && line[length] == ':')
    {
      line += length + 1;
      while (*line == ' ')
      {
        ++line;
      }
      unsigned long result = 0;
      while (*line >= '0' && *line <= '9')
      {
        result = result * 10 + *line++ - '0';
      }
      return result;
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
  check(false, name);
  return 0;
}

void
verify() {
  for (unsigned file = 0; file < Files; ++file)
  {
    for (unsigned index = 0; index < Pages; ++index)
    {
      check(views[file][index * Page] == char('A' + file), "refaulted file content");
    }
  }
}
} // namespace

extern "C" int
main() {
  const auto before = statistic("FileMappedEvictions");
  for (unsigned file = 0; file < Files; ++file)
  {
    strcpy(paths[file], "/tmp/reclaim-0");
    paths[file][13] = '0' + file;
    int fd = open(paths[file], O_CREATE | O_TRUNC | O_RDWR);
    check(fd >= 0, "create file");
    memset(page, 'A' + file, sizeof(page));
    for (unsigned index = 0; index < Pages; ++index)
    {
      check(write(fd, page, sizeof(page)) == sizeof(page), "populate file");
    }
    views[file] =
        static_cast<char *>(mmap(nullptr, Pages * Page, PROT_READ, MAP_SHARED, fd, 0));
    check(views[file] != MAP_FAILED, "map file");
    close(fd);
  }
  int fd = open(paths[0], O_RDONLY);
  auto *private_view = static_cast<char *>(
      mmap(nullptr, Page, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0));
  close(fd);
  check(private_view != MAP_FAILED, "private mapping");
  private_view[0] = 'P';
  printf("cachereclaim: fixtures ready\n");
  verify();
  int ready[2], resume[2];
  check(pipe(ready) == 0 && pipe(resume) == 0, "child synchronization");
  int child = fork();
  check(child >= 0, "fork mapped process");
  if (!child)
  {
    close(ready[0]);
    close(resume[1]);
    private_view[0] = 'C';
    check(write(ready[1], "R", 1) == 1 && read(resume[0], page, 1) == 1,
          "sleep and resume");
    check(private_view[0] == 'C', "child private COW survives reclaim");
    verify();
    exit(0);
  }
  close(ready[1]);
  close(resume[0]);
  check(read(ready[0], page, 1) == 1, "child is ready");
  printf("cachereclaim: sleeping child ready\n");
  verify();
  const auto target = statistic("FileCacheTarget");
  bool settled = false;
  for (unsigned attempt = 0; attempt < 500; ++attempt)
  {
    if (statistic("FileCachePages") <= target && !statistic("FileCachePending") &&
        statistic("FileMappedEvictions") > before)
    {
      settled = true;
      break;
    }
    pause(1);
  }
  check(
      settled,
      "global budget reclaims mapped physical pages, including sleeping process PTEs");
  const auto pressure_before = statistic("FileMappedEvictions");
  const auto cached_before = statistic("FileCachePages");
  unsigned held = 0;
  for (unsigned attempt = 0; attempt < 128; ++attempt)
  {
    auto free_kib = statistic("MemFree");
    if (free_kib <= 1024)
    {
      break;
    }
    unsigned kib = free_kib - 1024;
    if (kib > 4096)
    {
      kib = 4096;
    }
    kib &= ~3U;
    if (!kib)
    {
      break;
    }
    check(sbrk(kib * 1024) != SBRK_ERROR, "allocate pressure heap");
    held += kib * 1024;
  }
  bool pressure_reclaimed = false;
  for (unsigned attempt = 0; attempt < 500; ++attempt)
  {
    if (statistic("FileMappedEvictions") > pressure_before &&
        statistic("FileCachePages") < cached_before && !statistic("FileCachePending"))
    {
      pressure_reclaimed = true;
      break;
    }
    pause(1);
  }
  check(sbrk(-int(held)) != SBRK_ERROR, "release pressure heap");
  check(pressure_reclaimed,
        "low physical memory triggers mapped cache reclaim below capacity");
  check(private_view[0] == 'P', "parent private COW survives reclaim");
  check(write(resume[1], "G", 1) == 1, "resume child");
  int status = -1;
  check(waitpid(child, &status, 0) == child && status == 0,
        "child refaults successfully");
  verify();
  close(ready[0]);
  close(resume[1]);
  check(munmap(private_view, Page) == 0, "unmap private page");
  for (unsigned file = 0; file < Files; ++file)
  {
    check(munmap(views[file], Pages * Page) == 0 && unlink(paths[file]) == 0,
          "release fixture");
  }
  printf(
      "cachereclaim: PASS: global mapped eviction, sleeping aliases, refault and private COW\n");
  return 0;
}
