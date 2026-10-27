#include "kernel/support/types.h"
#include "user/runtime/user.h"
extern "C" int main(int argc, char **argv)
{
  if (argc < 3 || !*argv[1]) { fprintf(2, "usage: chmod OCTAL FILE...\n"); exit(1); }
  unsigned mode = 0;
  for (const char *p = argv[1]; *p; ++p)
  { if (*p < '0' || *p > '7' || mode > 01777 / 8) exit(1); mode = mode * 8 + *p - '0'; }
  int failed = 0;
  for (int i = 2; i < argc; ++i) if (chmod(argv[i], mode) < 0) { fprintf(2, "chmod: %s failed\n", argv[i]); failed = 1; }
  exit(failed);
}
