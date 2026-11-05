#pragma once

struct timespec
{
  long tv_sec;
  long tv_nsec;
};

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
