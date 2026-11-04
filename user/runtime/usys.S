#include "kernel/syscall/syscall.h"
.global kmod
kmod:
 li a7, SYS_kmod
 ecall
 ret
.global fork
fork:
 li a7, SYS_fork
 ecall
 ret
.global exit
exit:
 li a7, SYS_exit
 ecall
 ret
.global wait
wait:
 li a7, SYS_wait
 ecall
 ret
.global pipe
pipe:
 li a7, SYS_pipe
 ecall
 ret
.global __xv_read
__xv_read:
 li a7, SYS_read
 ecall
 ret
.global kill
kill:
 li a7, SYS_kill
 ecall
 ret
.global fstat
fstat:
 li a7, SYS_fstat
 ecall
 ret
.global chdir
chdir:
 li a7, SYS_chdir
 ecall
 ret
.global dup
dup:
 li a7, SYS_dup
 ecall
 ret
.global getpid
getpid:
 li a7, SYS_getpid
 ecall
 ret
.global sys_sbrk
sys_sbrk:
 li a7, SYS_sbrk
 ecall
 ret
.global pause
pause:
 li a7, SYS_pause
 ecall
 ret
.global uptime
uptime:
 li a7, SYS_uptime
 ecall
 ret
.global open
open:
 li a7, SYS_open
 ecall
 ret
.global __xv_write
__xv_write:
 li a7, SYS_write
 ecall
 ret
.global mknod
mknod:
 li a7, SYS_mknod
 ecall
 ret
.global unlink
unlink:
 li a7, SYS_unlink
 ecall
 ret
.global link
link:
 li a7, SYS_link
 ecall
 ret
.global mkdir
mkdir:
 li a7, SYS_mkdir
 ecall
 ret
.global __xv_close
__xv_close:
 li a7, SYS_close
 ecall
 ret
.global sync
sync:
 li a7, SYS_sync
 ecall
 ret
.global trace
trace:
 li a7, SYS_trace
 ecall
 ret
.global interpose
interpose:
 li a7, SYS_interpose
 ecall
 ret
.global sigalarm
sigalarm:
 li a7, SYS_sigalarm
 ecall
 ret
.global sigreturn
sigreturn:
 li a7, SYS_sigreturn
 ecall
 ret
.global symlink
symlink:
 li a7, SYS_symlink
 ecall
 ret
.global __xv_mmap
__xv_mmap:
 li a7, SYS_mmap
 ecall
 ret
.global __xv_munmap
__xv_munmap:
 li a7, SYS_munmap
 ecall
 ret
.global pgpte
pgpte:
 li a7, SYS_pgpte
 ecall
 ret
.global vmprint
vmprint:
 li a7, SYS_vmprint
 ecall
 ret
.global rwlktest
rwlktest:
 li a7, SYS_rwlktest
 ecall
 ret
.global cpupin
cpupin:
 li a7, SYS_cpupin
 ecall
 ret
.global pgaccess
pgaccess:
 li a7, SYS_pgaccess
 ecall
 ret
.global ksupernpte
ksupernpte:
 li a7, SYS_ksupernpte
 ecall
 ret

.global ioctl
ioctl:
 li a7, SYS_ioctl
 ecall
 ret

.global getpgrp
getpgrp:
 li a7, SYS_getpgrp
 ecall
 ret

.global setpgid
setpgid:
 li a7, SYS_setpgid
 ecall
 ret

.global getsid
getsid:
 li a7, SYS_getsid
 ecall
 ret

.global setsid
setsid:
 li a7, SYS_setsid
 ecall
 ret

.global sigsend
sigsend:
 li a7, SYS_sigsend
 ecall
 ret

.global sigdisposition
sigdisposition:
 li a7, SYS_sigdisposition
 ecall
 ret

.global __xv_waitpid
__xv_waitpid:
 li a7, SYS_waitpid
 ecall
 ret

.global openpty
openpty:
 li a7, SYS_openpty
 ecall
 ret

.global __xv_poll
__xv_poll:
 li a7, SYS_poll
 ecall
 ret

.global __xv_fcntl
__xv_fcntl:
 li a7, SYS_fcntl
 ecall
 ret

.global sigaction
sigaction:
 la a3, __signal_return
 li a7, SYS_sigaction
 ecall
 ret
.global sigprocmask
sigprocmask:
 li a7, SYS_sigprocmask
 ecall
 ret
.global sigpending
sigpending:
 li a7, SYS_sigpending
 ecall
 ret
.global __xv_sigsuspend
__xv_sigsuspend:
 li a7, SYS_sigsuspend
 ecall
 ret
.global __signal_return
__signal_return:
 li a7, SYS_sigreturn
 ecall
 # A failed restoration must not execute arbitrary stack bytes.
 ebreak

.global __xv_socket
__xv_socket:
 li a7, SYS_socket
 ecall
 ret
.global __xv_sockbind
__xv_sockbind:
 li a7, SYS_sockbind
 ecall
 ret
.global __xv_connect
__xv_connect:
 li a7, SYS_connect
 ecall
 ret
.global __xv_listen
__xv_listen:
 li a7, SYS_listen
 ecall
 ret
.global __xv_accept
__xv_accept:
 li a7, SYS_accept
 ecall
 ret
.global __xv_sendto
__xv_sendto:
 li a7, SYS_sendto
 ecall
 ret
.global __xv_recvfrom
__xv_recvfrom:
 li a7, SYS_recvfrom
 ecall
 ret
.global __xv_shutdown
__xv_shutdown:
 li a7, SYS_shutdown
 ecall
 ret
.global __xv_getsockname
__xv_getsockname:
 li a7, SYS_getsockname
 ecall
 ret
.global __xv_getpeername
__xv_getpeername:
 li a7, SYS_getpeername
 ecall
 ret
.global __xv_getsockopt
__xv_getsockopt:
 li a7, SYS_getsockopt
 ecall
 ret

.global netctl
netctl:
 li a7, SYS_netctl
 ecall
 ret

.global execve
execve:
 li a7, SYS_execve
 ecall
 ret

.global getuid
getuid:
 li a7, SYS_getuid
 ecall
 ret

.global geteuid
geteuid:
 li a7, SYS_geteuid
 ecall
 ret

.global getgid
getgid:
 li a7, SYS_getgid
 ecall
 ret

.global getegid
getegid:
 li a7, SYS_getegid
 ecall
 ret

.global setuid
setuid:
 li a7, SYS_setuid
 ecall
 ret

.global seteuid
seteuid:
 li a7, SYS_seteuid
 ecall
 ret

.global setgid
setgid:
 li a7, SYS_setgid
 ecall
 ret

.global setegid
setegid:
 li a7, SYS_setegid
 ecall
 ret

.global getgroups
getgroups:
 li a7, SYS_getgroups
 ecall
 ret

.global setgroups
setgroups:
 li a7, SYS_setgroups
 ecall
 ret

.global umask
umask:
 li a7, SYS_umask
 ecall
 ret

.global chmod
chmod:
 li a7, SYS_chmod
 ecall
 ret

.global chown
chown:
 li a7, SYS_chown
 ecall
 ret

.global access
access:
 li a7, SYS_access
 ecall
 ret

.global stat
stat:
 li a7, SYS_stat
 ecall
 ret

.global rename
rename:
 li a7, SYS_rename
 ecall
 ret

.global mount
mount:
 li a7, SYS_mount
 ecall
 ret

.global umount
umount:
 li a7, SYS_umount
 ecall
 ret

.global mountinfo
mountinfo:
 li a7, SYS_mountinfo
 ecall
 ret

.global clockticks
clockticks:
 li a7, SYS_clockticks
 ecall
 ret

.global __xv_lseek
__xv_lseek:
 li a7, SYS_lseek
 ecall
 ret

.global __xv_ftruncate
__xv_ftruncate:
 li a7, SYS_ftruncate
 ecall
 ret

.global fsync
fsync:
 li a7, SYS_fsync
 ecall
 ret

.global readlink
readlink:
 li a7, SYS_readlink
 ecall
 ret

.global lstat
lstat:
 li a7, SYS_lstat
 ecall
 ret

.global getdents64
getdents64:
 li a7, SYS_getdents64
 ecall
 ret

.global fsserve
fsserve:
 li a7, SYS_fsserve
 ecall
 ret

.global fsregister
fsregister:
 li a7, SYS_fsregister
 ecall
 ret

.global fexecve
fexecve:
 li a7, SYS_fexecve
 ecall
 ret

.global mountfs
mountfs:
 li a7, SYS_mountfs
 ecall
 ret

.global __xv_socketpair
__xv_socketpair:
 li a7, SYS_socketpair
 ecall
 ret

.global __xv_sendmsg
__xv_sendmsg:
 li a7, SYS_sendmsg
 ecall
 ret

.global __xv_recvmsg
__xv_recvmsg:
 li a7, SYS_recvmsg
 ecall
 ret

.global __xv_shm_open
__xv_shm_open:
 li a7, SYS_shm_open
 ecall
 ret

.global __xv_shm_unlink
__xv_shm_unlink:
 li a7, SYS_shm_unlink
 ecall
 ret
