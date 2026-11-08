#pragma once

#define XV_SCHED_GET 0U
#define XV_SCHED_SET 1U
#define XV_SCHED_CFS 0U
#define XV_SCHED_ROUND_ROBIN 1U
#define XV_SCHED_MLFQ 2U

struct xv_scheduler_status
{
  unsigned size, policy;
  unsigned long changes, ready, running;
};
