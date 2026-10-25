module;

#include "kernel/support/types.h"
#include "kernel/syscall/syscall.h"

module xv.syscall;

import :handlers;
import xv.process;
import xv.print;

namespace xv::syscall
{
// Map the shared user ABI numbers to private module handlers.
using SyscallHandler = uint64 (*)(void);

static SyscallHandler lookup_syscall(int number)
{
  switch (number)
  {
    // clang-format off
  case SYS_fork: return sys_fork;
  case SYS_exit: return sys_exit;
  case SYS_wait: return sys_wait;
  case SYS_pipe: return sys_pipe;
  case SYS_read: return sys_read;
  case SYS_kill: return sys_kill;
  case SYS_exec: return sys_exec;
  case SYS_fstat: return sys_fstat;
  case SYS_chdir: return sys_chdir;
  case SYS_dup: return sys_dup;
  case SYS_getpid: return sys_getpid;
  case SYS_sbrk: return sys_sbrk;
  case SYS_pause: return sys_pause;
  case SYS_uptime: return sys_uptime;
  case SYS_open: return sys_open;
  case SYS_write: return sys_write;
  case SYS_mknod: return sys_mknod;
  case SYS_unlink: return sys_unlink;
  case SYS_link: return sys_link;
  case SYS_mkdir: return sys_mkdir;
  case SYS_close: return sys_close;
  case SYS_sync: return sys_sync;
  case SYS_ioctl: return sys_ioctl;
#ifdef LAB_NET
  case SYS_bind: return sys_bind;
  case SYS_unbind: return sys_unbind;
  case SYS_send: return sys_send;
  case SYS_recv: return sys_recv;
#endif
#ifdef LAB_PGTBL
  case SYS_pgpte: return sys_pgpte;
  case SYS_vmprint: return sys_vmprint;
  case SYS_pgaccess: return sys_pgaccess;
  case SYS_ksupernpte: return sys_ksupernpte;
#endif
#ifdef LAB_LOCK
  case SYS_rwlktest: return sys_rwlktest;
  case SYS_cpupin: return sys_cpupin;
#endif
  // clang-format on
  default:
    return nullptr;
  }
}

void dispatch()
{
  int num;
  xv::process::Process *p = xv::process::current();

  num = p->registers()->a7;
  auto handler = lookup_syscall(num);
  if (handler)
  {
    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->registers()->a0
    p->registers()->a0 = handler();
  }
  else
  {
    xv::print::printk("%d %s: unknown sys call %d\n", p->id(), p->debug_name(), num);
    p->registers()->a0 = -1;
  }
}
} // namespace xv::syscall
