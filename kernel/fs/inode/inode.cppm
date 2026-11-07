module;
#include "kernel/fs/stat.h"

export module xv.fs:inode;
import xv.sync;

export namespace xv::fs
{
using Stat = ::stat;
class FileSystem;
class FileMapping;
struct MappedPages;

class Inode final
{
public:

  constexpr Inode() = default;
  Inode(const Inode &) = delete;
  Inode &operator=(const Inode &) = delete;

  unsigned
  uid() const {
    return uid_;
  }

  unsigned
  gid() const {
    return gid_;
  }

  unsigned
  mode() const {
    return mode_;
  }

  void
  security_stat(Stat *result) const {
    result->uid = uid_;
    result->gid = gid_;
    result->mode = mode_;
  }

  short
  kind() const {
    return type;
  }

  FileSystem &
  owner() const {
    return *owner_;
  }

  short
  device_minor() const {
    return minor;
  }

  bool
  linked() const {
    return nlink != 0;
  }

  short
  device_major() const {
    return major;
  }

  unsigned
  device() const {
    return dev;
  }

  unsigned
  number() const {
    return inum;
  }

  unsigned
  size_bytes() const {
    return size;
  }

private:

  friend class FileSystem;
  friend class FileMapping;
  friend struct MappedPages;
  friend class UserFileSystem;
  FileSystem *owner_ = nullptr;
  MappedPages *mapped_pages_ = nullptr;
  unsigned mapped_length_ = 0;
  unsigned dev = 0;
  unsigned inum = 0;
  int ref = 0;
  xv::sync::SleepRwLock data_lock_;
  xv::sync::SpinLock cache_state_;
  unsigned long write_epoch_ = 0;
  bool metadata_valid_ = false;
  short type = 0;
  short major = 0;
  short minor = 0;
  short nlink = 0;
  unsigned uid_ = 0, gid_ = 0;
  unsigned short mode_ = 0;
  unsigned size = 0;
};
} // namespace xv::fs
