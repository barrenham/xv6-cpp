#pragma once
#include "kernel/fs/stat.h"

namespace listing
{
inline const char *
name_color(const struct stat &entry) {
  switch (entry.type)
  {
  case T_DIR:
    return "\033[1;34m";
  case T_SYMLINK:
    return "\033[35m";
  case T_DEVICE:
  case T_BLOCK:
    return "\033[1;33m";
  case T_SOCKET:
    return "\033[1;35m";
  case T_FILE:
    return entry.mode & 0111 ? "\033[1;32m" : "";
  default:
    return "";
  }
}
} // namespace listing
