#include "kernel/support/types.h"
#include "user/runtime/proc.h"
extern "C" int main()
{
  char data[512];
  if (proc::read("/proc/meminfo", data, sizeof(data)) < 0)
  { fprintf(2, "free: cannot read /proc/meminfo\n"); exit(1); }
  int total = proc::number(data, "MemTotal"), free = proc::number(data, "MemFree");
  int available = proc::number(data, "MemAvailable");
  if (total < 0 || free < 0 || available < 0) exit(1);
  printf("        total used free available (KiB)\n");
  printf("Mem:    %d %d %d %d\n", total, total - free, free, available);
  exit(0);
}
