#pragma once
#include "kernel/support/types.h"

// Compatibility read/snapshot record. Native getdents64 has a separate ABI.
// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

// The name field may have DIRSIZ characters and not end in a NUL
// character.
struct dirent
{
  ushort inum;
  char name[DIRSIZ] __attribute__((nonstring));
};
