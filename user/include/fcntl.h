#pragma once
#include <sys/types.h>
#include "kernel/fs/fcntl.h"

#ifdef __cplusplus
extern "C"
{
#endif
  int fcntl(int, int, ...);

#ifdef __cplusplus
}
#endif
