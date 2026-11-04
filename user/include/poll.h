#pragma once
#include "kernel/fs/poll.h"

#ifdef __cplusplus
extern "C"
{
#endif
  typedef unsigned long nfds_t;
  int poll(struct pollfd *, nfds_t, int);

#ifdef __cplusplus
}
#endif
