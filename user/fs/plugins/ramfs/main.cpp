#include "user/runtime/user.h"
import xv.userfs.plugin;
import xv.userfs.ramfs;

extern "C" int main(int argc, char **argv)
{
  userfs::plugin::Invocation invocation(argc, argv);
  if (!invocation.valid) exit(2);
  if (invocation.probe) exit(1);
  unsigned workers = 0;
  if (invocation.options && *invocation.options)
  {
    if (!strcmp(invocation.options, "read-workers=1"))
    {
      workers = 1;
    }
    else if (!strcmp(invocation.options, "read-workers=2"))
    {
      workers = 2;
    }
    else if (strcmp(invocation.options, "read-workers=0"))
    {
      exit(2);
    }
  }
  userfs::MemoryBackend backend(userfs::heap_storage(), invocation.mode, workers);
  if (!backend.ready()) exit(1);
  exit(userfs::plugin::run(invocation, backend) < 0);
}
