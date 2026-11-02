#pragma once

// Resource capabilities expose typed kernel facts, never filesystem paths,
// formatted /proc text, kernel pointers, or a filesystem implementation.
namespace xv::fsservice
{
inline constexpr unsigned Version = 1;
inline constexpr unsigned QueryIoctl = 0x58560002U;
inline constexpr unsigned PayloadSize = 512;
enum class Operation : unsigned
{
  Device = 1,
  Pty,
  RetainPty,
  ReleasePty,
  InspectPty
};

struct Device
{
  unsigned index, major, minor, mode;
  unsigned block, readonly;
  unsigned long bytes;
  char name[32];
};

struct Pty
{
  unsigned number, uid, gid;
};

struct Query
{
  unsigned size = sizeof(Query), version = Version;
  Operation operation = Operation::Device;
  unsigned index = 0;
  int pid = 0, result = -1;
  unsigned long offset = 0;
  unsigned count = 0;
  alignas(8) unsigned char data[PayloadSize]{};
};
static_assert(sizeof(Device) <= PayloadSize && sizeof(Pty) <= PayloadSize);
} // namespace xv::fsservice
