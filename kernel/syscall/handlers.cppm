module xv.syscall:handlers;

namespace xv::syscall
{
unsigned long sys_stat();
unsigned long sys_access();
unsigned long sys_chown();
unsigned long sys_chmod();
unsigned long sys_umask();
unsigned long sys_setgroups();
unsigned long sys_getgroups();
unsigned long sys_setegid();
unsigned long sys_setgid();
unsigned long sys_seteuid();
unsigned long sys_setuid();
unsigned long sys_getegid();
unsigned long sys_getgid();
unsigned long sys_geteuid();
unsigned long sys_getuid();
unsigned long sys_execve();
unsigned long sys_lseek();
unsigned long sys_ftruncate();
unsigned long sys_fsync();
unsigned long sys_readlink();
unsigned long sys_lstat();
unsigned long sys_symlink();
unsigned long sys_clockticks();
unsigned long sys_rename();
unsigned long sys_mount();
unsigned long sys_umount();
unsigned long sys_mountinfo();
// Prototypes for the functions that handle system calls.
unsigned long sys_netctl();
unsigned long sys_socket();
unsigned long sys_sockbind();
unsigned long sys_connect();
unsigned long sys_listen();
unsigned long sys_accept();
unsigned long sys_sendto();
unsigned long sys_recvfrom();
unsigned long sys_shutdown();
unsigned long sys_getsockname();
unsigned long sys_getpeername();
unsigned long sys_getsockopt();
unsigned long sys_fork(void);
unsigned long sys_exit(void);
unsigned long sys_wait(void);
unsigned long sys_waitpid(void);
unsigned long sys_pipe(void);
unsigned long sys_openpty(void);
unsigned long sys_poll(void);
unsigned long sys_fcntl(void);
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
unsigned long sys_sigaction(void);
unsigned long sys_sigprocmask(void);
unsigned long sys_sigpending(void);
unsigned long sys_sigsuspend(void);
unsigned long sys_sigreturn(void);
unsigned long sys_sigsend(void);
unsigned long sys_sigdisposition(void);

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
