#include "kernel/support/types.h"
#include "kernel/process/wait/wait.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"

static volatile sig_atomic_t first, second, depth, deepest, mode;
static int report_fd = -1;
static volatile sig_atomic_t fork_child;
static void check(bool ok, const char *message)
{
  if (!ok)
  {
    fprintf(2, "signaltest: FAIL: %s\n", message);
    exit(1);
  }
}
static unsigned bit(int signal) { return 1U << signal; }
static void other(int signal)
{
  check(signal == SIGUSR2, "handler argument");
  second = second + 1;
  if (mode == 2) check(depth == 1, "different signal nests");
}
static void handler(int signal)
{
  check(signal == SIGUSR1, "primary argument");
  first = first + 1;
  depth = depth + 1;
  if (depth > deepest) deepest = depth;
  sigset_t mask;
  check(sigprocmask(99, nullptr, &mask) == 0, "handler mask query");
  if (mode == 1)
  {
    check((mask & (bit(SIGUSR1) | bit(SIGUSR2))) == (bit(SIGUSR1) | bit(SIGUSR2)),
          "sa_mask and automatic self blocking");
    if (first == 1)
    {
      check(sigsend(getpid(), SIGUSR1) == 0 && sigsend(getpid(), SIGUSR1) == 0 &&
            sigsend(getpid(), SIGUSR2) == 0, "queue within handler");
      check(first == 1 && second == 0, "masked handlers deferred");
    }
  }
  if (mode == 2)
    check(sigsend(getpid(), SIGUSR2) == 0 && second == 1, "nested handler returns");
  if (mode == 3 && first == 1)
    check(sigsend(getpid(), SIGUSR1) == 0 && first == 2, "SA_NODEFER reentry");
  if (mode == 4)
  {
    struct sigaction action;
    check(sigaction(SIGUSR1, nullptr, &action) == 0 && action.sa_handler == SIG_DFL,
          "SA_RESETHAND reset before invocation");
  }
  if (mode == 5)
  {
    int child = fork();
    check(child >= 0, "fork inside handler");
    fork_child = child ? child : -1;
  }
  if (report_fd >= 0) check(write(report_fd, "h", 1) == 1, "handler acknowledgment");
  // Force caller-saved temporaries to change; sigreturn must restore them.
  asm volatile("li t0, -1; li t1, -2; li t2, -3; li t3, -4; li t4, -5; li t5, -6; li t6, -7"
               ::: "t0", "t1", "t2", "t3", "t4", "t5", "t6");
  depth = depth - 1;
}
static void install(unsigned flags = 0, unsigned mask = 0)
{
  struct sigaction action{handler, mask, flags}, other_action{other, 0, 0};
  check(sigaction(SIGUSR1, &action, nullptr) == 0 &&
        sigaction(SIGUSR2, &other_action, nullptr) == 0, "install handlers");
  first = 0; second = 0; depth = 0; deepest = 0;
}
static void wait_ok(int pid)
{
  int status;
  check(wait(&status) == pid && status == 0, "worker exit");
}
extern "C" int signal_register_probe(int pid);
extern "C" void signal_register_handler(int);
extern "C" void signal_corrupt_handler(int);
extern "C" void signal_badpc_handler(int);
extern "C" void signal_badsp_handler(int);
extern "C" void signal_stack_exhaust(int);

static void basics()
{
  sigset_t empty = 0, old, pending;
  check(sigreturn() == -1, "sigreturn outside handler rejected");
  check(sigemptyset(&old) == 0 && sigaddset(&old, SIGUSR1) == 0 &&
        sigismember(&old, SIGUSR1) == 1 && sigdelset(&old, SIGUSR1) == 0 &&
        sigismember(&old, SIGUSR1) == 0 && sigaddset(&old, 32) == -1, "set helpers");
  check(sigfillset(&old) == 0 && sigprocmask(SIG_SETMASK, &old, nullptr) == 0 &&
        sigprocmask(0, nullptr, &old) == 0 &&
        !(old & (bit(SIGKILL) | bit(SIGSTOP))), "unmaskable signals removed");
  check(sigprocmask(SIG_SETMASK, &empty, nullptr) == 0, "clear mask");
  install();
  mode = 0;
  struct sigaction register_action{signal_register_handler, 0, 0};
  check(sigaction(SIGUSR1, &register_action, nullptr) == 0 &&
        signal_register_probe(getpid()) == 0, "all integer registers and PC restored");
  install();
  check(sigsend(getpid(), SIGUSR1) == 0 && first == 1, "ordinary handler returns");
  sigset_t blocked = bit(SIGUSR1);
  check(sigprocmask(SIG_BLOCK, &blocked, &old) == 0 && old == 0, "block signal");
  check(sigsend(getpid(), SIGUSR1) == 0 && sigsend(getpid(), SIGUSR1) == 0 && first == 1,
        "blocked delivery deferred");
  check(sigpending(&pending) == 0 && pending == blocked, "pending blocked set");
  check(sigprocmask(SIG_UNBLOCK, &blocked, nullptr) == 0 && first == 2,
        "unblock delivers one coalesced signal");
  struct sigaction action{SIG_IGN, 0, 0};
  check(sigprocmask(SIG_BLOCK, &blocked, nullptr) == 0 &&
        sigsend(getpid(), SIGUSR1) == 0 && sigaction(SIGUSR1, &action, nullptr) == 0 &&
        sigpending(&pending) == 0 && pending == 0, "ignore discards pending");
  check(sigprocmask(SIG_SETMASK, &empty, nullptr) == 0, "restore empty mask");
  install(0, bit(SIGUSR2)); mode = 1;
  check(sigsend(getpid(), SIGUSR1) == 0 && first == 2 && second == 1 && deepest == 1,
        "deferred handlers return and restore masks");
  check(sigprocmask(0, nullptr, &old) == 0 && old == 0, "original mask restored");
  install(); mode = 2;
  check(sigsend(getpid(), SIGUSR1) == 0 && first == 1 && second == 1, "nested delivery");
  install(SA_NODEFER); mode = 3;
  check(sigsend(getpid(), SIGUSR1) == 0 && first == 2 && deepest == 2, "recursive delivery");
  install(SA_RESETHAND); mode = 4;
  check(sigsend(getpid(), SIGUSR1) == 0 && first == 1, "one-shot delivery");
  struct sigaction bad{SIG_IGN, 0, 1}, saved;
  check(sigaction(SIGKILL, &bad, nullptr) == -1 && sigaction(SIGSTOP, nullptr, &saved) == -1 &&
        sigaction(SIGUSR1, &bad, nullptr) == -1, "uncatchable signals and flags rejected");
  bad = {reinterpret_cast<sighandler_t>(0x7fffffffffffffffUL), 0, 0};
  check(sigaction(SIGUSR1, &bad, nullptr) == -1, "invalid handler rejected");
  bad = {handler, 0, 0};
  check(sigaction(SIGUSR1, &bad, reinterpret_cast<struct sigaction *>(1)) == -1 &&
        sigaction(SIGUSR1, nullptr, &saved) == 0 && saved.sa_handler == SIG_DFL,
        "failed oldact output leaves disposition unchanged");
  old = 1U << 31;
  check(sigprocmask(SIG_BLOCK, &old, nullptr) == -1 &&
        sigprocmask(9, &empty, nullptr) == -1, "invalid masks rejected");
  install(); mode = 0;
}

enum Operation { PipeRead, PollWait, TimerWait, ChildWait, TtyRead, PipeWrite };
static void interrupt(Operation operation, bool restart)
{
  int ready[2], io[2], ack[2];
  check(pipe(ready) == 0 && pipe(io) == 0 && pipe(ack) == 0, "interrupt pipes");
  int pid = fork();
  check(pid >= 0, "interrupt fork");
  if (pid == 0)
  {
    close(ready[0]); close(ack[0]);
    install(restart ? SA_RESTART : 0); mode = 0; report_fd = ack[1];
    char c = 0;
    int child = -1;
    if (operation == ChildWait)
    {
      child = fork();
      check(child >= 0, "wait worker fork");
      if (!child) { close(ready[1]); close(ack[1]); close(io[1]); read(io[0], &c, 1); exit(0); }
    }
    if (operation == PipeWrite)
    {
      char fill[512]{};
      check(write(io[1], fill, sizeof(fill)) == sizeof(fill), "fill pipe before blocking");
    }
    check(write(ready[1], "r", 1) == 1, "worker ready");
    int result;
    if (operation == PipeRead) result = read(io[0], &c, 1);
    else if (operation == PollWait) { pollfd fd{io[0], POLLIN, 0}; result = poll(&fd, 1, -1); }
    else if (operation == TimerWait) result = pause(100000);
    else if (operation == ChildWait) result = wait(nullptr);
    else if (operation == TtyRead) result = read(0, &c, 1);
    else result = write(io[1], "x", 1);
    check(first == 1, "blocked call executed handler");
    check(result == (restart ? (operation == ChildWait ? child : 1) : -EINTR),
          "interrupted call result or restart");
    exit(0);
  }
  close(ready[1]); close(ack[1]);
  char c;
  check(read(ready[0], &c, 1) == 1, "parent ready");
  pause(1);
  check(sigsend(pid, SIGUSR1) == 0 && read(ack[0], &c, 1) == 1, "interrupt sleeping worker");
  if (restart) check(write(io[1], "x", 1) == 1, "release restarted call");
  wait_ok(pid);
  close(ready[0]); close(ack[0]); close(io[0]); close(io[1]);
}

static void partial_write()
{
  int ready[2], io[2], ack[2];
  check(pipe(ready) == 0 && pipe(io) == 0 && pipe(ack) == 0, "partial transfer pipes");
  int child = fork(); check(child >= 0, "partial transfer fork");
  if (!child)
  {
    close(io[0]); install(SA_RESTART); mode = 0; report_fd = ack[1];
    char data[1024]{};
    check(write(ready[1], "r", 1) == 1, "partial writer ready");
    check(write(io[1], data, sizeof(data)) == 512 && first == 1,
          "partial write reports progress without restarting");
    exit(0);
  }
  close(io[1]); close(ack[1]);
  char c; check(read(ready[0], &c, 1) == 1, "partial writer synchronization"); pause(1);
  check(sigsend(child, SIGUSR1) == 0 && read(ack[0], &c, 1) == 1, "interrupt partial writer");
  wait_ok(child);
  int bytes = 0, n; char data[128];
  while ((n = read(io[0], data, sizeof(data))) > 0) bytes += n;
  check(n == 0 && bytes == 512, "partial write not duplicated");
  close(ready[0]); close(ready[1]); close(ack[0]); close(io[0]);
}

static void suspend_and_fork()
{
  install(); mode = 0;
  sigset_t blocked = bit(SIGUSR1), empty = 0, pending;
  check(sigprocmask(SIG_BLOCK, &blocked, nullptr) == 0 && sigsend(getpid(), SIGUSR1) == 0,
        "pending before fork");
  int child = fork();
  check(child >= 0, "inheritance fork");
  if (!child)
  {
    check(sigpending(&pending) == 0 && pending == 0, "fork clears pending");
    check(sigprocmask(0, nullptr, &pending) == 0 && pending == blocked, "fork inherits mask");
    check(sigsend(getpid(), SIGUSR1) == 0 && sigsuspend(&empty) == -EINTR && first == 1,
          "sigsuspend atomically unblocks pending handler");
    check(sigprocmask(0, nullptr, &pending) == 0 && pending == blocked,
          "sigsuspend restores original mask");
    exit(0);
  }
  wait_ok(child);
  check(sigpending(&pending) == 0 && pending == blocked && first == 0,
        "child consumption leaves parent pending");
  check(sigprocmask(SIG_SETMASK, &empty, nullptr) == 0 && first == 1, "parent unblocks");
  // Sleep with no pending signal: verifies wakeup rather than only the fast path.
  int ready[2]; check(pipe(ready) == 0, "suspend pipe");
  child = fork(); check(child >= 0, "suspend fork");
  if (!child)
  {
    first = 0;
    check(sigprocmask(SIG_BLOCK, &blocked, nullptr) == 0 && write(ready[1], "r", 1) == 1,
          "prepare suspend worker");
    check(sigsuspend(&empty) == -EINTR && first == 1, "sleeping sigsuspend wakes");
    exit(0);
  }
  char c; check(read(ready[0], &c, 1) == 1, "suspend ready"); pause(1);
  check(sigsend(child, SIGUSR1) == 0, "wake suspended child"); wait_ok(child);
  close(ready[0]); close(ready[1]);
}

int signal_exec_check()
{
  sigset_t mask, pending;
  struct sigaction action;
  check(sigprocmask(0, nullptr, &mask) == 0 && mask == bit(SIGUSR1), "exec preserves mask");
  check(sigpending(&pending) == 0 && pending == bit(SIGUSR1), "exec preserves pending");
  check(sigaction(SIGUSR1, nullptr, &action) == 0 && action.sa_handler == SIG_DFL,
        "exec resets caught actions");
  check(sigaction(SIGTERM, nullptr, &action) == 0 && action.sa_handler == SIG_IGN,
        "exec preserves ignored actions");
  check(sigreturn() == -1, "exec has no old signal frame");
  sigset_t empty = 0;
  sigprocmask(SIG_SETMASK, &empty, nullptr);
  exit(99);
}
static void exec_inheritance()
{
  int child = fork(); check(child >= 0, "exec fork");
  if (!child)
  {
    struct sigaction ignored{SIG_IGN, 0, 0};
    sigset_t blocked = bit(SIGUSR1);
    check(sigaction(SIGTERM, &ignored, nullptr) == 0 &&
          sigprocmask(SIG_BLOCK, &blocked, nullptr) == 0 && sigsend(getpid(), SIGUSR1) == 0,
          "prepare exec signals");
    char *args[] = {const_cast<char *>("signaltest"), const_cast<char *>("exec-check"), nullptr};
    exec("/usr/tests/signaltest", args); exit(99);
  }
  int status;
  check(wait(&status) == child && status == SIGNAL_EXIT_STATUS(SIGUSR1), "exec default delivery");
}

static void continue_handler(int signal)
{
  check(signal == SIGCONT, "continue handler argument");
  second = second + 1;
}
static void continue_semantics()
{
  for (int ignored = 0; ignored < 2; ++ignored)
  {
    int child = fork(); check(child >= 0, "continue worker fork");
    if (!child)
    {
      check(setpgid(0, 0) == 0, "continue worker group");
      second = 0;
      struct sigaction action{ignored ? SIG_IGN : continue_handler, 0, 0};
      sigset_t blocked = bit(SIGCONT), pending;
      check(sigaction(SIGCONT, &action, nullptr) == 0 &&
            sigprocmask(SIG_BLOCK, &blocked, nullptr) == 0, "block or ignore continue");
      check(sigsend(getpid(), SIGSTOP) == 0, "stop before continue");
      check(second == 0 && sigpending(&pending) == 0 &&
            pending == (ignored ? 0U : blocked), "continue resumes before blocked handler");
      check(sigprocmask(SIG_UNBLOCK, &blocked, nullptr) == 0 && second == (ignored ? 0 : 1),
            "continue handler delivered on unblock");
      exit(0);
    }
    int status;
    check(waitpid(child, &status, WUNTRACED) == child && WIFSTOPPED(status),
          "observe stopped worker");
    check(sigsend(-child, SIGCONT) == 0, "group continue resumes blocked/ignored signal");
    wait_ok(child);
  }
}
static void suspend_continue()
{
  int ready[2]; check(pipe(ready) == 0, "suspend/stop ready pipe");
  int child = fork(); check(child >= 0, "suspend/stop fork");
  if (!child)
  {
    sigset_t blocked = bit(SIGUSR1), empty = 0, mask;
    struct sigaction action{continue_handler, 0, 0}; second = 0;
    check(sigaction(SIGCONT, &action, nullptr) == 0 &&
          sigprocmask(SIG_BLOCK, &blocked, nullptr) == 0 && write(ready[1], "r", 1) == 1,
          "prepare suspend/stop worker");
    check(sigsuspend(&empty) == -EINTR && second == 1, "caught continue completes sigsuspend");
    check(sigprocmask(0, nullptr, &mask) == 0 && mask == blocked,
          "stop/continue preserves original suspend mask");
    exit(0);
  }
  char c; check(read(ready[0], &c, 1) == 1, "suspend/stop synchronization"); pause(1);
  check(sigsend(child, SIGSTOP) == 0, "stop suspended worker");
  int status;
  check(waitpid(child, &status, WUNTRACED) == child && WIFSTOPPED(status) &&
        sigsend(child, SIGCONT) == 0, "continue suspended worker");
  wait_ok(child); close(ready[0]); close(ready[1]);
}

static void blocked_does_not_interrupt()
{
  int ready[2], io[2]; check(pipe(ready) == 0 && pipe(io) == 0, "masked wait pipes");
  int child = fork(); check(child >= 0, "masked wait fork");
  if (!child)
  {
    install(); mode = 0;
    sigset_t blocked = bit(SIGUSR1);
    check(sigprocmask(SIG_BLOCK, &blocked, nullptr) == 0 && write(ready[1], "r", 1) == 1,
          "masked worker ready");
    char c;
    check(read(io[0], &c, 1) == 1 && c == 'x' && first == 0,
          "blocked signal does not interrupt read");
    check(sigprocmask(SIG_UNBLOCK, &blocked, nullptr) == 0 && first == 1,
          "blocked signal delivered after read and unblock");
    exit(0);
  }
  char c; check(read(ready[0], &c, 1) == 1, "masked worker synchronization");
  check(sigsend(child, SIGUSR1) == 0, "post blocked signal"); pause(1);
  int status;
  check(waitpid(child, &status, WNOHANG) == 0 && write(io[1], "x", 1) == 1,
        "masked worker remains alive and blocked");
  wait_ok(child);
  close(ready[0]); close(ready[1]); close(io[0]); close(io[1]);
}

static void handler_lifetime()
{
  install(); mode = 5; fork_child = 0;
  check(sigsend(getpid(), SIGUSR1) == 0 && first == 1, "forked handler returns");
  if (fork_child == -1) exit(0);
  wait_ok(fork_child);
  mode = 0;
  sighandler_t invalid[] = {signal_corrupt_handler, signal_badpc_handler, signal_badsp_handler};
  for (auto handler : invalid)
  {
    int child = fork(); check(child >= 0, "corrupt-frame fork");
    if (!child)
    {
      struct sigaction action{handler, 0, 0};
      check(sigaction(SIGUSR1, &action, nullptr) == 0, "install frame-corrupting handler");
      sigsend(getpid(), SIGUSR1); exit(99);
    }
    int status;
    check(wait(&status) == child && status == -1, "invalid frame rejected in user mode");
  }
  int child = fork(); check(child >= 0, "exhausted-stack fork");
  if (!child) { signal_stack_exhaust(getpid()); exit(99); }
  int status;
  check(wait(&status) == child && status == SIGNAL_EXIT_STATUS(SIGSEGV),
        "unwritable signal stack terminates only target");
}

void signal_handler_checks()
{
  basics();
  interrupt(PipeRead, false);
  interrupt(PollWait, false);
  interrupt(TimerWait, false);
  interrupt(ChildWait, false);
  interrupt(TtyRead, false);
  interrupt(PipeWrite, false);
  interrupt(PipeRead, true);
  interrupt(ChildWait, true);
  partial_write();
  suspend_and_fork();
  blocked_does_not_interrupt();
  continue_semantics();
  suspend_continue();
  exec_inheritance();
  handler_lifetime();
  struct sigaction defaults{SIG_DFL, 0, 0};
  check(sigaction(SIGUSR1, &defaults, nullptr) == 0 &&
        sigaction(SIGUSR2, &defaults, nullptr) == 0, "restore test dispositions");
  printf("signaltest: handlers, masks and contexts succeeded\n");
}
