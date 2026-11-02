#include "kernel/support/types.h"
#include "user/runtime/user.h"

// Loaded directly from -initrd. No path, disk image, /dev or /proc is needed.
extern "C" int main(int, char **)
{
  printf("fsboottest: entered userspace pid=%d\n", getpid());
  if (getpid() != 1)
  {
    printf("fsboottest: FAIL: initial process identity\n");
    for (;;) pause(1);
  }
  int child = fork();
  if (child == 0)
  {
    printf("fsboottest: child console before root mount\n");
    exit(23);
  }
  int status = -1;
  if (child < 0 || wait(&status) != child || status != 23)
  {
    printf("fsboottest: FAIL: pre-root fork/wait/exit\n");
  }
  else
  {
    printf("fsboottest: PASS: userspace bootstrap without a mounted filesystem\n");
  }
  for (;;) pause(1);
}
