#include "application.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "kernel/fs/userfs/protocol.h"
import xv.userfs;

#include "archive.h"

extern "C" int
app_main(int argc, char **argv) {
  if (argc != 3)
  {
    fprintf(2, "usage: tarfsd archive.tar registration-type\n");
    exit(1);
  }
  int image = open(argv[1], O_RDONLY);
  if (image < 0 || !tarfs::load(image))
  {
    fprintf(2, "tarfsd: invalid or unsupported ustar archive\n");
    exit(1);
  }
  int result = tarfs::serve(image, argv[2]);
  close(image);
  return result;
}
