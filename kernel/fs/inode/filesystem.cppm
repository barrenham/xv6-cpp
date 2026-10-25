module;
#include "kernel/fs/stat.h"

export module xv.fs:filesystem;

export import :inode;
export import :journal;

export namespace xv::fs
{
class FileSystem
{
public:

  FileSystem(const FileSystem &) = delete;
  FileSystem &operator=(const FileSystem &) = delete;
  virtual const char *name() const = 0;
  virtual void initialize(int device) = 0;
  virtual void initialize_inodes() = 0;
  virtual Inode *allocate(unsigned device, short type) = 0;
  virtual Inode *retain(Inode *node) = 0;
  virtual void release(Inode *node) = 0;
  virtual void lock(Inode *node) = 0;
  virtual void unlock(Inode *node) = 0;
  virtual void unlock_release(Inode *node) = 0;
  virtual void update(Inode *node) = 0;
  virtual void truncate(Inode *node) = 0;
  virtual int read(Inode *node, int user, unsigned long destination, unsigned offset,
                   unsigned count) = 0;
  virtual int write(Inode *node, int user, unsigned long source, unsigned offset,
                    unsigned count) = 0;
  virtual void stat(Inode *node, Stat *result) = 0;
  virtual Inode *lookup(const char *path) = 0;
  virtual Inode *lookup_parent(const char *path, char *name) = 0;
  virtual Inode *create(char *path, short type, short major, short minor) = 0;
  virtual int link(char *old_path, char *new_path) = 0;
  virtual int unlink(char *path) = 0;

protected:

  constexpr FileSystem() = default;
  ~FileSystem() = default;
};

FileSystem &filesystem();

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
