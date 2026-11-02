#pragma once
#include "kernel/fs/format/layout.h"
#include "kernel/fs/ext3/volume.h"
#include "kernel/support/param.h"

namespace userfs::xv6
{
using Io = xv::ext3::Io;
using Error = xv::ext3::Error;
struct Inode { unsigned number = 0; dinode value{}; };
struct Entry { unsigned inode = 0; char name[DIRSIZ + 1]{}; };

// A process-serialized format engine. All changes are staged until a durable
// xv6 redo commit; failed preparation discards the entire proposed mutation.
class Volume final
{
public:
  explicit Volume(Io &io) : io_(io) {}
  ~Volume();
  bool mount();
  Error error() const { return error_; }
  unsigned inodes() const { return layout_.ninodes; }
  bool read_inode(unsigned, Inode &);
  bool lookup(unsigned parent, const char *, Inode &);
  int read(unsigned, unsigned long, void *, unsigned);
  int write(unsigned, unsigned long, const void *, unsigned);
  bool resize(unsigned, unsigned);
  bool setattr(unsigned, unsigned, unsigned, unsigned);
  bool create(unsigned parent, const char *, unsigned type, unsigned mode,
              unsigned uid, unsigned gid, unsigned major, unsigned minor, Inode &);
  bool link(unsigned parent, const char *, unsigned target);
  bool unlink(unsigned parent, const char *, Inode &removed);
  bool rename(unsigned from, const char *, unsigned to, const char *, unsigned &replaced);
  bool next_entry(unsigned, unsigned long &, Entry &);
  bool reclaim(unsigned);
  bool sync();

private:
  struct Dirty { unsigned block = 0; unsigned char *bytes = nullptr; } dirty_[LOGBLOCKS]{};
  struct Transaction;
  bool fail(Error error) { error_ = error; return false; }
  bool available();
  void discard();
  bool commit();
  bool recover();
  bool read_block(unsigned, void *);
  unsigned char *edit(unsigned, bool fresh = false);
  bool data_block(unsigned) const;
  unsigned allocate_block();
  bool free_block(unsigned);
  bool load(unsigned, Inode &, bool allocated = true);
  bool store(const Inode &);
  bool allocate_inode(unsigned type, unsigned mode, unsigned uid, unsigned gid,
                       unsigned major, unsigned minor, Inode &);
  unsigned map(Inode &, unsigned, bool create);
  int read_raw(Inode &, unsigned long, void *, unsigned);
  bool write_raw(Inode &, unsigned, const void *, unsigned);
  bool resize_raw(Inode &, unsigned);
  bool find_entry(Inode &, const char *, unsigned &offset, unsigned &number);
  bool add_entry(Inode &, const char *, unsigned);
  bool empty(Inode &);
  static bool component(const char *, bool special = false);
  static bool equal_name(const char *, const char *);
  Io &io_;
  superblock layout_{};
  unsigned first_data_ = 0, count_ = 0;
  Error error_ = Error::None;
  bool ready_ = false, failed_ = false;
};
} // namespace userfs::xv6
