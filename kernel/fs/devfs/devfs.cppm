module;
#include "kernel/support/param.h"
module xv.fs:devfs;
import :file;
namespace xv::fs
{
class DevFileSystem final : public FileSystem
{
public:
  constexpr DevFileSystem() = default;
  const char *name() const override { return "devfs"; }
  bool persistent() const override { return false; }
  bool read_only() const override { return true; }
  void initialize(int device) override;
  void initialize_inodes() override;
  Inode *root() override { return retain(&nodes_[0]); }
  xv::drivers::CharacterDevice *open_device(Inode *node, bool nonblocking) override;
  Inode *allocate(unsigned device, short type) override;
  Inode *retain(Inode *node) override;
  void release(Inode *node) override;
  void lock(Inode *node) override;
  void unlock(Inode *node) override;
  void unlock_release(Inode *node) override;
  void update(Inode *node) override;
  void truncate(Inode *node) override;
  int read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count) override;
  int write(Inode *node, int user, unsigned long source, unsigned offset, unsigned count) override;
  void stat(Inode *node, Stat *result) override;
  Inode *lookup_entry(Inode *directory, const char *name, unsigned *offset) override;
  Inode *create_at(Inode *directory, const char *name, short type, short major, short minor) override;
  int link_at(Inode *directory, const char *name, Inode *target) override;
  int unlink_at(Inode *directory, const char *name) override;
private:
  xv::sync::SpinLock references_;
  static constexpr unsigned PtsRoot = NDEV + 1;
  static constexpr unsigned PtsStart = NDEV + 2;
  Inode nodes_[PtsStart + xv::drivers::MaxNamedPtys]{};
  unsigned records_[NDEV]{};
  unsigned count_ = 0;
};
}
