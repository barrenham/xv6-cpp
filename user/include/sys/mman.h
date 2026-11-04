#pragma once
#include <sys/types.h>
#include "kernel/io/mman.h"

#ifdef __cplusplus
extern "C"
{
#endif
  int shm_open(const char *, int, mode_t);
  int shm_unlink(const char *);
  void *mmap(void *, size_t, int, int, int, off_t);
  int munmap(void *, size_t);

#ifdef __cplusplus
}
#endif
