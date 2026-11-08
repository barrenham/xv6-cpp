#pragma once
#include "kernel/tty/termios.h"
#include "user.h"

namespace terminal_style
{
inline constexpr const char *reset = "\033[0m";
inline constexpr const char *heading = "\033[1m";

// Check the destination, not stdin: a command may read from a pipe while
// displaying on a terminal. An unset TERM keeps serial consoles usable.
inline bool
enabled(int fd) {
  const char *no_color = getenv("NO_COLOR");
  const char *term = getenv("TERM");
  if ((no_color && *no_color) || (term && !strcmp(term, "dumb")))
  {
    return false;
  }
  int saved_errno = errno;
  termios settings{};
  bool terminal = tcgetattr(fd, &settings) == 0;
  errno = saved_errno;
  return terminal;
}
} // namespace terminal_style
