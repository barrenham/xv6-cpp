#include "kernel/support/types.h"
#include "user/runtime/user.h"

static bool supported(int signal)
{
  return signal == SIGHUP || signal == SIGINT || signal == SIGKILL ||
         signal == SIGTERM || signal == SIGCONT || signal == SIGSTOP ||
         signal == SIGTSTP || signal == SIGCHLD || signal == SIGTTIN || signal == SIGUSR1 ||
         signal == SIGUSR2 || signal == SIGSEGV;
}

int sigemptyset(sigset_t *set)
{
  if (!set) return -1;
  *set = 0;
  return 0;
}

int sigfillset(sigset_t *set)
{
  if (!set) return -1;
  *set = 0;
  for (int i = 1; i < 32; ++i) if (supported(i)) *set |= 1U << i;
  return 0;
}

int sigaddset(sigset_t *set, int signal)
{
  if (!set || !supported(signal)) return -1;
  *set |= 1U << signal;
  return 0;
}

int sigdelset(sigset_t *set, int signal)
{
  if (!set || !supported(signal)) return -1;
  *set &= ~(1U << signal);
  return 0;
}

int sigismember(const sigset_t *set, int signal)
{
  if (!set || !supported(signal)) return -1;
  return (*set & (1U << signal)) != 0;
}

sighandler_t signal(int number, sighandler_t handler)
{
  struct sigaction action{handler, 0, SA_RESTART}, old;
  return sigaction(number, &action, &old) < 0 ? SIG_ERR : old.sa_handler;
}
