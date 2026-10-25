#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/process/signals/signal.h"
#include "kernel/process/wait/wait.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"

static void check(bool ok, const char *message)
{
  if (!ok)
  {
    fprintf(2, "ptytest: FAIL: %s\n", message);
    exit(1);
  }
}
static void pair(int *fds) { check(openpty(fds) == 0, "openpty"); }
static void close_pair(int *fds) { close(fds[0]); close(fds[1]); }
static void raw(int fd)
{
  termios settings;
  check(tcgetattr(fd, &settings) == 0, "get termios");
  cfmakeraw(&settings);
  check(tcsetattr(fd, TCSANOW, &settings) == 0, "raw termios");
}
static void send(int fd) { check(write(fd, "!", 1) == 1, "sync send"); }
static void receive(int fd) { char c; check(read(fd, &c, 1) == 1, "sync read"); }
static void reap(int child)
{
  int status;
  check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
        WEXITSTATUS(status) == 0, "child result");
}
static void basic()
{
  int fds[2], other[2];
  pair(fds); pair(other);
  termios settings, independent;
  check(tcgetattr(fds[1], &settings) == 0, "slave termios");
  settings.c_lflag &= ~ECHO;
  check(tcsetattr(fds[1], TCSANOW, &settings) == 0 &&
        tcgetattr(other[1], &independent) == 0 && (independent.c_lflag & ECHO),
        "separate PTY settings");
  char bytes[32];
  check(write(fds[0], "ab\177c\r", 5) == 5 && read(fds[1], bytes, sizeof(bytes)) == 3 &&
        memcmp(bytes, "ac\n", 3) == 0, "canonical erase and CR conversion");
  settings.c_oflag |= ONLCR;
  check(tcsetattr(fds[0], TCSANOW, &settings) == 0, "master settings forwarding");
  check(write(fds[1], "ok\n", 3) == 3 && read(fds[0], bytes, sizeof(bytes)) == 4 &&
        memcmp(bytes, "ok\r\n", 4) == 0, "output conversion");
  check(write(fds[0], "pending\n", 8) == 8 && tcflush(fds[1], TCIFLUSH) == 0,
        "input flush");
  int count = -1;
  check(ioctl(fds[1], FIONREAD, reinterpret_cast<unsigned long>(&count)) == 0 && count == 0,
        "flushed input count");
  raw(fds[1]);
  const unsigned char binary[] = {0, 3, 26, 255, '\r', '\n'};
  check(write(fds[0], binary, sizeof(binary)) == sizeof(binary) &&
        read(fds[1], bytes, sizeof(bytes)) == sizeof(binary) &&
        memcmp(bytes, binary, sizeof(binary)) == 0, "raw binary input");
  check(write(fds[1], binary, sizeof(binary)) == sizeof(binary), "binary output");
  check(ioctl(fds[0], FIONREAD, reinterpret_cast<unsigned long>(&count)) == 0 &&
        count == sizeof(binary), "master output count");
  check(read(fds[0], reinterpret_cast<void *>(~0UL), 1) == -1 &&
        read(fds[0], bytes, sizeof(bytes)) == sizeof(binary) &&
        memcmp(bytes, binary, sizeof(binary)) == 0, "bad master copyout preserves byte");
  check(write(fds[0], "x", 1) == 1 &&
        read(fds[1], reinterpret_cast<void *>(~0UL), 1) == -1 &&
        read(fds[1], bytes, 1) == 1 && bytes[0] == 'x', "bad slave copyout preserves byte");
  check(write(fds[0], reinterpret_cast<void *>(~0UL), 1) == -1 &&
        write(fds[1], reinterpret_cast<void *>(~0UL), 1) == -1,
        "invalid write addresses");
  check(read(fds[0], bytes, 0) == 0 && write(fds[1], bytes, 0) == 0 &&
        read(fds[1], bytes, -1) == -1, "zero and negative lengths");
  struct stat st;
  check(fstat(fds[0], &st) == 0 && st.type == T_DEVICE, "endpoint stat");
  winsize size{33, 99, 0, 0}, actual{};
  check(ioctl(fds[0], TIOCSWINSZ, reinterpret_cast<unsigned long>(&size)) == 0 &&
        ioctl(fds[1], TIOCGWINSZ, reinterpret_cast<unsigned long>(&actual)) == 0 &&
        actual.ws_row == 33 && actual.ws_col == 99, "shared window size");
  check(tcgetattr(fds[1], &settings) == 0, "timed settings");
  settings.c_cc[VMIN] = 0; settings.c_cc[VTIME] = 1;
  check(tcsetattr(fds[1], TCSANOW, &settings) == 0 && read(fds[1], bytes, 1) == 0,
        "PTY timer expiration");
  close_pair(fds); close_pair(other);
  printf("ptytest: basic OK\n");
}
static void flow(bool output)
{
  int fds[2]; pair(fds); raw(fds[1]);
  int child = fork(); check(child >= 0, "flow fork");
  if (child == 0)
  {
    close(fds[output ? 1 : 0]);
    char buffer[37]; int done = 0;
    while (done < 4096)
    {
      int n = read(fds[output ? 0 : 1], buffer, sizeof(buffer));
      check(n > 0, "flow read");
      for (int i = 0; i < n; ++i)
        check(static_cast<unsigned char>(buffer[i]) == static_cast<unsigned char>(done + i),
              "flow content");
      done += n;
    }
    exit(0);
  }
  close(fds[output ? 0 : 1]);
  static char buffer[4096];
  for (int i = 0; i < 4096; ++i) buffer[i] = i;
  check(write(fds[output ? 1 : 0], buffer, sizeof(buffer)) == sizeof(buffer),
        "flow write with backpressure");
  close(fds[output ? 1 : 0]);
  reap(child);
}
static void closure()
{
  int fds[2]; char bytes[16]; pair(fds); raw(fds[1]);
  int alias = dup(fds[1]);
  check(alias >= 0, "dup slave");
  close(fds[1]);
  check(write(alias, "tail", 4) == 4, "last slave reference retained");
  close(alias);
  check(read(fds[0], bytes, sizeof(bytes)) == 4 && memcmp(bytes, "tail", 4) == 0 &&
        read(fds[0], bytes, 1) == 0 && write(fds[0], "x", 1) == -1,
        "slave close drains output then EOF");
  close(fds[0]);
  pair(fds);
  termios settings; tcgetattr(fds[1], &settings); settings.c_lflag &= ~ECHO;
  tcsetattr(fds[1], TCSANOW, &settings);
  check(write(fds[0], "tail", 4) == 4, "unfinished canonical input");
  close(fds[0]);
  check(read(fds[1], bytes, sizeof(bytes)) == 4 && memcmp(bytes, "tail", 4) == 0 &&
        read(fds[1], bytes, 1) == 0 && write(fds[1], "x", 1) == -1 &&
        ioctl(fds[1], TIOCSCTTY, 0) == -1, "master hangup drains input then EOF");
  close(fds[1]);
  // Closing master must wake a blocked slave read and blocked output write.
  for (int writer = 0; writer != 2; ++writer)
  {
    int ready[2]; pair(fds); raw(fds[1]); check(pipe(ready) == 0, "closure pipe");
    int child = fork(); check(child >= 0, "closure fork");
    if (child == 0)
    {
      close(fds[0]); close(ready[0]);
      if (writer)
      {
        char full[1024]; memset(full, 'x', sizeof(full));
        check(write(fds[1], full, sizeof(full)) == sizeof(full), "fill output queue");
      }
      send(ready[1]);
      int result = writer ? write(fds[1], "x", 1) : read(fds[1], bytes, 1);
      check(result == (writer ? -1 : 0), "hangup wakes blocked operation");
      exit(0);
    }
    close(fds[1]); close(ready[1]); receive(ready[0]);
    close(fds[0]); close(ready[0]); reap(child);
  }
  printf("ptytest: flow and closure OK\n");
}
static void signals(bool hangup)
{
  int fds[2], ready[2]; pair(fds); check(pipe(ready) == 0, "signal pipe");
  int child = fork(); check(child >= 0, "signal fork");
  if (child == 0)
  {
    close(fds[0]); close(ready[0]);
    check(setsid() == getpid() && ioctl(fds[1], TIOCSCTTY, 0) == 0,
          "slave controlling terminal");
    send(ready[1]);
    for (;;) pause(1000);
  }
  close(fds[1]); close(ready[1]); receive(ready[0]); close(ready[0]);
  int status;
  check(write(fds[0], "\032", 1) == 1 &&
        waitpid(child, &status, WUNTRACED) == child && WIFSTOPPED(status) &&
        WSTOPSIG(status) == SIGTSTP, "Ctrl-Z targets slave foreground group");
  if (hangup)
  {
    close(fds[0]);
    check(waitpid(child, &status, 0) == child && WIFSIGNALED(status) &&
          WTERMSIG(status) == SIGHUP, "master close sends HUP and continues stopped group");
  }
  else
  {
    check(sigsend(child, SIGCONT) == 0 && write(fds[0], "\003", 1) == 1,
          "continue and Ctrl-C input");
    check(waitpid(child, &status, 0) == child && WIFSIGNALED(status) &&
          WTERMSIG(status) == SIGINT, "Ctrl-C targets slave foreground group");
    close(fds[0]);
  }
}
static void allocation()
{
  int fds[2];
  for (int i = 0; i < 40; ++i)
    check(openpty(reinterpret_cast<int *>(~0UL)) == -1, "failed copyout rollback");
  for (int i = 0; i < 128; ++i) { pair(fds); close_pair(fds); }
  int fill[64], count = 0;
  while (count < 64)
  {
    int fd = dup(0);
    if (fd < 0) break;
    fill[count++] = fd;
  }
  check(count > 0, "descriptor fill");
  close(fill[--count]); // Exactly one descriptor free; openpty needs two.
  check(openpty(fds) == -1, "descriptor shortage rollback");
  int last = dup(0);
  check(last >= 0, "failed openpty did not consume descriptor");
  close(last);
  while (count) close(fill[--count]);
  pair(fds); close_pair(fds);
  printf("ptytest: allocation and signals OK\n");
}
extern "C" void run_poll_checks();
extern "C" void run_nonblock_checks();
extern "C" void check_terminal_flags();
extern "C" void nonblock_terminal_mode(bool terminate);

extern "C" int main(int argc, char **argv)
{
  if (argc > 1 && strcmp(argv[1], "poll") == 0)
  {
    run_poll_checks();
    exit(0);
  }
  if (argc > 1 && strcmp(argv[1], "nonblock") == 0) { run_nonblock_checks(); exit(0); }
  if (argc > 1 && strcmp(argv[1], "checkflags") == 0) { check_terminal_flags(); exit(0); }
  if (argc > 1 && strcmp(argv[1], "nbmode") == 0) nonblock_terminal_mode(false);
  if (argc > 1 && strcmp(argv[1], "nbkill") == 0) nonblock_terminal_mode(true);
  basic(); flow(false); flow(true); closure(); signals(false); signals(true); allocation();
  printf("ptytest: all tests succeeded\n");
  exit(0);
}
