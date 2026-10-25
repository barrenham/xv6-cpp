module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"

export module xv.fs:inode;
import xv.sync;

export namespace xv::fs
{
using Stat = ::stat;
class Xv6FileSystem;

class Inode final
{
public:

  constexpr Inode() = default;
  Inode(const Inode &) = delete;
  Inode &operator=(const Inode &) = delete;

  short kind() const
  {
    return type;
  }

  short device_major() const
  {
    return major;
  }

  unsigned device() const
  {
    return dev;
  }

  unsigned number() const
  {
    return inum;
  }

  unsigned size_bytes() const
  {
    return size;
  }

private:

  friend class Xv6FileSystem;
  unsigned dev = 0;
  unsigned inum = 0;
  int ref = 0;
  xv::sync::SleepLock lock;
  int valid = 0;
  short type = 0;
  short major = 0;
  short minor = 0;
  short nlink = 0;
  unsigned size = 0;
  unsigned addrs[NDIRECT + 1]{};
};
} // namespace xv::fs
