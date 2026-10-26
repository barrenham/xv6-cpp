#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/tty/termios.h"
#include "kernel/process/signals/signal.h"
#include "user/runtime/user.h"

static void check(bool ok, const char *message)
{
  if (!ok)
  {
    fprintf(2, "fgtest: FAIL: %s\n", message);
    exit(1);
  }
}

static void controls(int terminal)
{
  int group = getpgrp();
  check(tcgetpgrp(terminal) == group, "command owns foreground");
  check(tcsetpgrp(terminal, group) == 0, "same-session foreground set");
  check(tcsetpgrp(terminal, -1) == -1 && tcsetpgrp(terminal, 0x7fffffff) == -1,
        "invalid foreground group");
  check(ioctl(terminal, TIOCSCTTY, 0) == -1, "only session leader claims terminal");
  check(ioctl(terminal, TIOCGPGRP, ~0UL) == -1 &&
            ioctl(terminal, TIOCSPGRP, ~0UL) == -1 && tcgetpgrp(terminal) == group,
        "bad pointers preserve foreground");
  int fds[2];
  check(pipe(fds) == 0, "pipe");
  check(tcgetpgrp(fds[0]) == -1, "pipe is not a terminal");
  close(fds[0]);
  close(fds[1]);
  int child = fork();
  check(child >= 0, "foreign-session fork");
  if (child == 0)
  {
    check(setsid() == getpid(), "foreign session");
    check(tcgetpgrp(terminal) == -1 && tcsetpgrp(terminal, group) == -1 &&
              ioctl(terminal, TIOCSCTTY, 0) == -1,
          "cannot steal controlling terminal");
    exit(0);
  }
  int status;
  check(wait(&status) == child && status == 0, "foreign-session checks");
  printf("fgtest: controls passed sid=%d\n", getsid(0));
}

static void ready(const char *mode)
{
  char line[128];
  int used = 0;
  auto text = [&](const char *value)
  {
    while (*value && used < 110)
    {
      line[used++] = *value++;
    }
  };
  auto number = [&](int value)
  {
    check(used < 110, "ready mode length");
    char reverse[16];
    int count = 0;
    do
    {
      reverse[count++] = '0' + value % 10;
      value /= 10;
    } while (value);
    while (count)
    {
      line[used++] = reverse[--count];
    }
  };
  text("fgtest: ");
  text(mode);
  text(" ready pid=");
  number(getpid());
  text(" pgid=");
  number(getpgrp());
  line[used++] = '\n';
  check(write(2, line, used) == used, "atomic ready message");
}

extern "C" int main(int argc, char **argv)
{
  int terminal = open("/dev/console", O_RDWR);
  check(terminal >= 0, "console open");
  check(sigdisposition(SIGINT, SIGNAL_DEFAULT) == SIGNAL_DEFAULT,
        "shell restored child SIGINT default");
  if (argc > 1 && strcmp(argv[1], "controls") == 0)
  {
    controls(terminal);
    return 0;
  }
  if (argc > 2 && strcmp(argv[1], "absent") == 0)
  {
    check(sigsend(atoi(argv[2]), 0) == -1, "foreground member fully reaped");
    printf("fgtest: member absent\n");
    return 0;
  }
  if (argc > 2 && strcmp(argv[1], "cleanup") == 0)
  {
    int pid = atoi(argv[2]);
    check(sigsend(pid, 0) == 0, "background survives foreground interrupt");
    check(sigsend(pid, SIGKILL) == 0, "background cleanup");
    for (int i = 0; i < 20 && sigsend(pid, 0) == 0; ++i)
    {
      pause(1);
    }
    check(sigsend(pid, 0) == -1, "background reaped by init");
    printf("fgtest: background survived\n");
    return 0;
  }
  const char *mode = argc > 1 ? argv[1] : "spin";
  bool background = strcmp(mode, "background") == 0;
  int foreground = tcgetpgrp(terminal);
  check(background ? foreground != getpgrp() : foreground == getpgrp(),
        "foreground/background ownership");
  if (background)
  {
    char byte;
    sigdisposition(SIGTTIN, SIGNAL_IGNORE);
    check(read(terminal, &byte, 1) == -1, "background read does not steal input");
    sigdisposition(SIGTTIN, SIGNAL_DEFAULT);
  }
  ready(mode);
  char data[256]{};
  for (;;)
  {
    if (background)
    {
      pause(1000000);
    }
    else if (strcmp(mode, "read") == 0 || strcmp(mode, "sink") == 0)
    {
      read(0, data, sizeof(data));
    }
    else if (strcmp(mode, "source") == 0)
    {
      write(1, data, sizeof(data));
    }
    else
    {
      asm volatile("" ::: "memory");
    }
  }
}
