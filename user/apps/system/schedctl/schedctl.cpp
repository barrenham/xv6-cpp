#include "application.h"
#include "kernel/io/scheduler.h"
#include "user/runtime/user.h"
#include "user/runtime/ui.h"

namespace
{
const char *name(unsigned kind) {
  switch (kind)
  {
  case XV_SCHED_CFS: return "cfs";
  case XV_SCHED_MLFQ: return "mlfq";
  case XV_SCHED_ROUND_ROBIN: return "round-robin";
  default: return "unknown";
  }
}
}

extern "C" int
app_main(int argc, char **argv) {
  unsigned kind = XV_SCHED_CFS;
  bool show = argc == 1 || (argc == 2 && strcmp(argv[1], "show") == 0);
  if (!show)
  {
    bool valid = argc == 2;
    if (valid && (strcmp(argv[1], "cfs") == 0 || strcmp(argv[1], "fair") == 0)) kind = XV_SCHED_CFS;
    else if (valid && strcmp(argv[1], "mlfq") == 0) kind = XV_SCHED_MLFQ;
    else if (valid && strcmp(argv[1], "round-robin") == 0) kind = XV_SCHED_ROUND_ROBIN;
    else
    {
      printf("Usage: schedctl [show|cfs|mlfq|round-robin]\nChanges take effect immediately; root required to change policy.\n");
      return argc == 2 && strcmp(argv[1], "--help") == 0 ? 0 : 1;
    }
    if (schedctl(XV_SCHED_SET, kind, nullptr) < 0)
    {
      ui::error(errno == EPERM ? "schedctl: root required\n" : "schedctl: policy change failed\n");
      return 1;
    }
  }
  xv_scheduler_status status{};
  if (schedctl(XV_SCHED_GET, 0, &status) < 0)
  {
    ui::error("schedctl: status unavailable\n");
    return 1;
  }
  ui::heading("SCHEDULER\n");
  printf("Policy:    %s\nChanges:   %lu\nReady:     %lu\nRunning:   %lu\n",
         name(status.policy), status.changes, status.ready, status.running);
  return 0;
}
