module;
#include "kernel/support/types.h"
#include "kernel/fs/userfs/protocol.h"
#include "user/fs/backends/ext3/volume.h"
export module xv.userfs.ext3;
export import xv.userfs;

export namespace userfs
{
// All directory, allocation, metadata, orphan and journal policy runs here in
// userspace. The kernel sees object IDs and framed requests, not ext3 structures.
class Ext3Backend final : public Backend
{
public:
  unsigned cache_flags() const override { return xv::userfs::CacheMetadata; }

  explicit Ext3Backend(xv::ext3::Io &io) : volume_(io), io_(io) {
  }

  ~Ext3Backend();
  bool initialize();

  unsigned long
  operations() override {
    return xv::userfs::AllOperations & ~xv::userfs::SnapshotOperations;
  }

  const char *
  extension_interface() const override {
    return "ext3.native";
  }

  unsigned
  extension_version() const override {
    return 1;
  }

  int control(unsigned, const char *, unsigned, unsigned, unsigned, const void *,
              unsigned, void *, unsigned) override;

  xv::ext3::Error
  error() const {
    return volume_.error();
  }

  Attributes root() override;
  int lookup(unsigned, const char *, Attributes &) override;
  int getattr(unsigned, Attributes &) override;
  int readdir(unsigned, unsigned long, Entry &) override;
  int read(unsigned, unsigned long, void *, unsigned) override;
  int write(unsigned, unsigned long, const void *, unsigned) override;
  int create(unsigned, const char *, const Attributes &, Attributes &) override;
  int resize(unsigned, unsigned) override;
  int setattr(unsigned, const Attributes &) override;
  int link(unsigned, const char *, unsigned) override;
  int unlink(unsigned, const char *, Attributes &) override;
  int rename(unsigned, const char *, unsigned, const char *) override;
  int sync(unsigned) override;
  void forget(unsigned) override;

  bool
  maintenance_pending() const override {
    return cleanup_pending_;
  }

  bool maintain() override;

  unsigned
  maintenance_delay_ms() const override {
    return cleanup_retry_ ? 10 : 0;
  }

private:

  static unsigned
  number(unsigned node) {
    return node == 1 ? 2 : node;
  }

  static unsigned
  identity(unsigned inode) {
    return inode == 2 ? 1 : inode;
  }

  static bool attributes(const xv::ext3::Inode &, Attributes &);
  bool inode(unsigned, xv::ext3::Inode &);
  bool borrow(unsigned);
  bool retained(unsigned) const;
  bool commit();
  bool collect(unsigned);
  bool maintenance_failure();
  xv::ext3::Volume volume_;
  xv::ext3::Io &io_;
  unsigned live_[128]{};
  bool ready_ = false, failed_ = false;
  bool cleanup_pending_ = false, cleanup_retry_ = false;
};
} // namespace userfs
