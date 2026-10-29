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

  Inode *root() override
  {
    return get(device_, ROOTINO);
  }

  Inode *lookup_entry(Inode *directory, const char *name, unsigned *offset) override;
  Inode *create_at(Inode *directory, const char *name, short type, short major,
                   short minor) override;
  int link_at(Inode *directory, const char *name, Inode *target) override;
  int unlink_at(Inode *directory, const char *name) override;
  int rename_at(Inode *from, const char *old_name, Inode *to,
                const char *new_name) override;
  xv::drivers::CharacterDevice *open_device(Inode *node, bool nonblocking) override;
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
  int resize(Inode *node, unsigned size) override;

  void flush() override
  {
    cache_.flush();
  }

  bool busy() override;

  Journal *transaction_log() override
  {
    return &journal_;
  }

  int read(Inode *node, int user, unsigned long destination, unsigned offset,
           unsigned count) override;
  int write(Inode *node, int user, unsigned long source, unsigned offset,
            unsigned count) override;
  void stat(Inode *node, Stat *result) override;

private:

  struct LinkFlow;
  struct UnlinkFlow;
  struct CreateFlow;
  struct RenameFlow;

  struct InitializeFlow;
  struct AllocateBlockFlow;
  struct AllocateFlow;
  struct GetFlow;
  struct LockFlow;
  struct ReleaseFlow;
  struct ReclaimFlow;
  struct MapBlockFlow;
  struct TruncateFlow;
  struct ReadFlow;
  struct WriteFlow;
  struct ResizeFlow;
  struct BlockIoProtocol;

  void read_super(int device, superblock *layout);
  void zero_block(int device, int block);
  unsigned allocate_block(unsigned device);
  void free_block(int device, unsigned block);
  Inode *get(unsigned device, unsigned number);
  void free_inode(unsigned device, unsigned number);
  void reclaim(int device);
  unsigned map_block(Inode *node, unsigned block, bool create = true);
  static int compare_names(const char *first, const char *second);
  int link_entry(Inode *directory, const char *name, unsigned number);
  int directory_empty(Inode *directory);
  unsigned device_ = ROOTDEV;
  BlockCache &cache_;
  Journal &journal_;
  superblock layout_{};
  xv::sync::SpinLock inode_lock_;
  Inode inodes_[NINODE]{};
};
} // namespace xv::fs
