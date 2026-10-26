#include "kernel/support/types.h"
#include "kernel/process/signals/signal.h"
#include "user/runtime/user.h"

static void check(bool ok, const char *message)
{
  if (!ok)
  {
    fprintf(2, "signaltest: FAIL: %s\n", message);
    exit(1);
  }
}

static void send(int fd, int value)
{
  check(write(fd, &value, sizeof(value)) == sizeof(value), "synchronization send");
}

static int receive(int fd)
{
  int value;
  check(read(fd, &value, sizeof(value)) == sizeof(value), "synchronization receive");
  return value;
}

static void wait_signal(int pid, int signal)
{
  int status = 0;
  check(wait(&status) == pid && status == SIGNAL_EXIT_STATUS(signal),
        "signal exit status");
  check(sigsend(pid, 0) == -1, "reaped PID absent");
}

enum Work
{
  Spin,
  Timer,
  ReadPipe,
  WritePipe,
  ReadTty,
  WaitChild
};

static void blocked(Work work, int signal)
{
  int ready[2], io[2];
  check(pipe(ready) == 0 && pipe(io) == 0, "blocking pipes");
  int pid = fork();
  check(pid >= 0, "blocking fork");
  if (pid == 0)
  {
    close(ready[0]);
    char data[1024]{};
    if (work == WaitChild)
    {
      close(io[0]);
      int child = fork();
      check(child >= 0, "wait worker fork");
      if (child == 0)
      {
        close(ready[1]);
        pause(1000000);
        exit(97);
      }
      close(io[1]);
      send(ready[1], child);
      wait(nullptr);
      exit(97);
    }
    if (work == WritePipe)
    {
      close(io[0]);
    }
    if (work == ReadPipe)
    {
      close(io[1]);
    }
    send(ready[1], 1);
    if (work == Spin)
    {
      for (;;)
      {
        asm volatile("" ::: "memory");
      }
    }
    if (work == Timer)
    {
      pause(1000000);
    }
    if (work == ReadPipe)
    {
      read(io[0], data, 1);
    }
    if (work == WritePipe)
    {
      write(io[1], data, sizeof(data));
    }
    if (work == ReadTty)
    {
      read(0, data, sizeof(data));
    }
    exit(97);
  }
  close(ready[1]);
  if (work == WritePipe || work == WaitChild)
  {
    close(io[1]);
  }
  if (work == ReadPipe)
  {
    close(io[0]);
  }
  int worker = receive(ready[0]);
  pause(1);
  check(sigsend(pid, 0) == 0, "signal zero probe");
  check(sigsend(pid, signal) == 0, "single process delivery");
  wait_signal(pid, signal);
  char data[128];
  if (work == ReadPipe)
  {
    check(write(io[1], "x", 1) == -1, "signal closes pipe reader");
  }
  if (work == WritePipe)
  {
    int bytes = 0, n;
    while ((n = read(io[0], data, sizeof(data))) > 0)
    {
      bytes += n;
    }
    check(bytes > 0 && n == 0, "signal closes blocked pipe writer");
  }
  if (work == WaitChild)
  {
    check(sigsend(worker, SIGKILL) == 0, "signal adopted child");
    check(read(io[0], data, 1) == 0, "adopted child releases pipe");
  }
  close(ready[0]);
  if (work != ReadPipe)
  {
    close(io[0]);
  }
  if (work != WritePipe && work != WaitChild)
  {
    close(io[1]);
  }
}

static void queued_tty_readers()
{
  int ready[2];
  check(pipe(ready) == 0, "TTY readers pipe");
  int children[2];
  for (int i = 0; i < 2; ++i)
  {
    children[i] = fork();
    check(children[i] >= 0, "TTY reader fork");
    if (children[i] == 0)
    {
      close(ready[0]);
      send(ready[1], 1);
      char byte;
      read(0, &byte, 1);
      exit(97);
    }
    receive(ready[0]);
    pause(1);
  }
  // The second reader is waiting for the read lock held by the first.
  check(sigsend(children[1], SIGINT) == 0, "signal queued TTY reader");
  wait_signal(children[1], SIGINT);
  check(sigsend(children[0], SIGTERM) == 0, "signal owning TTY reader");
  wait_signal(children[0], SIGTERM);
  close(ready[0]);
  close(ready[1]);
}

static void group_delivery()
{
  int ready[2];
  check(pipe(ready) == 0, "group ready pipe");
  int first = fork();
  check(first >= 0, "group leader fork");
  if (first == 0)
  {
    check(setpgid(0, 0) == 0, "group creation");
    send(ready[1], 1);
    pause(1000000);
    exit(97);
  }
  receive(ready[0]);
  int second = fork();
  check(second >= 0, "group member fork");
  if (second == 0)
  {
    check(setpgid(0, first) == 0, "group join");
    send(ready[1], 1);
    for (;;)
    {
      asm volatile("" ::: "memory");
    }
  }
  receive(ready[0]);
  int outsider = fork();
  check(outsider >= 0, "outsider fork");
  if (outsider == 0)
  {
    send(ready[1], 1);
    pause(1000000);
    exit(97);
  }
  receive(ready[0]);
  check(sigsend(-first, 0) == 0 && sigsend(-first, SIGTERM) == 0,
        "signal selected process group");
  for (int i = 0; i < 2; ++i)
  {
    int status;
    int pid = wait(&status);
    check((pid == first || pid == second) && status == SIGNAL_EXIT_STATUS(SIGTERM),
          "all selected members terminate");
  }
  check(sigsend(-first, 0) == -1 && sigsend(outsider, 0) == 0,
        "empty group absent and outsider unaffected");
  check(kill(outsider) == 0, "legacy kill still available");
  int status;
  check(wait(&status) == outsider && status == -1, "legacy kill status preserved");
  close(ready[0]);
  close(ready[1]);
}

static void leaderless_group()
{
  int ready[2], lifetime[2];
  check(pipe(ready) == 0 && pipe(lifetime) == 0, "leaderless group pipes");
  int leader = fork();
  check(leader >= 0, "leaderless fork");
  if (leader == 0)
  {
    close(ready[0]);
    close(lifetime[0]);
    check(setpgid(0, 0) == 0, "leaderless group creation");
    int survivor = fork();
    check(survivor >= 0, "leaderless survivor fork");
    if (survivor == 0)
    {
      close(ready[1]);
      pause(1000000);
      exit(97);
    }
    send(ready[1], survivor);
    exit(0);
  }
  close(ready[1]);
  close(lifetime[1]);
  int survivor = receive(ready[0]);
  int status;
  check(wait(&status) == leader && status == 0, "leader reaped");
  check(sigsend(survivor, 0) == 0 && sigsend(-leader, SIGINT) == 0,
        "deliver to group after leader reaping");
  char byte;
  check(read(lifetime[0], &byte, 1) == 0, "leaderless member cleans up");
  close(ready[0]);
  close(lifetime[0]);
}

void signal_handler_checks();
void child_signal_checks();
int signal_exec_check();

extern "C" int main(int argc, char **argv)
{
  if (argc == 2 && strcmp(argv[1], "exec-check") == 0) return signal_exec_check();
  signal_handler_checks();
  child_signal_checks();
  check(sigsend(getpid(), 3) == -1 && sigsend(getpid(), 32) == -1 &&
            sigsend(getpid(), -1) == -1 && sigsend(-1, 0) == -1 &&
            sigsend(-0x7fffffff - 1, 0) == -1 && sigsend(0x7fffffff, SIGINT) == -1,
        "invalid selections and unsupported signals");
  check(sigsend(1, 0) == 0 && sigsend(1, SIGKILL) == -1 && kill(1) == -1,
        "initial process protection");
  blocked(Spin, SIGINT);
  blocked(Timer, SIGTERM);
  blocked(ReadPipe, SIGINT);
  blocked(WritePipe, SIGTERM);
  blocked(ReadTty, SIGKILL);
  blocked(WaitChild, SIGINT);
  queued_tty_readers();
  group_delivery();
  leaderless_group();
  for (int i = 0; i < 32; ++i)
  {
    int child = fork();
    check(child >= 0, "early signal fork");
    if (child == 0)
    {
      for (;;)
      {
        asm volatile("" ::: "memory");
      }
    }
    check(sigsend(child, SIGINT) == 0, "signal around first dispatch");
    wait_signal(child, SIGINT);
  }
  int child = fork();
  check(child >= 0, "self-group fork");
  if (child == 0)
  {
    check(setpgid(0, 0) == 0, "self-group creation");
    sigsend(0, SIGINT);
    exit(97);
  }
  wait_signal(child, SIGINT);
  printf("signaltest: all tests succeeded\n");
  return 0;
}
