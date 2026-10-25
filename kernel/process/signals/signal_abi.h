#pragma once
#include "kernel/process/signals/signal.h"

// Project ABI: bit n represents signal n. Unsupported bits are rejected.
typedef unsigned int sigset_t;
typedef int sig_atomic_t;
typedef void (*sighandler_t)(int);
#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)-1)
#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2
#define SA_NODEFER 0x40000000U
#define SA_RESETHAND 0x80000000U
#define SA_RESTART 0x10000000U
struct sigaction
{
  sighandler_t sa_handler;
  sigset_t sa_mask;
  unsigned int sa_flags;
};
#ifndef EINTR
#define EINTR 4
#endif
