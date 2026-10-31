#pragma once
#include "kernel/support/types.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"

namespace terminal_proxy
{
// Query the child PTY, not our own input terminal. An inner proxy claims this
// flag while forwarding, so a single key always reaches the innermost proxy.
inline bool detach_requested(int master, unsigned char character)
{
  if (character != 29)
  {
    return false;
  }
  termios child{};
  return tcgetattr(master, &child) < 0 || !(child.c_lflag & XV_PROXY);
}
} // namespace terminal_proxy
