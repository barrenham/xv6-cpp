#pragma once

namespace xv::fsmount
{
inline constexpr unsigned Version = 1;
struct Request
{
  unsigned version = Version, id = 0;
  int pid = 0, result = -1;
  char type[32]{}, path[128]{};
};
} // namespace xv::fsmount
