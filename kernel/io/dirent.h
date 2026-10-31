#pragma once
#include "kernel/support/types.h"
// RV64 Linux dirent64 record layout; records are padded to an 8-byte boundary.
struct linux_dirent64
{
  uint64 d_ino;
  long long d_off;
  unsigned short d_reclen;
  unsigned char d_type;
  char d_name[];
};
inline constexpr unsigned DIRENT64_NAME_OFFSET=19;
inline constexpr unsigned DIRENT64_MAX_RECORD=280;
static_assert(__builtin_offsetof(linux_dirent64,d_name)==DIRENT64_NAME_OFFSET);
