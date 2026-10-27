#include "kernel/fs/image_paths.h"
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/fcntl.h"
#include "kernel/process/signals/signal.h"
#include "kernel/process/wait/wait.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"

namespace
{
void check(bool ok, const char *message)
{
  if (!ok)
  {
    fprintf(2, "polltest: FAIL: %s\n", message);
    exit(1);
  }
}
short query(int fd, short events)
{
  pollfd descriptor{fd, events, -1};
  int result = poll(&descriptor, 1, 0);
  check(result == (descriptor.revents ? 1 : 0), "ready count");
  return descriptor.revents;
}
void pair(int *fds) { check(openpty(fds) == 0, "PTY pair"); }
void raw(int fd)
{
  termios settings;
  check(tcgetattr(fd, &settings) == 0, "get settings");
  cfmakeraw(&settings);
  check(tcsetattr(fd, TCSANOW, &settings) == 0, "raw settings");
}
void reap(int child)
{
  int status;
  check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
        WEXITSTATUS(status) == 0, "child exit");
}
void abi()
{
  check(poll(nullptr, 0, 0) == 0 && poll(nullptr, -1, 0) == -1 &&
        poll(nullptr, NOFILE + 1, 0) == -1 &&
        poll(reinterpret_cast<pollfd *>(~0UL), 1, 0) == -1, "argument validation");
  pollfd invalid[] = {{-1, POLLIN, -1}, {NOFILE, 0, 0}};
  check(poll(invalid, 2, -1) == 1 && invalid[0].revents == 0 &&
        invalid[1].revents == POLLNVAL, "ignored and invalid descriptors");
  int regular = open(XV_README_PATH, O_RDONLY);
  check(regular >= 0 && query(regular, POLLIN | POLLOUT) == POLLIN,
        "regular file and access mask");
  close(regular);
  unsigned before = uptime();
  check(poll(nullptr, 0, 150) == 0 && uptime() - before >= 1 && uptime() - before < 10,
        "finite timeout without descriptors");
}
void pipes()
{
  int fds[2]; char bytes[512]; memset(bytes, 'p', sizeof(bytes));
  check(pipe(fds) == 0, "pipe");
  check(query(fds[0], POLLIN) == 0 && query(fds[1], POLLOUT) == POLLOUT,
        "empty pipe readiness");
  check(write(fds[1], bytes, sizeof(bytes)) == sizeof(bytes) &&
        query(fds[1], POLLOUT) == 0 && query(fds[0], POLLIN) == POLLIN,
        "full pipe readiness");
  pollfd duplicate[] = {{fds[0], POLLIN, 0}, {fds[0], POLLIN, 0}};
  check(poll(duplicate, 2, 0) == 2, "duplicate descriptors count separately");
  check(read(fds[0], bytes, 1) == 1 && query(fds[1], POLLOUT) == POLLOUT,
        "read releases pipe space");
  close(fds[1]);
  check(query(fds[0], POLLIN) == (POLLIN | POLLHUP) &&
        read(fds[0], bytes, sizeof(bytes)) == 511 &&
        query(fds[0], 0) == POLLHUP && read(fds[0], bytes, 1) == 0,
        "pipe drains before EOF, unrequested HUP");
  close(fds[0]);
  check(query(fds[0], 0) == POLLNVAL, "closed descriptor");
  check(pipe(fds) == 0, "broken pipe"); close(fds[0]);
  check(query(fds[1], 0) == POLLERR, "unrequested broken-pipe error");
  close(fds[1]);
}
void terminals()
{
  int fds[2]; pair(fds);
  termios settings;
  check(tcgetattr(fds[1], &settings) == 0, "canonical settings");
  settings.c_lflag &= ~ECHO;
  check(tcsetattr(fds[1], TCSANOW, &settings) == 0, "disable echo");
  char bytes[128]{};
  check(query(fds[0], POLLOUT) == POLLOUT && query(fds[1], POLLIN) == 0,
        "initial PTY readiness");
  check(write(fds[0], "a", 1) == 1 && query(fds[1], POLLIN) == 0 &&
        write(fds[0], "\n", 1) == 1 && query(fds[1], POLLIN) == POLLIN &&
        read(fds[1], bytes, sizeof(bytes)) == 2, "canonical record readiness");
  check(write(fds[0], "\004", 1) == 1 && query(fds[1], POLLIN) == POLLIN &&
        read(fds[1], bytes, 1) == 0 && query(fds[1], POLLIN) == 0,
        "canonical EOF slot is readable");
  raw(fds[1]);
  check(tcgetattr(fds[1], &settings) == 0, "raw get");
  settings.c_cc[VMIN] = 2;
  check(tcsetattr(fds[1], TCSANOW, &settings) == 0 && write(fds[0], "a", 1) == 1 &&
        query(fds[1], POLLIN) == 0 && write(fds[0], "b", 1) == 1 &&
        query(fds[1], POLLIN) == POLLIN && read(fds[1], bytes, 2) == 2,
        "noncanonical minimum readiness");
  raw(fds[1]);
  check(write(fds[0], bytes, sizeof(bytes)) == sizeof(bytes) &&
        query(fds[0], POLLOUT) == 0 && read(fds[1], bytes, 1) == 1 &&
        query(fds[0], POLLOUT) == POLLOUT, "master input backpressure");
  check(read(fds[1], bytes, sizeof(bytes)) == 127, "drain input");
  for (int i = 0; i < 8; ++i)
    check(write(fds[1], bytes, sizeof(bytes)) == sizeof(bytes), "fill output");
  check(query(fds[1], POLLOUT) == 0 && query(fds[0], POLLIN) == POLLIN &&
        read(fds[0], bytes, 2) == 2 && query(fds[1], POLLOUT) == POLLOUT,
        "slave output backpressure");
  close(fds[1]);
  check(query(fds[0], POLLIN) == (POLLIN | POLLHUP), "master drain and HUP");
  close(fds[0]);
  pair(fds); close(fds[0]);
  check(query(fds[1], POLLIN | POLLOUT) == (POLLIN | POLLHUP), "slave hangup");
  close(fds[1]);
}
void races()
{
  // Alternating pipe notifications repeatedly cover readiness before registration
  // and notifications between registration and sleep on SMP.
  int request[2], reply[2];
  check(pipe(request) == 0 && pipe(reply) == 0, "race pipes");
  int child = fork(); check(child >= 0, "race fork");
  if (child == 0)
  {
    close(request[1]); close(reply[0]);
    for (int i = 0; i < 128; ++i)
    {
      pollfd descriptor{request[0], POLLIN, 0}; char byte;
      check(poll(&descriptor, 1, -1) == 1 && (descriptor.revents & POLLIN) &&
            read(request[0], &byte, 1) == 1 && write(reply[1], &byte, 1) == 1,
            "child infinite poll wake");
    }
    exit(0);
  }
  close(request[0]); close(reply[1]);
  for (int i = 0; i < 128; ++i)
  {
    char byte = i;
    check(write(request[1], &byte, 1) == 1, "request write");
    pollfd descriptor{reply[0], POLLIN, 0};
    check(poll(&descriptor, 1, 1000) == 1 && (descriptor.revents & POLLIN) &&
          read(reply[0], &byte, 1) == 1 && static_cast<unsigned char>(byte) == i,
          "parent poll wake and data");
  }
  close(request[1]); close(reply[0]); reap(child);

  int ready[2], data[2];
  check(pipe(ready) == 0 && pipe(data) == 0, "multiwait pipes");
  int children[2];
  for (int i = 0; i < 2; ++i)
  {
    children[i] = fork(); check(children[i] >= 0, "multiwait fork");
    if (children[i] == 0)
    {
      close(ready[0]); close(data[1]);
      check(write(ready[1], "r", 1) == 1, "multiwait ready");
      pollfd descriptor{data[0], POLLIN, 0};
      check(poll(&descriptor, 1, -1) == 1 && (descriptor.revents & POLLIN),
            "all poll waiters notified");
      exit(0);
    }
  }
  close(ready[1]); close(data[0]); char byte;
  check(read(ready[0], &byte, 1) == 1 && read(ready[0], &byte, 1) == 1 &&
        write(data[1], "x", 1) == 1, "multiwait trigger");
  reap(children[0]); reap(children[1]); close(ready[0]); close(data[1]);

  check(pipe(ready) == 0, "signal ready pipe");
  child = fork(); check(child >= 0, "signal fork");
  if (child == 0)
  {
    close(ready[0]); check(write(ready[1], "r", 1) == 1, "signal ready");
    poll(nullptr, 0, -1);
    exit(1);
  }
  close(ready[1]); check(read(ready[0], &byte, 1) == 1, "signal wait ready");
  pause(1); // Give the child one timer interval to enter its infinite wait.
  check(sigsend(child, SIGTERM) == 0, "interrupt poll");
  int status;
  check(waitpid(child, &status, 0) == child && WIFSIGNALED(status) &&
        WTERMSIG(status) == SIGTERM, "signal wakes infinite poll");
  close(ready[0]);
}
}

extern "C" void run_poll_checks()
{
  abi(); pipes(); terminals(); races();
  printf("ptytest: poll tests succeeded\n");
}
