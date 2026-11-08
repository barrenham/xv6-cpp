#include "user/runtime/user.h"
import xv.userfs.plugin;
import xv.userfs.ramfs;

extern "C" int
main(int argc, char **argv) {
  userfs::plugin::Invocation invocation(argc, argv);
  if (!invocation.valid)
  {
    return 2;
  }
  if (invocation.probe)
  {
    return 1;
  }
  if (invocation.options && *invocation.options &&
      strcmp(invocation.options, "workers=1") &&
      strcmp(invocation.options, "workers=2"))
  {
    return 2;
  }
  userfs::MemoryBackend backend(userfs::heap_storage(), invocation.mode);
  return !backend.ready() || userfs::plugin::run(invocation, backend) < 0;
}
