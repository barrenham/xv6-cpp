#pragma once
#include "buffer.h"
#include "kernel/fs/stat.h"
#include "kernel/support/param.h"

namespace vim
{
class File final
{
public:

  char path[MAXPATH]{};
  const char *error = nullptr;
  bool load(Buffer &buffer, const char *name);
  bool save(Buffer &buffer, const char *name = nullptr);

private:

  // MAXPATH is 4096; keep this in the heap-owned editor, off the small user stack.
  char temporary_[MAXPATH]{};

  bool exists_ = false;
  struct stat original_{};
  uint64 hash_ = 0;
};
} // namespace vim
