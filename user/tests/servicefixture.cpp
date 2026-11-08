#include "user/runtime/user.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/fcntl.h"

namespace
{
volatile sig_atomic_t bursts = 1;

void
more_output(int) {
  bursts = bursts + 1;
}
} // namespace

extern "C" int
main(int argc, char **argv) {
  const char *mode = argc > 1 ? argv[1] : "hold";
  if (!strcmp(mode, "crash"))
  {
    printf("fixture: crash\n");
    exit(42);
  }
  const char *notification = getenv("SERVICE_READY_FD");
  if (!strcmp(mode, "logger"))
  {
    signal(SIGUSR1, more_output);
  }
  if (notification)
  {
    int fd = atoi(notification);
    if (strcmp(mode, "bad-ready"))
    {
      if (write(fd, "R", 1) != 1)
      {
        exit(2);
      }
    }
    close(fd);
  }
  if (!strcmp(mode, "bad-ready"))
  {
    exit(0);
  }
  printf("fixture: ready pid=%d\n", getpid());
  unsigned sequence = 0;
  unsigned completed = 0;
  for (;;)
  {
    if (!strcmp(mode, "logger") && bursts)
    {
      bursts = bursts - 1;
      for (unsigned i = 0; i < 32; ++i)
      {
        char block[256];
        memset(block, 'A' + sequence++ % 26, sizeof(block));
        block[sizeof(block) - 1] = '\n';
        if (write(i & 1 ? 2 : 1, block, sizeof(block)) != sizeof(block))
        {
          exit(3);
        }
      }
      if (argc > 2)
      {
        int fd = open(argv[2], O_CREATE | O_TRUNC | O_WRONLY);
        ++completed;
        if (fd < 0 || write(fd, &completed, sizeof(completed)) != sizeof(completed))
        {
          exit(4);
        }
        close(fd);
      }
    }
    poll(nullptr, 0, 1000);
  }
}
