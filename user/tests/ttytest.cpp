#include "kernel/support/types.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"

static termios saved;

static void check(bool ok, const char *message)
{
  if (!ok)
  {
    tcsetattr(0, TCSANOW, &saved);
    fprintf(2, "ttytest: FAIL: %s\n", message);
    exit(1);
  }
}

extern "C" int main(int argc, char **argv)
{
  check(tcgetattr(0, &saved) == 0, "TCGETS");
  check(saved.c_lflag & ICANON, "canonical default");
  check(tcgetattr(-1, &saved) == -1, "invalid descriptor");
  check(ioctl(0, TCGETS, ~0UL) == -1, "invalid user address");
  int descriptors[2];
  check(pipe(descriptors) == 0, "pipe");
  check(tcgetattr(descriptors[0], &saved) == -1, "pipe is not a tty");
  close(descriptors[0]);
  close(descriptors[1]);
  winsize size{};
  check(ioctl(0, TIOCGWINSZ, reinterpret_cast<unsigned long>(&size)) == 0,
        "window get");
  check(size.ws_row == 24 && size.ws_col == 80, "window default");
  auto original_size = size;
  size.ws_row = 40;
  size.ws_col = 100;
  check(ioctl(0, TIOCSWINSZ, reinterpret_cast<unsigned long>(&size)) == 0,
        "window set");
  check(ioctl(0, TIOCSWINSZ, reinterpret_cast<unsigned long>(&original_size)) == 0,
        "window restore");
  termios raw = saved;
  cfmakeraw(&raw);
  raw.c_cc[VMIN] = 0;
  check(tcsetattr(0, TCSADRAIN, &raw) == 0, "raw mode and drain");
  char input[16];
  check(read(0, input, sizeof(input)) == 0, "MIN=0 TIME=0");
  raw.c_cc[VTIME] = 1;
  check(tcsetattr(0, TCSANOW, &raw) == 0, "timed mode");
  check(read(0, input, sizeof(input)) == 0, "MIN=0 TIME>0 expires");
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;
  check(tcsetattr(0, TCSANOW, &raw) == 0, "blocking raw mode");
  check(pipe(descriptors) == 0, "reader synchronization pipe");
  int child = fork();
  check(child >= 0, "reader fork");
  if (child == 0)
  {
    close(descriptors[0]);
    write(descriptors[1], "r", 1);
    read(0, input, 1);
    exit(0);
  }
  close(descriptors[1]);
  check(read(descriptors[0], input, 1) == 1, "reader started");
  close(descriptors[0]);
  pause(2);
  check(kill(child) == 0, "kill blocked TTY reader");
  int status = 0;
  check(wait(&status) == child && status != 0, "killed reader releases TTY lock");
  raw.c_cc[VMIN] = 0;
  check(tcsetattr(0, TCSANOW, &raw) == 0, "nonblocking raw mode");
  check(read(0, input, sizeof(input)) == 0, "read after killed reader");
  raw.c_lflag |= ISIG;
  check(tcsetattr(0, TCSANOW, &raw) == -1, "unsupported signals rejected");
  check(tcsetattr(0, TCSAFLUSH, &saved) == 0, "restore");
  check(tcflush(0, TCIFLUSH) == 0, "flush");
  if (argc > 1 && strcmp(argv[1], "interactive") == 0)
  {
    raw = saved;
    cfmakeraw(&raw);
    raw.c_cc[VMIN] = 3;
    check(tcsetattr(0, TCSANOW, &raw) == 0, "interactive raw mode");
    printf("ttytest: raw ready\n");
    check(read(0, input, sizeof(input)) == 3 && input[0] == 0 && input[1] == 4 &&
              input[2] == '\r',
          "UART raw binary input");
    check(tcsetattr(0, TCSAFLUSH, &saved) == 0, "interactive canonical mode");
    printf("ttytest: canonical ready\n");
    check(read(0, input, sizeof(input)) == 3 && memcmp(input, "ac\n", 3) == 0,
          "UART canonical editing");
    printf("ttytest: eof ready\n");
    check(read(0, input, sizeof(input)) == 0, "UART canonical EOF");
  }
  printf("ttytest: all tests succeeded\n");
  return 0;
}
