#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/ext3/format.h"
import xv.userfs.plugin;
import xv.userfs.ext3;
import xv.userfs.block;

extern "C" int main(int argc, char **argv)
{
  userfs::plugin::Invocation invocation(argc, argv);
  if (!invocation.valid) exit(2);
  int resources = fcntl(userfs::plugin::Resources, F_GETFL, 0) >= 0 ? userfs::plugin::Resources : -1;
  userfs::BlockIo io(userfs::plugin::Source, resources);
  if (invocation.probe)
  {
    unsigned char signature[128];
    if (!io.read(1024, signature, sizeof(signature))) exit(1);
    exit(xv::ext3::le16(signature + 56) == xv::ext3::Magic &&
         !(xv::ext3::le32(signature + 96) & 0x40) ? 0 : 1);
  }
  void *storage = malloc(sizeof(userfs::Ext3Backend));
  if (!storage) exit(1);
  auto &backend = *new (storage) userfs::Ext3Backend(io);
  if (!backend.initialize()) exit(1);
  exit(userfs::plugin::run(invocation, backend) < 0);
}
