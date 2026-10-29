// Bounded fork/reap stress. Dynamic storage can legitimately reach N children;
// allocation failure before N must still leave every successful child waitable.

#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"

extern "C"
{

#define N 1000

  void print(const char *s)
  {
    write(1, s, strlen(s));
  }

  void forktest(void)
  {
    int n, pid;

    print("fork test\n");
    const uint64 start = clockticks();

    for (n = 0; n < N; n++)
    {
      pid = fork();
      if (pid < 0)
      {
        break;
      }
      if (pid == 0)
      {
        exit(0);
      }
    }

    const int children = n;
    if (!children) { print("no fork at all!\n"); exit(1); }

    for (; n > 0; n--)
    {
      if (wait(0) < 0)
      {
        print("wait stopped early\n");
        exit(1);
      }
    }

    if (wait(0) != -1)
    {
      print("wait got too many\n");
      exit(1);
    }

    const uint64 elapsed = clockticks() - start;
    printf("fork perf: children=%d ticks=%lu\n", children, elapsed);
    print("fork test OK\n");
  }

  int main(void)
  {
    forktest();
    exit(0);
  }
}
