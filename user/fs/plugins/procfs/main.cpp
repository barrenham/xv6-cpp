#include "user/runtime/user.h"
import xv.userfs.plugin;
import xv.userfs.procfs;

extern "C" int main(int argc, char **argv)
{
  userfs::plugin::Invocation invocation(argc, argv);
  if (!invocation.valid) exit(2);
  if (invocation.probe) exit(1);
  userfs::ProcessBackend backend(userfs::plugin::Resources);
  close(userfs::plugin::Resources);
  if (!backend.ready()) exit(1);
  exit(userfs::plugin::run(invocation, backend) < 0);
}
