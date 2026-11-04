#pragma once

namespace xv::bootdata
{
inline constexpr unsigned Version = 1;
inline constexpr unsigned OpenIoctl = 0x58560006U;
inline constexpr unsigned Executable = 1;
inline constexpr unsigned NameSize = 64;

// Empty name enumerates index; a nonempty name selects that exact asset.
// Successful ioctl returns an immutable descriptor and fills this record.
struct Request
{
  unsigned size = sizeof(Request), version = Version, index = 0, flags = 0;
  unsigned long bytes = 0;
  char name[NameSize]{};
};
}
