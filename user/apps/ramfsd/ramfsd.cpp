#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/userfs/memory.h"

extern "C" int main(int argc, char **argv)
{
  bool registered = argc == 3 && !strcmp(argv[1], "--register");
  bool mounting = argc == 4 && !strcmp(argv[1], "--mount");
  if (!registered && !mounting)
  {
    fprintf(2, "usage: ramfsd --register type | ramfsd --mount type /mountpoint\n");
    exit(1);
  }
  userfs::MemoryBackend backend(userfs::heap_storage());
  int endpoint = backend.ready() ? userfs::register_backend(argv[2], backend) : -1;
  if (endpoint < 0 || (mounting && mount(argv[2], argv[3]) < 0))
  {
    if (endpoint >= 0) close(endpoint);
    fprintf(2, "ramfsd: registration or mount failed\n");
    exit(1);
  }
  printf("ramfsd: %s %s pid=%d\n", mounting ? "mounted" : "registered", argv[2], getpid());
  int result = userfs::serve(endpoint, backend);
  close(endpoint);
  exit(result < 0);
}
