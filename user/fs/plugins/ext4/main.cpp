#include "user/runtime/user.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/ext3/format.h"
import xv.userfs.plugin;
import xv.userfs.ext4;
import xv.userfs.block;

extern "C" int main(int argc, char **argv)
{
  userfs::plugin::Invocation invocation(argc, argv);
  if (!invocation.valid) return 2;
  userfs::BlockIo io(userfs::plugin::Source);
  if (invocation.probe)
  {
    unsigned char super[128];
    if (!io.read(1024, super, sizeof(super))) return 1;
    return xv::ext3::le16(super + 56) == xv::ext3::Magic &&
           (xv::ext3::le32(super + 96) & 0x40) ? 0 : 1;
  }
  void *storage = malloc(sizeof(userfs::Ext4Backend));
  if (!storage) return 1;
  auto &backend = *new (storage) userfs::Ext4Backend(io);
  if (!backend.initialize())
  {
    fprintf(2, "ext4: read-only mount rejected, error=%u\n", unsigned(backend.error()));
    return 1;
  }
  return userfs::plugin::run(invocation, backend) < 0;
}
