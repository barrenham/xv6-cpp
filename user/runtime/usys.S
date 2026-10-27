#include "kernel/syscall/syscall.h"
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
.global read
read:
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
.global write
write:
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
.global close
close:
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
.global mmap
mmap:
 li a7, SYS_mmap
 ecall
 ret
.global munmap
munmap:
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

.global waitpid
waitpid:
 li a7, SYS_waitpid
 ecall
 ret

.global openpty
openpty:
 li a7, SYS_openpty
 ecall
 ret

.global poll
poll:
 li a7, SYS_poll
 ecall
 ret

.global fcntl
fcntl:
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
.global sigsuspend
sigsuspend:
 li a7, SYS_sigsuspend
 ecall
 ret
.global __signal_return
__signal_return:
 li a7, SYS_sigreturn
 ecall
 # A failed restoration must not execute arbitrary stack bytes.
 ebreak

.global socket
socket:
 li a7, SYS_socket
 ecall
 ret
.global sockbind
sockbind:
 li a7, SYS_sockbind
 ecall
 ret
.global connect
connect:
 li a7, SYS_connect
 ecall
 ret
.global listen
listen:
 li a7, SYS_listen
 ecall
 ret
.global accept
accept:
 li a7, SYS_accept
 ecall
 ret
.global sendto
sendto:
 li a7, SYS_sendto
 ecall
 ret
.global recvfrom
recvfrom:
 li a7, SYS_recvfrom
 ecall
 ret
.global shutdown
shutdown:
 li a7, SYS_shutdown
 ecall
 ret
.global getsockname
getsockname:
 li a7, SYS_getsockname
 ecall
 ret
.global getpeername
getpeername:
 li a7, SYS_getpeername
 ecall
 ret
.global getsockopt
getsockopt:
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

.global lseek
lseek:
 li a7, SYS_lseek
 ecall
 ret

.global ftruncate
ftruncate:
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
