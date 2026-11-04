#pragma once

namespace xv::fsmount
{
inline constexpr unsigned Version = 2;
inline constexpr int Progress = -2;
struct Request
{
  unsigned version = Version, id = 0;
  int pid = 0, result = -1;
  char type[32]{}, path[128]{};
  char source[128]{}, options[256]{};
};
} // namespace xv::fsmount
