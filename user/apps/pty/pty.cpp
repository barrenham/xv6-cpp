#include "kernel/support/types.h"
#include "kernel/process/wait/wait.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/io_status.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"

static void transfer(int master)
{
  // These flags belong to open file descriptions and may also be used by the
  // waiting outer shell. Capture both before changing either alias.
  int input_flags = fcntl(0, F_GETFL);
  int output_flags = fcntl(1, F_GETFL);
  if (input_flags < 0 || output_flags < 0) return;
  bool configured = fcntl(0, F_SETFL, input_flags | O_NONBLOCK) == 0 &&
                    fcntl(1, F_SETFL, output_flags | O_NONBLOCK) == 0 &&
                    fcntl(master, F_SETFL, O_NONBLOCK) == 0;
  if (configured)
  {
    char output[256];
    int used = 0, sent = 0;
    unsigned char input[256];
    int input_used = 0, input_sent = 0;
    bool hungup = false;
    for (;;)
    {
      short master_events = 0;
      if (sent == used) master_events |= POLLIN;
      if (input_sent < input_used && !hungup) master_events |= POLLOUT;
      pollfd descriptors[] = {
        {input_sent < input_used || hungup ? -1 : 0, POLLIN, 0},
        {master_events ? master : -1, master_events, 0},
        {sent < used ? 1 : -1, POLLOUT, 0},
      };
      if (poll(descriptors, 3, -1) < 0) break;
      bool failed = false;
      for (auto &descriptor : descriptors)
        if (descriptor.revents & (POLLERR | POLLNVAL)) failed = true;
      if (failed || (descriptors[2].revents & POLLHUP)) break;
      if (descriptors[1].revents & POLLHUP)
      {
        hungup = true;
        input_sent = input_used;
      }
      if (sent == used && (descriptors[1].revents & (POLLIN | POLLHUP)))
      {
        int n = read(master, output, sizeof(output));
        if (n != -EAGAIN)
        {
          if (n <= 0) break;
          used = n;
          sent = 0;
        }
      }
      if (sent < used && (descriptors[2].revents & POLLOUT))
      {
        int n = write(1, output + sent, used - sent);
        if (n > 0) sent += n;
        else if (n != -EAGAIN) break;
      }
      if (descriptors[0].revents & (POLLIN | POLLHUP))
      {
        int n = read(0, input, sizeof(input));
        if (n != -EAGAIN)
        {
          if (n <= 0) break;
          bool escape = false;
          for (int i = 0; i < n; ++i) if (input[i] == 29) escape = true;
          if (escape) break;
          input_used = n;
          input_sent = 0;
        }
      }
      if (input_sent < input_used && !hungup && (descriptors[1].revents & POLLOUT))
      {
        int n = write(master, input + input_sent, input_used - input_sent);
        if (n > 0) input_sent += n;
        else if (n != -EAGAIN) break;
      }
    }
  }
  fcntl(0, F_SETFL, input_flags);
  fcntl(1, F_SETFL, output_flags);
}

// One proxy process multiplexes keyboard, master and console output with poll().
extern "C" int main(int argc, char **argv)
{
  int fds[2];
  termios saved;
  char slave_path[32];
  fds[0] = posix_openpt(O_RDWR);
  bool opened = fds[0] >= 0 && grantpt(fds[0]) == 0 && unlockpt(fds[0]) == 0 &&
                ptsname_r(fds[0], slave_path, sizeof(slave_path)) == 0;
  fds[1] = opened ? open(slave_path, O_RDWR) : -1;
  if (tcgetattr(0, &saved) < 0 || tcgetpgrp(0) != getpgrp() || fds[1] < 0)
  {
    fprintf(2, "pty: foreground terminal required or allocation failed\n");
    exit(1);
  }
  winsize size;
  if (ioctl(0, TIOCGWINSZ, reinterpret_cast<unsigned long>(&size)) == 0)
    ioctl(fds[1], TIOCSWINSZ, reinterpret_cast<unsigned long>(&size));
  int child = fork();
  if (child < 0)
  {
    close(fds[0]); close(fds[1]);
    exit(1);
  }
  if (child == 0)
  {
    close(fds[0]);
    if (setsid() < 0 || ioctl(fds[1], TIOCSCTTY, 0) < 0) exit(1);
    close(0); close(1); close(2);
    if (dup(fds[1]) != 0 || dup(fds[1]) != 1 || dup(fds[1]) != 2) exit(1);
    close(fds[1]);
    char *shell[] = {const_cast<char *>("sh"), nullptr};
    execvp(argc > 1 ? argv[1] : "sh", argc > 1 ? argv + 1 : shell);
    fprintf(2, "pty: exec failed\n");
    exit(1);
  }
  close(fds[1]);
  termios raw = saved;
  cfmakeraw(&raw);
  if (tcsetattr(0, TCSANOW, &raw) < 0)
  {
    close(fds[0]);
    kill(child);
    wait(nullptr);
    exit(1);
  }
  printf("pty: connected; Ctrl-] exits\n");
  transfer(fds[0]);
  close(fds[0]);
  tcsetattr(0, TCSANOW, &saved);
  waitpid(child, nullptr, 0);
  printf("pty: disconnected\n");
  exit(0);
}
