module;
#include "kernel/support/flow.h"

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
  case SYS_execve: return sys_execve;
  case SYS_getuid: return sys_getuid;
  case SYS_geteuid: return sys_geteuid;
  case SYS_getgid: return sys_getgid;
  case SYS_getegid: return sys_getegid;
  case SYS_setuid: return sys_setuid;
  case SYS_seteuid: return sys_seteuid;
  case SYS_setgid: return sys_setgid;
  case SYS_setegid: return sys_setegid;
  case SYS_getgroups: return sys_getgroups;
  case SYS_setgroups: return sys_setgroups;
  case SYS_umask: return sys_umask;
  case SYS_chmod: return sys_chmod;
  case SYS_chown: return sys_chown;
  case SYS_access: return sys_access;
  case SYS_lseek: return sys_lseek;
  case SYS_ftruncate: return sys_ftruncate;
  case SYS_fsync: return sys_fsync;
  case SYS_readlink: return sys_readlink;
  case SYS_lstat: return sys_lstat;
  case SYS_getdents64: return sys_getdents64;
  case SYS_symlink: return sys_symlink;
  case SYS_clockticks: return sys_clockticks;
  case SYS_rename: return sys_rename;
  case SYS_mount: return sys_mount;
  case SYS_umount: return sys_umount;
  case SYS_mountinfo: return sys_mountinfo;
  case SYS_stat: return sys_stat;
  case SYS_netctl: return sys_netctl;
  case SYS_socket: return sys_socket;
  case SYS_sockbind: return sys_sockbind;
  case SYS_connect: return sys_connect;
  case SYS_listen: return sys_listen;
  case SYS_accept: return sys_accept;
  case SYS_sendto: return sys_sendto;
  case SYS_recvfrom: return sys_recvfrom;
  case SYS_shutdown: return sys_shutdown;
  case SYS_getsockname: return sys_getsockname;
  case SYS_getpeername: return sys_getpeername;
  case SYS_getsockopt: return sys_getsockopt;
  case SYS_fork: return sys_fork;
  case SYS_exit: return sys_exit;
  case SYS_wait: return sys_wait;
  case SYS_waitpid: return sys_waitpid;
  case SYS_openpty: return sys_openpty;
  case SYS_poll: return sys_poll;
  case SYS_fcntl: return sys_fcntl;
  case SYS_sigaction: return sys_sigaction;
  case SYS_sigprocmask: return sys_sigprocmask;
  case SYS_sigpending: return sys_sigpending;
  case SYS_sigsuspend: return sys_sigsuspend;
  case SYS_sigreturn: return sys_sigreturn;
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
  case SYS_getpgrp: return sys_getpgrp;
  case SYS_setpgid: return sys_setpgid;
  case SYS_getsid: return sys_getsid;
  case SYS_setsid: return sys_setsid;
  case SYS_sigsend: return sys_sigsend;
  case SYS_sigdisposition: return sys_sigdisposition;
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

namespace
{
namespace flow = xv::flow;
struct DispatchFlow
{
  struct Context
  {
    xv::process::Process *process;
    int number;
    SyscallHandler handler;
    unsigned long argument = 0, result = 0;
  };
  static bool known(Context &c) noexcept { return c.handler != nullptr; }
  static void invoke(Context &c) noexcept
  {
    c.argument = c.process->registers()->a0;
    c.process->begin_system_call();
    c.result = static_cast<unsigned long>(
        c.process->finish_system_call(c.number, c.argument, c.handler()));
  }
  static bool restart(Context &c) noexcept
  {
    bool restartable =
        c.number == SYS_read || c.number == SYS_write || c.number == SYS_wait ||
        c.number == SYS_waitpid || c.number == SYS_accept || c.number == SYS_recvfrom ||
        c.number == SYS_sendto || c.number == SYS_pause || c.number == SYS_sigsuspend;
    return c.result == ~0UL && restartable && c.process->stop_requested();
  }
  static void rewind(Context &c) noexcept
  {
    c.process->registers()->epc -= 4;
    c.process->registers()->a0 = c.argument;
  }
  static void publish(Context &c) noexcept { c.process->registers()->a0 = c.result; }
  static void unknown(Context &c) noexcept
  {
    xv::print::printk("%d %s: unknown sys call %d\n", c.process->id(),
                      c.process->debug_name(), c.number);
    c.process->registers()->a0 = -1;
  }
  struct Known;
  struct Invoke;
  struct Rewind;
  struct Publish;
  struct Unknown;
  using Graph = flow::graph<
      Known, flow::types<flow::node<Known, flow::noop,
                                    flow::branch<flow::condition<&known>,
                                                 flow::go<Invoke>, flow::go<Unknown>>>,
                         flow::node<Invoke, flow::action<&invoke>,
                                    flow::branch<flow::condition<&restart>,
                                                 flow::go<Rewind>, flow::go<Publish>>>,
                         flow::node<Rewind, flow::action<&rewind>, flow::stop>,
                         flow::node<Publish, flow::action<&publish>, flow::stop>,
                         flow::node<Unknown, flow::action<&unknown>, flow::stop>>>;
};

} // namespace

void dispatch()
{
  auto *process = xv::process::current();
  int number = process->registers()->a7;
  auto handler = lookup_syscall(number);
  DispatchFlow::Context context{process, number, handler};
  (void)DispatchFlow::Graph::run<0>(context);
}
} // namespace xv::syscall
