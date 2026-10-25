#include "kernel/support/types.h"
#include "kernel/process/wait/wait.h"
#include "kernel/fs/poll.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"

// One proxy process multiplexes keyboard, master and console output with poll().
extern "C" int main(int argc, char **argv)
{
  int fds[2];
  termios saved;
  if (tcgetattr(0, &saved) < 0 || openpty(fds) < 0)
  {
    fprintf(2, "pty: terminal or allocation failed\n");
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
    exec(argc > 1 ? argv[1] : "sh", argc > 1 ? argv + 1 : shell);
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
  char output[256];
  int used = 0, sent = 0;
  unsigned char input = 0;
  bool pending_input = false;
  bool hungup = false;
  for (;;)
  {
    short master_events = 0;
    if (sent == used) master_events |= POLLIN;
    if (pending_input && !hungup) master_events |= POLLOUT;
    pollfd descriptors[] = {
      {pending_input || hungup ? -1 : 0, POLLIN, 0},
      {master_events ? fds[0] : -1, master_events, 0},
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
      pending_input = false;
    }
    if (sent == used && (descriptors[1].revents & (POLLIN | POLLHUP)))
    {
      used = read(fds[0], output, sizeof(output));
      sent = 0;
      if (used <= 0) break;
    }
    // Send one byte per writable snapshot: large writes may wait for space even
    // after poll reports POLLOUT, preventing us from servicing the other side.
    if (sent < used && (descriptors[2].revents & POLLOUT))
    {
      if (write(1, output + sent, 1) != 1) break;
      ++sent;
    }
    if (descriptors[0].revents & (POLLIN | POLLHUP))
    {
      if (read(0, &input, 1) != 1 || input == 29) break;
      pending_input = true;
    }
    if (pending_input && !hungup && (descriptors[1].revents & POLLOUT))
    {
      if (write(fds[0], &input, 1) != 1) break;
      pending_input = false;
    }
  }
  close(fds[0]);
  tcsetattr(0, TCSANOW, &saved);
  waitpid(child, nullptr, 0);
  printf("pty: disconnected\n");
  exit(0);
}
