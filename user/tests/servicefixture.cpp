#include "user/runtime/user.h"
#include "kernel/fs/poll.h"

extern "C" int
main(int argc, char **argv) {
  const char *mode = argc > 1 ? argv[1] : "hold";
  if (!strcmp(mode, "crash"))
  {
    printf("fixture: crash\n");
    exit(42);
  }
  const char *notification = getenv("SERVICE_READY_FD");
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
  for (;;)
  {
    poll(nullptr, 0, 1000);
  }
}
