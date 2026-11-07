module;
#include "kernel/fs/stat.h"

export module xv.fs:filesystem;

export import :inode;
export import :buffer;
export import xv.drivers;

export namespace xv::fs
{
struct DirectoryRecord
{
  unsigned long inode = 0;
  unsigned char type = 0;
  char name[256]{};
};

class FileSystem
{
public:

  FileSystem(const FileSystem &) = delete;
  FileSystem &operator=(const FileSystem &) = delete;
  virtual const char *name() const = 0;

  virtual unsigned
  name_max() const {
    return 14;
  }

  virtual const char *
  type_name() const {
    return name();
  }

  virtual bool attributes(Inode *, unsigned, unsigned, unsigned) = 0;

  virtual bool
  busy() {
    return true;
  }

  virtual void
  shutdown() {
  }

  virtual bool
  persistent() const {
    return true;
  }

  virtual Inode *root() = 0;
  virtual Inode *lookup_entry(Inode *directory, const char *name, unsigned *offset) = 0;
  virtual Inode *create_at(Inode *directory, const char *name, short type, short major,
                           short minor) = 0;
  virtual int link_at(Inode *directory, const char *name, Inode *target) = 0;
  virtual int unlink_at(Inode *directory, const char *name) = 0;

  virtual int
  rename_at(Inode *, const char *, Inode *, const char *) {
    return -1;
  }

  virtual Inode *retain(Inode *node) = 0;
  virtual void release(Inode *node) = 0;
  virtual void lock(Inode *node) = 0;
  virtual void unlock(Inode *node) = 0;
  virtual void unlock_release(Inode *node) = 0;

  virtual void
  lock_data(Inode *node) {
    lock(node);
  }

  virtual void
  unlock_data(Inode *node) {
    unlock(node);
  }

  virtual void
  lock_read(Inode *node) {
    lock_data(node);
  }

  virtual void
  unlock_read(Inode *node) {
    unlock_data(node);
  }
  virtual int read_data(Inode *node, int user, unsigned long address, unsigned offset,
                         unsigned count) { return read(node, user, address, offset, count); }
  virtual int write_data(Inode *node, int user, unsigned long address, unsigned offset,
                          unsigned count) { return write(node, user, address, offset, count);
  }

  virtual int
  write_cached_data(Inode *node, int user, unsigned long address, unsigned offset,
                    unsigned count) {
    return write_data(node, user, address, offset, count);
  }

  virtual bool
  publishes_cached_writes(Inode *) const {
    return false;
  }

  using CacheCopy = int (*)(void *, void *, unsigned, unsigned);
  int cache_written(Inode *, unsigned old_size, unsigned offset, unsigned count,
                    CacheCopy, void *context);

  // read_file allows shared data ownership; mutations require exclusive ownership.
  int read_file(Inode *, int user, unsigned long, unsigned offset, unsigned count);
  int write_file(Inode *, int user, unsigned long, unsigned offset, unsigned count);
  int resize_file(Inode *, unsigned size);
  int flush_mapped(Inode *);
  int sync_file(Inode *);
  void discard_file_cache(Inode *);

  virtual void
  record_read_hit() {
  }

  virtual bool mappable() const { return false; }

  virtual int
  resize(Inode *, unsigned) {
    return -1;
  }

  virtual int
  control(Inode *, unsigned long, unsigned) {
    return -1;
  }

  virtual int synchronize(Inode *) = 0;
  virtual int read(Inode *node, int user, unsigned long destination, unsigned offset,
                   unsigned count) = 0;
  virtual int write(Inode *node, int user, unsigned long source, unsigned offset,
                    unsigned count) = 0;
  virtual void stat(Inode *node, Stat *result) = 0;

  virtual int
  status(Inode *node, Stat *result) {
    stat(node, result);
    return 0;
  }

  virtual xv::drivers::CharacterDevice *
  open_device(Inode *, bool) {
    return nullptr;
  }

  virtual xv::drivers::BlockDevice *
  open_block_device(Inode *, bool) {
    return nullptr;
  }

  // Cookie is backend-specific. Returns 1 entry, 0 EOF, or -1 on failure.
  virtual int
  next_directory(Inode *, unsigned long &, DirectoryRecord &) {
    return -1;
  }

  virtual bool
  snapshot_reads() const {
    return false;
  }

  virtual bool
  read_only() const {
    return false;
  }

  virtual bool
  snapshot(Inode *, PageBuffer &) {
    return false;
  }

protected:

  constexpr FileSystem() = default;
  ~FileSystem() = default;
};

class LockedInode final
{
public:

  LockedInode(FileSystem &fs, Inode &node, bool data = false)
      : fs_(fs), node_(node), data_(data) {
    if (data_) { fs_.lock_data(&node_); } else { fs_.lock(&node_); }
  }

  ~LockedInode() {
    if (data_) { fs_.unlock_data(&node_); } else { fs_.unlock(&node_); }
  }

  LockedInode(const LockedInode &) = delete;
  LockedInode &operator=(const LockedInode &) = delete;

private:

  FileSystem &fs_;
  Inode &node_;
  bool data_;
};

class ReadLockedInode final
{
public:

  ReadLockedInode(FileSystem &fs, Inode &node);
  ~ReadLockedInode();

  ReadLockedInode(const ReadLockedInode &) = delete;
  ReadLockedInode &operator=(const ReadLockedInode &) = delete;
  static bool held();

private:

  FileSystem &fs_;
  Inode &node_;
  int pid_ = 0;
  ReadLockedInode *next_ = nullptr;
  static void link(ReadLockedInode &);
  static void unlink(ReadLockedInode &);
};
} // namespace xv::fs
