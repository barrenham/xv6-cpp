#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "user/fs/backends/xv6/format.h"
#include "user/runtime/proc.h"
#include "user/runtime/directory.h"

namespace
{
unsigned long
number(char *&text) {
  while (*text == ' ')
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

void
threads(int pid, char *path) {
  auto length = strlen(path);
  strcpy(path + length - 6, "threads");
  static char data[4096];
  if (proc::read(path, data, sizeof(data)) < 0)
  {
    return;
  }
  for (char *line = data; *line;)
  {
    auto tid = number(line);
    while (*line == ' ')
    {
      ++line;
    }
    char state = *line++;
    auto cpu = number(line), kib = number(line);
    while (*line == ' ')
    {
      ++line;
    }
    char *end = strchr(line, '\n');
    if (!end)
    {
      break;
    }
    *end = 0;
    printf("%6d %6lu %-5c %10lu %8lu %s\n", pid, tid, state, cpu, kib, line);
    line = end + 1;
  }
}
} // namespace

extern "C" int
app_main(int argc, char **argv) {
  bool show_threads = argc >= 2 && !strcmp(argv[1], "-T");
  int selected = argc == 3 ? atoi(argv[2]) : 0;
  if (argc > 3 || (argc > 1 && !show_threads) || (argc == 3 && selected <= 0))
  {
    ui::usage("usage: ps [-T [PID]]\n");
    exit(1);
  }
  int directory = open("/proc", O_RDONLY);
  if (directory < 0)
  {
    ui::error("ps: cannot open /proc\n");
    exit(1);
  }
  if (show_threads)
  {
    ui::heading("%6s %6s %-5s %10s %8s %s\n", "PID", "TID", "STATE", "CPU_MS", "KiB",
                "NAME");
  }
  else
  {
    ui::heading("%6s %6s %6s %6s %-5s %8s %s\n", "PID", "PPID", "PGID", "SID", "STATE",
                "KiB", "NAME");
  }
  DirectoryReader reader(directory);
  const linux_dirent64 *entry;
  while (reader.next(entry) == 1)
  {
    if (!entry->d_ino || entry->d_name[0] < '1' || entry->d_name[0] > '9')
    {
      continue;
    }
    int pid = atoi(entry->d_name);
    if (selected && pid != selected)
    {
      continue;
    }
    char path[48], data[512];
    proc::path(pid, path);
    if (show_threads)
    {
      threads(pid, path);
      continue;
    }
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
