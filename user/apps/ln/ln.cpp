#include "kernel/support/types.h"
#include "user/runtime/user.h"
extern "C" int main(int argc, char **argv)
{
 bool symbolic = argc == 4 && !strcmp(argv[1], "-s");
 if ((!symbolic && argc != 3) || (symbolic ? symlink(argv[2], argv[3]) : link(argv[1], argv[2])) < 0)
 { fprintf(2, "ln: usage: ln [-s] target name (or link failed)\n"); exit(1); }
 exit(0);
}
