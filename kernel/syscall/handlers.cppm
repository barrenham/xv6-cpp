module xv.syscall:handlers;

namespace xv::syscall
{
// Prototypes for the functions that handle system calls.
unsigned long sys_fork(void);
unsigned long sys_exit(void);
unsigned long sys_wait(void);
unsigned long sys_waitpid(void);
unsigned long sys_pipe(void);
unsigned long sys_read(void);
unsigned long sys_kill(void);
unsigned long sys_exec(void);
unsigned long sys_fstat(void);
unsigned long sys_chdir(void);
unsigned long sys_dup(void);
unsigned long sys_getpid(void);
unsigned long sys_sbrk(void);
unsigned long sys_pause(void);
unsigned long sys_uptime(void);
unsigned long sys_open(void);
unsigned long sys_write(void);
unsigned long sys_mknod(void);
unsigned long sys_unlink(void);
unsigned long sys_link(void);
unsigned long sys_mkdir(void);
unsigned long sys_close(void);
unsigned long sys_sync(void);
unsigned long sys_ioctl(void);
unsigned long sys_getpgrp(void);
unsigned long sys_setpgid(void);
unsigned long sys_getsid(void);
unsigned long sys_setsid(void);
unsigned long sys_sigsend(void);
unsigned long sys_sigdisposition(void);

#ifdef LAB_NET
unsigned long sys_bind(void);
unsigned long sys_unbind(void);
unsigned long sys_send(void);
unsigned long sys_recv(void);
#endif
#ifdef LAB_PGTBL
unsigned long sys_pgpte(void);
unsigned long sys_vmprint(void);
unsigned long sys_pgaccess(void);
unsigned long sys_ksupernpte(void);
#endif
#ifdef LAB_LOCK
unsigned long sys_rwlktest(void);
unsigned long sys_cpupin(void);
#endif

} // namespace xv::syscall
