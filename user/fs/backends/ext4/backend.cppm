module;
#include "volume.h"
#include "kernel/fs/userfs/protocol.h"
export module xv.userfs.ext4;
export import xv.userfs;

export namespace userfs
{
class Ext4Backend final : public Backend
{
public:
  explicit Ext4Backend(xv::ext3::Io &io) : volume_(io) {}
  bool initialize() { ready_ = volume_.mount(); return ready_; }
  xv::ext3::Error error() const { return volume_.error(); }
  unsigned flags() override { return xv::userfs::ReadOnly; }
  unsigned long operations() override
  { return xv::userfs::ReadOperations | xv::userfs::operation_bit(xv::userfs::Operation::Control); }
  const char *extension_interface() const override { return "ext4.native"; }
  unsigned extension_version() const override { return 1; }
  Attributes root() override;
  int lookup(unsigned, const char *, Attributes &) override;
  int getattr(unsigned, Attributes &) override;
  int readdir(unsigned, unsigned long, Entry &) override;
  int read(unsigned, unsigned long, void *, unsigned) override;
  void forget(unsigned) override {}
  int control(unsigned, const char *, unsigned, unsigned, unsigned,
              const void *, unsigned, void *, unsigned) override;
private:
  ext4::Volume volume_;
  bool ready_ = false;
  static unsigned disk_node(unsigned node) { return node == 1 ? 2 : node; }
  static unsigned wire_node(unsigned node) { return node == 2 ? 1 : node; }
  static bool attributes(const ext4::Inode &, Attributes &);
};
}
