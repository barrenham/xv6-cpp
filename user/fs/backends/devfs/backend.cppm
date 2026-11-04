module;
#include "kernel/support/types.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/io/fs_resources.h"
#include "user/runtime/user.h"
export module xv.userfs.devfs;
export import xv.userfs;
import xv.userfs.resources;

export namespace userfs
{
class DeviceBackend final : public Backend
{
public:

  explicit DeviceBackend(int resources) : resources_(resources) {}
  bool ready() const { return resources_.ready(); }
  unsigned flags() override { return xv::userfs::ReadOnly; }
  int lookup(unsigned, const char *, Attributes &) override;
  int getattr(unsigned, Attributes &) override;
  int readdir(unsigned, unsigned long, Entry &) override;
  int read(unsigned, unsigned long, void *, unsigned) override;
  void forget(unsigned) override;

private:

  static constexpr unsigned Root = 1, Pts = 2, DeviceBase = 16, PtyBase = 256;
  static bool decimal(const char *, unsigned &);
  static void name(unsigned, char *);
  bool device(unsigned, Attributes &, char * = nullptr);
  bool pty(unsigned, Attributes &, bool);
  Resources resources_;
};
} // namespace userfs
