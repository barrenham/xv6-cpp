#include "protocol.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/process/wait/wait.h"

namespace services
{
int
launch_daemon() {
  int channel[2];
  if (pipe(channel) < 0)
  {
    return -1;
  }
  int pid = fork();
  if (!pid)
  {
    close(channel[0]);
    for (int fd = 0; fd < 16; ++fd)
    {
      if (fd != channel[1])
      {
        close(fd);
      }
    }
    if (open("/dev/zero", O_RDONLY) != 0 || open_manager_log() != 1 || dup(1) != 2)
    {
      exit(1);
    }
    char descriptor[16], reversed[16];
    unsigned value = channel[1], count = 0, used = 0;
    do
    {
      reversed[count++] = '0' + value % 10;
      value /= 10;
    } while (value);
    while (count)
    {
      descriptor[used++] = reversed[--count];
    }
    descriptor[used] = 0;
    char *arguments[] = {const_cast<char *>("serviced"),
                         const_cast<char *>("--ready-fd"), descriptor, nullptr};
    exec("/sbin/serviced", arguments);
    exit(127);
  }
  close(channel[1]);
  if (pid < 0)
  {
    close(channel[0]);
    return -1;
  }
  pollfd ready{channel[0], POLLIN, 0};
  char value = 0;
  bool good =
      poll(&ready, 1, 5000) > 0 && read(channel[0], &value, 1) == 1 && value == 'R';
  close(channel[0]);
  if (!good)
  {
    kill(pid);
    waitpid(pid, nullptr, 0);
    return -1;
  }
  return pid;
}
} // namespace services
