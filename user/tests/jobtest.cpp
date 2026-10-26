#include "kernel/support/types.h"
#include "kernel/process/signals/signal.h"
#include "kernel/process/wait/wait.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"

static void check(bool ok, const char *message)
{
  if (!ok)
  {
    fprintf(2, "jobtest: FAIL: %s\n", message);
    exit(1);
  }
}

static void send(int fd, int value)
{
  check(write(fd, &value, sizeof(value)) == sizeof(value), "pipe send");
}

static int receive(int fd)
{
  int value;
  check(read(fd, &value, sizeof(value)) == sizeof(value), "pipe receive");
  return value;
}

static void contracts()
{
  int ready[2], work[2];
  check(pipe(ready) == 0 && pipe(work) == 0, "contract pipes");
  int child = fork();
  check(child >= 0, "contract fork");
  if (child == 0)
  {
    close(ready[0]);
    close(work[1]);
    send(ready[1], 1);
    check(receive(work[0]) == 42, "read restarted after stop/continue");
    exit(7);
  }
  close(ready[1]);
  close(work[0]);
  receive(ready[0]);
  int status = 0x1234;
  check(waitpid(child, &status, WNOHANG) == 0 && status == 0x1234,
        "WNOHANG leaves status unchanged");
  check(waitpid(child, &status, 0x4000) == -1 &&
            waitpid(-0x7fffffff - 1, &status, 0) == -1,
        "invalid options and selector");
  check(sigsend(child, SIGSTOP) == 0, "SIGSTOP send");
  check(waitpid(child, reinterpret_cast<int *>(~0UL), WUNTRACED) == -1,
        "bad status pointer does not consume event");
  check(waitpid(child, &status, WUNTRACED) == child && WIFSTOPPED(status) &&
            WSTOPSIG(status) == SIGSTOP,
        "stop event");
  check(waitpid(child, &status, WNOHANG | WUNTRACED) == 0, "stop reported once");
  check(sigsend(child, SIGCONT) == 0, "SIGCONT send");
  check(waitpid(child, &status, WCONTINUED) == child && WIFCONTINUED(status),
        "continue event");
  send(work[1], 42);
  check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
            WEXITSTATUS(status) == 7,
        "resumed read and encoded exit");
  close(ready[0]);
  close(work[1]);
  child = fork();
  check(child >= 0, "stopped kill fork");
  if (child == 0)
  {
    for (;;)
    {
      asm volatile("" ::: "memory");
    }
  }
  check(sigsend(child, SIGSTOP) == 0 && waitpid(child, &status, WUNTRACED) == child &&
            WIFSTOPPED(status),
        "early stop before first user dispatch");
  check(sigsend(child, SIGKILL) == 0 && waitpid(child, &status, 0) == child &&
            WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL,
        "SIGKILL wakes stopped process");
  child = fork();
  check(child >= 0, "pending termination fork");
  if (child == 0)
  {
    for (;;)
    {
      asm volatile("" ::: "memory");
    }
  }
  check(sigsend(child, SIGSTOP) == 0 && waitpid(child, &status, WUNTRACED) == child &&
            WIFSTOPPED(status),
        "pending termination stop");
  check(sigsend(child, SIGTERM) == 0 && waitpid(child, &status, WNOHANG) == 0,
        "ordinary fatal signal stays pending while stopped");
  check(sigsend(child, SIGCONT) == 0 && waitpid(child, &status, 0) == child &&
            WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM,
        "continue delivers pending termination");
  printf("jobtest: contracts passed\n");
}

extern "C" int main(int argc, char **argv)
{
  if (argc > 1 && strcmp(argv[1], "exit") == 0)
  {
    return 0;
  }
  if (argc > 2 && strcmp(argv[1], "delay") == 0)
  {
    pause(atoi(argv[2]));
    return 0;
  }
  if (argc > 1 && strcmp(argv[1], "descriptors") == 0)
  {
    // The shell's private terminal and SIGCHLD pipe must not survive exec.
    for (int fd = 3; fd < 16; ++fd)
      check(close(fd) == -1, "shell private descriptor leaked into command");
    printf("jobtest: descriptors clean\n");
    return 0;
  }
  if (argc > 1 && strcmp(argv[1], "contracts") == 0)
  {
    contracts();
    return 0;
  }
  if (argc > 1 && strcmp(argv[1], "raw") == 0)
  {
    termios settings;
    check(tcgetattr(0, &settings) == 0, "raw settings");
    settings.c_lflag &= ~(ICANON | ECHO);
    settings.c_lflag |= ISIG;
    settings.c_cc[VMIN] = 1;
    settings.c_cc[VTIME] = 0;
    check(tcsetattr(0, TCSANOW, &settings) == 0, "noncanonical mode with signals");
    write(2, "jobtest: raw ready\n", 19);
    char byte;
    for (;;)
    {
      read(0, &byte, 1);
    }
  }
  if (argc > 1 && strcmp(argv[1], "progress") == 0)
  {
    write(2, "jobtest: progress ready\n", 24);
    for (;;)
    {
      pause(1);
      write(2, "jobtest: tick\n", 14);
    }
  }
  fprintf(2, "usage: jobtest contracts|raw|progress\n");
  return 1;
}
