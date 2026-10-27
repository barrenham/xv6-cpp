#include "kernel/support/types.h"
#include "user/runtime/user.h"
extern "C" int main(int argc, char **argv)
{
  int first = 1;
  if (first < argc && !strcmp(argv[first], "-i")) { clearenv(); ++first; }
  while (first < argc && strchr(argv[first], '='))
  { char *equal = strchr(argv[first], '='); *equal = 0; if (setenv(argv[first], equal + 1, 1) < 0) exit(1); ++first; }
  if (first < argc) { execvp(argv[first], argv + first); fprintf(2, "env: exec failed\n"); exit(1); }
  for (char **entry = environ; entry && *entry; ++entry) printf("%s\n", *entry);
  exit(0);
}
