#pragma once
#include "kernel/process/signals/signal_abi.h"
#ifdef __cplusplus
extern "C"
{
#endif
  sighandler_t signal(int, sighandler_t);
  int sigaction(int, const struct sigaction *, struct sigaction *);
  int sigprocmask(int, const sigset_t *, sigset_t *);
  int sigpending(sigset_t *);
  int sigsuspend(const sigset_t *);
  int sigemptyset(sigset_t *);
  int sigfillset(sigset_t *);
  int sigaddset(sigset_t *, int);
  int sigdelset(sigset_t *, int);
  int sigismember(const sigset_t *, int);
#ifdef __cplusplus
}
#endif
