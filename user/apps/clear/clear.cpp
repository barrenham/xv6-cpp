#include "kernel/support/types.h"
#include "user/runtime/user.h"
extern "C" int main(int argc, char **argv)
{
  bool scrollback = true;
  for (int i=1;i<argc;++i)
  {
    if (!strcmp(argv[i],"-x")) scrollback=false;
    else if (!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h"))
    { printf("usage: clear [-x]\n  -x  preserve scrollback\n");exit(0); }
    else { fprintf(2,"usage: clear [-x]\n");exit(2); }
  }
  const char *sequence=scrollback ? "\033[H\033[2J\033[3J" : "\033[H\033[2J";
  int length=int(strlen(sequence)), done=0;
  while(done<length) { int n=write(1,sequence+done,length-done);if(n<=0)exit(1);done+=n; }
  exit(0);
}
