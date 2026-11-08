#include "user/runtime/ui.h"
#include "application.h"
#include "backend.h"
#include "user/runtime/user.h"
#include "kernel/fs/userfs/protocol.h"

extern "C" int
app_main(int argc, char **argv) {
  bool registered = (argc == 3 || argc == 4) && !strcmp(argv[1], "--register");
  if (!registered && argc != 2)
  {
    ui::usage("usage: memfsd /mountpoint | memfsd --register type [max-payload]\n");
    return 1;
  }
  unsigned maximum =
      registered && argc == 4 ? unsigned(atoi(argv[3])) : xv::userfs::DataSize;
  return memfs::serve(registered ? argv[2] : argv[1], registered, maximum);
}
