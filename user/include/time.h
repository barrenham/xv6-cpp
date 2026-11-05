#pragma once
#include "kernel/io/time.h"
typedef int clockid_t;
typedef long time_t;
#ifdef __cplusplus
extern "C"
{
#endif
  int clock_gettime(clockid_t, struct timespec *);
#ifdef __cplusplus
}
#endif
