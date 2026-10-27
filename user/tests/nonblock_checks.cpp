#include "kernel/fs/image_paths.h"
#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/io_status.h"
#include "kernel/fs/poll.h"
#include "kernel/tty/termios.h"
#include "kernel/process/wait/wait.h"
#include "kernel/process/signals/signal.h"
#include "user/runtime/user.h"

namespace
{
static char data[1536], copy[1536];
void check(bool ok, const char *message)
{
  if (!ok) { fprintf(2, "nbtest: FAIL: %s\n", message); exit(1); }
}
void nb(int fd)
{
  int flags = fcntl(fd, F_GETFL);
  check(flags >= 0 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0, "set nonblocking");
}
void raw(int fd)
{
  termios settings;
  check(tcgetattr(fd, &settings) == 0, "get raw settings");
  cfmakeraw(&settings);
  check(tcsetattr(fd, TCSANOW, &settings) == 0, "raw settings");
}
void reap(int pid)
{
  int status;
  check(waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0,
        "child exit");
}
void flags()
{
  int fds[2]; check(pipe(fds) == 0, "flags pipe");
  check(fcntl(fds[0], F_GETFL) == O_RDONLY && fcntl(fds[1], F_GETFL) == O_WRONLY,
        "pipe access flags");
  int alias = dup(fds[0]); check(alias >= 0, "duplicate flags");
  nb(alias);
  check(fcntl(fds[0], F_GETFL) == (O_RDONLY | O_NONBLOCK) &&
        fcntl(fds[1], F_GETFL) == O_WRONLY, "dup shared, peer independent");
  check(fcntl(alias, F_SETFL, O_RDWR | O_NONBLOCK) == 0 &&
        (fcntl(alias, F_GETFL) & O_ACCMODE) == O_RDONLY, "access bits immutable");
  int before = fcntl(alias, F_GETFL);
  check(fcntl(alias, F_SETFL, O_CREATE) == -1 && fcntl(alias, F_GETFL) == before &&
        fcntl(alias, 999, 0) == -1 && fcntl(-1, F_GETFL) == -1,
        "invalid fcntl unchanged");
  int child = fork(); check(child >= 0, "flags fork");
  if (child == 0)
  {
    check(fcntl(alias, F_SETFL, 0) == 0, "child clears flag");
    exit(0);
  }
  reap(child);
  check(fcntl(fds[0], F_GETFL) == O_RDONLY, "fork shares open-file status");
  close(alias); close(fds[0]); close(fds[1]);
  int first = open("/dev/console", O_RDONLY | O_NONBLOCK);
  int second = open("/dev/console", O_RDONLY);
  check(first >= 0 && second >= 0 && fcntl(first, F_GETFL) == O_NONBLOCK &&
        fcntl(second, F_GETFL) == O_RDONLY, "independent opens of one TTY");
  check(read(first, copy, 1) == -EAGAIN && read(first, copy, 0) == 0,
        "nonblocking serial input");
  close(first); close(second);
  int regular = open(XV_README_PATH, O_RDONLY | O_NONBLOCK);
  check(regular >= 0 && read(regular, copy, 1) == 1, "regular file accepts nonblock");
  close(regular);
  int directory = open(".", O_RDONLY | O_NONBLOCK);
  check(directory >= 0 && fcntl(directory, F_GETFL) == O_NONBLOCK,
        "nonblock does not change directory access mode");
  close(directory);
  int input_before = fcntl(0, F_GETFL);
  termios terminal_before, terminal_after;
  check(tcgetattr(0, &terminal_before) == 0, "foreground guard settings");
  child = fork(); check(child >= 0, "background proxy fork");
  if (child == 0)
  {
    check(setpgid(0, 0) == 0, "background proxy group");
    char *arguments[] = {const_cast<char *>("pty"), nullptr};
    exec("/bin/pty", arguments);
    exit(99);
  }
  int status;
  check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
        WEXITSTATUS(status) == 1 && fcntl(0, F_GETFL) == input_before &&
        tcgetattr(0, &terminal_after) == 0 &&
        memcmp(&terminal_before, &terminal_after, sizeof(termios)) == 0,
        "background proxy rejects before changing shared terminal flags");
}
void pipes()
{
  int fds[2]; check(pipe(fds) == 0, "nonblock pipe"); nb(fds[0]); nb(fds[1]);
  check(read(fds[0], copy, 1) == -EAGAIN && read(fds[0], copy, 0) == 0 &&
        write(fds[1], data, 0) == 0, "empty pipe and zero lengths");
  check(write(fds[1], data, 1024) == 512 && write(fds[1], data, 1) == -EAGAIN,
        "large pipe partial write and full EAGAIN");
  check(read(fds[0], copy, 1) == 1 && write(fds[1], "ab", 2) == -EAGAIN &&
        write(fds[1], data + 512, 1024) == 1, "small pipe write is atomic");
  check(read(fds[0], copy + 1, 512) == 512 && memcmp(copy, data, 513) == 0,
        "partial pipe data unchanged");
  check(read(fds[0], copy, 1) == -EAGAIN, "drained open pipe is not EOF");
  close(fds[1]); check(read(fds[0], copy, 1) == 0, "closed pipe EOF"); close(fds[0]);
  check(pipe(fds) == 0, "broken nonblock pipe"); nb(fds[1]); close(fds[0]);
  check(write(fds[1], data, 1) == -1, "broken pipe is not EAGAIN"); close(fds[1]);
}
void terminals()
{
  int fds[2]; check(openpty(fds) == 0, "nonblock PTY"); nb(fds[0]); nb(fds[1]);
  check(read(fds[0], copy, 1) == -EAGAIN && read(fds[1], copy, 1) == -EAGAIN,
        "empty master and canonical slave");
  termios settings; tcgetattr(fds[1], &settings); settings.c_lflag &= ~ECHO;
  check(tcsetattr(fds[1], TCSANOW, &settings) == 0 && write(fds[0], "a", 1) == 1 &&
        read(fds[1], copy, 1) == -EAGAIN && write(fds[0], "\004", 1) == 1 &&
        read(fds[1], copy, 32) == 1 && copy[0] == 'a', "canonical pending line and EOF");
  check(write(fds[0], "\004", 1) == 1 && read(fds[1], copy, 1) == 0 &&
        read(fds[1], copy, 1) == -EAGAIN, "EOF record consumed exactly once");
  raw(fds[1]); tcgetattr(fds[1], &settings);
  settings.c_cc[VMIN] = 8; settings.c_cc[VTIME] = 5;
  check(tcsetattr(fds[1], TCSANOW, &settings) == 0 && write(fds[0], "ab", 2) == 2 &&
        read(fds[1], copy, 32) == 2 && read(fds[1], copy, 32) == -EAGAIN,
        "nonblock overrides VMIN and VTIME");
  settings.c_cc[VMIN] = settings.c_cc[VTIME] = 0;
  check(tcsetattr(fds[1], TCSANOW, &settings) == 0 && read(fds[1], copy, 1) == -EAGAIN,
        "empty raw nonblock returns EAGAIN even VMIN=0");
  raw(fds[1]);
  int written = 0, received = 0;
  while (received < sizeof(data))
  {
    if (written < sizeof(data))
    {
      int n = write(fds[0], data + written, sizeof(data) - written);
      check(n > 0 || n == -EAGAIN, "master nonblocking partial");
      if (n > 0) written += n;
    }
    int n = read(fds[1], copy + received, sizeof(copy) - received);
    check(n > 0, "slave drains partial input"); received += n;
  }
  check(written == sizeof(data) && memcmp(copy, data, sizeof(data)) == 0,
        "large raw input retains all bytes");
  check(write(fds[1], data, sizeof(data)) == 1024 && write(fds[1], data, 1) == -EAGAIN &&
        read(fds[0], copy, 1) == 1, "slave output partial and backpressure");
  tcgetattr(fds[1], &settings); settings.c_oflag = OPOST | ONLCR;
  check(tcsetattr(fds[1], TCSANOW, &settings) == 0 &&
        write(fds[1], "\n", 1) == -EAGAIN && write(fds[1], "z", 1) == 1,
        "CRLF needs two slots, failed write consumes none");
  check(read(fds[0], copy, sizeof(copy)) == 1024 && copy[1023] == 'z', "drain full output");
  memset(copy, 'x', 1022);
  check(write(fds[1], copy, 1022) == 1022 && write(fds[1], "a\nZ", 3) == 1 &&
        read(fds[0], copy, sizeof(copy)) == 1023 && copy[1022] == 'a',
        "partial result counts original source bytes");
  check(write(fds[1], "\nZ", 2) == 2 && read(fds[0], copy, sizeof(copy)) == 3 &&
        memcmp(copy, "\r\nZ", 3) == 0, "retry emits exactly one complete CRLF");
  close(fds[0]);
  check(read(fds[1], copy, 1) == 0 && write(fds[1], data, 1) == -1,
        "slave hangup distinguished from EAGAIN"); close(fds[1]);
  check(openpty(fds) == 0, "master hangup pair"); nb(fds[0]); close(fds[1]);
  check(read(fds[0], copy, 1) == 0 && write(fds[0], data, 1) == -1,
        "master hangup distinguished from EAGAIN"); close(fds[0]);
}
}
extern "C" void check_terminal_flags()
{
  check(!(fcntl(0, F_GETFL) & O_NONBLOCK), "shell input flag restored");
  check(!(fcntl(1, F_GETFL) & O_NONBLOCK), "shell output flag restored");
  printf("ptytest: terminal flags restored\n");
}
extern "C" void nonblock_terminal_mode(bool terminate)
{
  termios settings; check(tcgetattr(0, &settings) == 0, "helper settings");
  settings.c_lflag &= ~(ICANON | ECHO); settings.c_lflag |= ISIG;
  settings.c_cc[VMIN] = 1; settings.c_cc[VTIME] = 0;
  check(tcsetattr(0, TCSANOW, &settings) == 0, "helper raw signals");
  nb(0);
  if (terminate) sigsend(getpid(), SIGKILL);
  check(write(2, "ptytest: nonblock ready\n", 24) == 24, "helper ready");
  for (;;)
  {
    pollfd descriptor{0, POLLIN, 0};
    poll(&descriptor, 1, -1);
    read(0, copy, sizeof(copy));
  }
}
extern "C" void run_nonblock_checks()
{
  for (unsigned i = 0; i < sizeof(data); ++i) data[i] = i;
  flags(); pipes(); terminals(); check_terminal_flags();
  printf("ptytest: nonblock tests succeeded\n");
}
