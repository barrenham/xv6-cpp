#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/io_status.h"
#include "kernel/fs/poll.h"
#include "kernel/process/wait/wait.h"
#include "user/runtime/user.h"

static volatile sig_atomic_t notifications;
static int channel[2];
static void check(bool ok, const char *message)
{
  if (!ok) { fprintf(2, "signaltest: FAIL: SIGCHLD %s\n", message); exit(1); }
}
static void handler(int signal)
{
  if (signal == SIGCHLD) notifications = notifications + 1;
  write(channel[1], "c", 1);
}
static int wait_event(int pid, int *status, int flags)
{
  int result;
  do { result = waitpid(pid, status, flags); } while (result == -EINTR);
  return result;
}
static void event()
{
  char c;
  for (;;)
  {
    int n = read(channel[0], &c, 1);
    if (n == 1) return;
    check(n == -EAGAIN || n == -EINTR, "notification read");
    pollfd fd{channel[0], POLLIN, 0};
    int ready = poll(&fd, 1, 3000);
    check(ready > 0 || ready == -EINTR, "parent notified while waiting on another fd");
  }
}
static void clear_events()
{
  char bytes[64];
  while (read(channel[0], bytes, sizeof(bytes)) > 0) {}
  notifications = 0;
}
void child_signal_checks()
{
  check(pipe(channel) == 0 && fcntl(channel[0], F_SETFL, O_NONBLOCK) == 0 &&
        fcntl(channel[1], F_SETFL, O_NONBLOCK) == 0, "event pipe");
  struct sigaction action{handler, 0, 0}, defaults{SIG_DFL, 0, 0};
  check(sigaction(SIGCHLD, &action, nullptr) == 0, "install handler");
  int child = fork(); check(child >= 0, "exit fork");
  if (!child) { pause(1); exit(7); }
  event();
  int status;
  check(wait_event(child, &status, 0) == child && WIFEXITED(status) &&
        WEXITSTATUS(status) == 7 && notifications == 1, "exit publishes waitable status");
  clear_events();

  int gate[2]; check(pipe(gate) == 0, "stop/continue gate");
  child = fork(); check(child >= 0, "stop fork");
  if (!child) { close(gate[1]); char c; read(gate[0], &c, 1); exit(0); }
  close(gate[0]);
  check(sigsend(child, SIGSTOP) == 0, "send stop"); event();
  check(wait_event(child, &status, WUNTRACED) == child && WIFSTOPPED(status) &&
        WSTOPSIG(status) == SIGSTOP, "stop status available after notification");
  clear_events();
  check(sigsend(child, SIGCONT) == 0, "send continue"); event();
  check(wait_event(child, &status, WCONTINUED) == child && WIFCONTINUED(status),
        "continue status available after notification");
  clear_events();
  check(sigsend(child, SIGCONT) == 0 && notifications == 0,
        "continue of running child does not create a new event");
  check(write(gate[1], "x", 1) == 1, "finish continued child"); close(gate[1]); event();
  check(wait_event(child, &status, 0) == child && WIFEXITED(status), "continued child exits");
  clear_events();

  sigset_t blocked = 1U << SIGCHLD, old, pending;
  check(sigprocmask(SIG_BLOCK, &blocked, &old) == 0, "block child signals");
  int children[8];
  for (int i = 0; i < 8; ++i)
  {
    children[i] = fork(); check(children[i] >= 0, "burst fork");
    if (!children[i]) exit(20 + i);
  }
  // Reaping while blocked makes all eight publications deterministic, even if
  // another hart delays an exit. Notifications remain pending independently.
  for (int i = 0; i < 8; ++i)
    check(wait_event(children[i], &status, 0) == children[i] && WIFEXITED(status) &&
          WEXITSTATUS(status) == 20 + i, "each coalesced exit has its own status");
  check(notifications == 0 && sigpending(&pending) == 0 && pending == blocked,
        "eight exits coalesce into one blocked notification");
  check(sigprocmask(SIG_SETMASK, &old, nullptr) == 0 && notifications == 1,
        "unblocking runs one handler"); event(); clear_events();

  // A grandchild must notify its parent, not this process.
  check(pipe(gate) == 0 && sigprocmask(SIG_BLOCK, &blocked, nullptr) == 0,
        "parent identity gate");
  int done[2]; check(pipe(done) == 0, "parent identity ready pipe");
  child = fork(); check(child >= 0, "parent identity fork");
  if (!child)
  {
    sigaction(SIGCHLD, &defaults, nullptr);
    int grandchild = fork(); check(grandchild >= 0, "grandchild fork");
    if (!grandchild) exit(0);
    check(wait_event(grandchild, &status, 0) == grandchild, "grandchild reaped by real parent");
    write(done[1], "d", 1); char c; read(gate[0], &c, 1); exit(0);
  }
  char c; check(read(done[0], &c, 1) == 1 && sigpending(&pending) == 0 && pending == 0,
                "grandchild exit does not notify grandparent");
  check(write(gate[1], "g", 1) == 1 && wait_event(child, &status, 0) == child,
        "real child exits");
  check(sigpending(&pending) == 0 && pending == blocked, "parent receives direct child exit");
  close(gate[0]); close(gate[1]); close(done[0]); close(done[1]);
  check(sigaction(SIGCHLD, &defaults, nullptr) == 0 && sigpending(&pending) == 0 && pending == 0,
        "default disposition clears pending signal");
  check(sigprocmask(SIG_SETMASK, &old, nullptr) == 0, "restore mask");

  // Both default and explicit ignore retain the project's wait/reap contract.
  for (int ignored = 0; ignored < 2; ++ignored)
  {
    struct sigaction value{ignored ? SIG_IGN : SIG_DFL, 0, 0};
    check(sigaction(SIGCHLD, &value, nullptr) == 0, "set default/ignore");
    child = fork(); check(child >= 0, "default/ignore fork");
    if (!child) exit(9);
    check(wait_event(child, &status, 0) == child && WEXITSTATUS(status) == 9,
          "default/ignore preserves wait status");
  }
  check(sigaction(SIGCHLD, &defaults, nullptr) == 0, "restore default");
  close(channel[0]); close(channel[1]);
  printf("signaltest: SIGCHLD notifications succeeded\n");
}
