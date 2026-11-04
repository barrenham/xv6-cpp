module;
#include "user/runtime/user.h"
module xv.userfs.plugin;

namespace userfs::plugin
{
Invocation::Invocation(int argc, char **argv)
{
  if ((argc != 5 && argc != 6) || strcmp(argv[2], "1")) return;
  probe = !strcmp(argv[1], "--fs-probe");
  if ((probe && argc != 5) || (!probe && (argc != 6 || strcmp(argv[1], "--fs-plugin")))) return;
  unsigned length = strlen(argv[3]);
  if (!length || length >= 32) return;
  for (unsigned i = 0; i < length; ++i)
  {
    char c = argv[3][i];
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) return;
  }
  if (!probe)
  {
    mode = 0;
    if (!argv[4][0]) return;
    for (unsigned i = 0; argv[4][i]; ++i)
    {
      if (i >= 5 || argv[4][i] < '0' || argv[4][i] > '7') return;
      mode = mode * 8 + argv[4][i] - '0';
    }
    if (mode > 07777) return;
  }
  type = argv[3];
  options = argv[probe ? 4 : 5];
  close(Image);
  valid = true;
}

int run(const Invocation &invocation, Backend &backend)
{
  if (!invocation.valid || invocation.probe) return -1;
  int endpoint = register_backend(invocation.type, backend);
  if (endpoint < 0) return -1;
  if (write(Ready, "R", 1) != 1) { close(endpoint); return -1; }
  close(Ready);
  char approved;
  int received = read(Activation, &approved, 1);
  close(Activation);
  if (received != 1 || approved != 'G') { close(endpoint); return -1; }
  int result = serve(endpoint, backend);
  close(endpoint);
  return result;
}
}
