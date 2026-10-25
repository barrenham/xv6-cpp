module;
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"

module xv.fs:xv6;
import :filesystem;

namespace xv::fs
{
class Xv6FileSystem final : public FileSystem
{
public:

  constexpr Xv6FileSystem(BlockCache &cache, Journal &journal)
      : cache_(cache), journal_(journal)
  {
  }

  const char *name() const override
  {
    return "xv6 disk filesystem";
  }

  void initialize(int device) override;
  void initialize_inodes() override;
  Inode *allocate(unsigned device, short type) override;
  Inode *retain(Inode *node) override;
  void release(Inode *node) override;
  void lock(Inode *node) override;
  void unlock(Inode *node) override;
  void unlock_release(Inode *node) override;
  void update(Inode *node) override;
  void truncate(Inode *node) override;
  int read(Inode *node, int user, unsigned long destination, unsigned offset,
           unsigned count) override;
  int write(Inode *node, int user, unsigned long source, unsigned offset,
            unsigned count) override;
  void stat(Inode *node, Stat *result) override;
  Inode *lookup(const char *path) override;
  Inode *lookup_parent(const char *path, char *name) override;
  Inode *create(char *path, short type, short major, short minor) override;
  int link(char *old_path, char *new_path) override;
  int unlink(char *path) override;

private:

  void read_super(int device, superblock *layout);
  void zero_block(int device, int block);
  unsigned allocate_block(unsigned device);
  void free_block(int device, unsigned block);
  Inode *get(unsigned device, unsigned number);
  void free_inode(unsigned device, unsigned number);
  void reclaim(int device);
  unsigned map_block(Inode *node, unsigned block);
  static int compare_names(const char *first, const char *second);
  Inode *lookup_entry(Inode *directory, const char *name, unsigned *offset);
  int link_entry(Inode *directory, const char *name, unsigned number);
  Inode *walk_path(const char *path, int parent, char *name);
  int directory_empty(Inode *directory);
  BlockCache &cache_;
  Journal &journal_;
  superblock layout_{};
  xv::sync::SpinLock inode_lock_;
  Inode inodes_[NINODE]{};
};
} // namespace xv::fs
