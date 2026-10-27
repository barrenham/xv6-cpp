#pragma once
// System call numbers
#define SYS_fork   1
#define SYS_exit   2
#define SYS_wait   3
#define SYS_pipe   4
#define SYS_read   5
#define SYS_kill   6
#define SYS_exec   7
#define SYS_fstat  8
#define SYS_chdir  9
#define SYS_dup    10
#define SYS_getpid 11
#define SYS_sbrk   12
#define SYS_pause  13
#define SYS_uptime 14
#define SYS_open   15
#define SYS_write  16
#define SYS_mknod  17
#define SYS_unlink 18
#define SYS_link   19
#define SYS_mkdir  20
#define SYS_close  21
#define SYS_sync   22
#define SYS_ioctl  40
#define SYS_getpgrp 41
#define SYS_setpgid 42
#define SYS_getsid  43
#define SYS_setsid  44
#define SYS_sigsend 45

// System calls for labs
#define SYS_trace     23
#define SYS_interpose 24
#define SYS_sigalarm  25
#define SYS_sigreturn 26
#define SYS_symlink   27
#define SYS_mmap      28
#define SYS_munmap    29
#define SYS_pgpte     34
#define SYS_vmprint   35
#define SYS_rwlktest  36
#define SYS_cpupin    37
#define SYS_pgaccess  38
#define SYS_ksupernpte  39

#define SYS_sigdisposition 46

#define SYS_waitpid 47
#define SYS_openpty 48
#define SYS_poll 49
#define SYS_fcntl 50
#define SYS_sigaction 51
#define SYS_sigprocmask 52
#define SYS_sigpending 53
#define SYS_sigsuspend 54

#define SYS_socket 55
#define SYS_sockbind 56
#define SYS_connect 57
#define SYS_listen 58
#define SYS_accept 59
#define SYS_sendto 60
#define SYS_recvfrom 61
#define SYS_shutdown 62
#define SYS_getsockname 63
#define SYS_getpeername 64
#define SYS_getsockopt 65

#define SYS_netctl 66

#define SYS_execve 67

#define SYS_getuid 68

#define SYS_geteuid 69

#define SYS_getgid 70

#define SYS_getegid 71

#define SYS_setuid 72

#define SYS_seteuid 73

#define SYS_setgid 74

#define SYS_setegid 75

#define SYS_getgroups 76

#define SYS_setgroups 77

#define SYS_umask 78

#define SYS_chmod 79

#define SYS_chown 80

#define SYS_access 81

#define SYS_stat 82

#define SYS_rename 83
#define SYS_mount 84
#define SYS_umount 85
#define SYS_mountinfo 86

#define SYS_clockticks 87

#define SYS_lseek 88

#define SYS_ftruncate 89

#define SYS_fsync 90

#define SYS_readlink 91

#define SYS_lstat 92
