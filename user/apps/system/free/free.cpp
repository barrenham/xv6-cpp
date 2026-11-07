#include "application.h"
#include "kernel/support/types.h"
#include "user/runtime/proc.h"

extern "C" int
app_main() {
  char data[512];
  if (proc::read("/proc/meminfo", data, sizeof(data)) < 0)
  {
    fprintf(2, "free: cannot read /proc/meminfo\n");
    exit(1);
  }
  int total = proc::number(data, "MemTotal"), free = proc::number(data, "MemFree");
  int available = proc::number(data, "MemAvailable");
  if (total < 0 || free < 0 || available < 0)
  {
    exit(1);
  }
  printf("%-8s %10s %10s %10s %10s (KiB)\n", "", "total", "used", "free", "available");
  printf("%-8s %10d %10d %10d %10d\n", "Mem:", total, total - free, free, available);
  exit(0);
}
