module;
#include "kernel/fs/stat.h"

export module xv.fs:filesystem;

export import :inode;
export import :buffer;
export import :journal;
export import xv.drivers;

export namespace xv::fs
{
struct DirectoryRecord {unsigned long inode=0;unsigned char type=0;char name[256]{};};

class FileSystem
{
public:

  FileSystem(const FileSystem &) = delete;
  FileSystem &operator=(const FileSystem &) = delete;
  virtual const char *name() const = 0;
  virtual unsigned name_max() const {return 14;}
  virtual const char *type_name() const {return name();}
  virtual void initialize(int device) = 0;
  virtual void initialize_inodes() = 0;
  void attributes(Inode *node, unsigned uid, unsigned gid, unsigned mode)
  { node->uid_ = uid; node->gid_ = gid; node->mode_ = mode; update(node); }
  virtual bool initialize_mount(unsigned) { return false; }
  virtual bool busy() { return true; }
  virtual void shutdown() {}
  virtual bool persistent() const { return true; }
  virtual Inode *root() = 0;
  virtual Inode *lookup_entry(Inode *directory, const char *name, unsigned *offset) = 0;
  virtual Inode *create_at(Inode *directory, const char *name, short type, short major, short minor) = 0;
  virtual int link_at(Inode *directory, const char *name, Inode *target) = 0;
  virtual int unlink_at(Inode *directory, const char *name) = 0;
  virtual int rename_at(Inode *, const char *, Inode *, const char *) { return -1; }
  virtual Inode *allocate(unsigned device, short type) = 0;
  virtual Inode *retain(Inode *node) = 0;
  virtual void release(Inode *node) = 0;
  virtual void lock(Inode *node) = 0;
  virtual void unlock(Inode *node) = 0;
  virtual void unlock_release(Inode *node) = 0;
  virtual void update(Inode *node) = 0;
  virtual void truncate(Inode *node) = 0;
  virtual int resize(Inode *, unsigned) { return -1; }
  virtual void flush() {}
  virtual Journal *transaction_log() { return nullptr; }
  virtual int read(Inode *node, int user, unsigned long destination, unsigned offset,
                   unsigned count) = 0;
  virtual int write(Inode *node, int user, unsigned long source, unsigned offset,
                    unsigned count) = 0;
  virtual void stat(Inode *node, Stat *result) = 0;
  virtual xv::drivers::CharacterDevice *open_device(Inode *, bool) { return nullptr; }
  virtual xv::drivers::BlockDevice *open_block_device(Inode *, bool) { return nullptr; }
  // Cookie is backend-specific. Returns 1 entry, 0 EOF, or -1 on failure.
  virtual int next_directory(Inode *,unsigned long &,DirectoryRecord &) {return -1;}
  int legacy_next_directory(Inode *,unsigned long &,DirectoryRecord &);
  virtual bool snapshot_reads() const { return false; }
  virtual bool read_only() const { return false; }
  virtual bool snapshot(Inode *, PageBuffer &) { return false; }

protected:

  constexpr FileSystem() = default;
  ~FileSystem() = default;
};


class LockedInode final
{
public:

  LockedInode(FileSystem &fs, Inode &node) : fs_(fs), node_(node)
  {
    fs_.lock(&node_);
  }

  ~LockedInode()
  {
    fs_.unlock(&node_);
  }

  LockedInode(const LockedInode &) = delete;
  LockedInode &operator=(const LockedInode &) = delete;

private:

  FileSystem &fs_;
  Inode &node_;
};
} // namespace xv::fs
