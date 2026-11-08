#pragma once
#include "kernel/support/types.h"
#include "kernel/process/signals/signal_abi.h"
#include "kernel/net/socket/socket.h"
#include "errno.h"
#include "posix_types.h"
#include "kernel/io/mman.h"
#include "kernel/net/control.h"
#include "dns/dns.h"
extern "C" {
#pragma once

#define SBRK_ERROR ((char *)-1)

struct stat;
struct mount_info;
int mount(const char *type, const char *path);
int mountfs(const char *type, const char *path, const char *source, const char *options);
long kmod(unsigned operation, unsigned long argument, unsigned long value);
int fsserve(const char *path);
int fsregister(const void *registration);
int fexecve(int descriptor, char *const arguments[], char *const environment[]);
int umount(const char *path);
int mountinfo(int index, struct mount_info *info);
struct termios;
struct pollfd;
struct sigaction;

int netctl(unsigned operation, struct netctl_request *request);
int shm_open(const char *, int, mode_t);
int shm_unlink(const char *);
void *mmap(void *, size_t, int, int, int, off_t);
int munmap(void *, size_t);
int mprotect(void *, size_t, int);
int msync(void *, size_t, int);
int socketpair(int, int, int, int descriptors[2]);
long sendmsg(int, const struct msghdr *, int);
long recvmsg(int, struct msghdr *, int);
int socket(int domain, int type, int protocol);
int bind(int fd, const struct sockaddr *, socklen_t);
ssize_t send(int, const void *, size_t, int);
ssize_t recv(int, void *, size_t, int);
int sockbind(int fd, const struct sockaddr *address, socklen_t length);
int connect(int fd, const struct sockaddr *address, socklen_t length);
int listen(int fd, int backlog);
int accept(int fd, struct sockaddr *address, socklen_t *length);
ssize_t sendto(int fd, const void *data, size_t length, int flags,
               const struct sockaddr *address, socklen_t address_length);
ssize_t recvfrom(int fd, void *data, size_t length, int flags, struct sockaddr *address,
                 socklen_t *address_length);
int shutdown(int fd, int how);
int getsockname(int fd, struct sockaddr *address, socklen_t *length);
int getpeername(int fd, struct sockaddr *address, socklen_t *length);
int getsockopt(int fd, int level, int option, void *value, socklen_t *length);
// system calls
int fork(void);
int sched_yield(void);
void xv_user_enter(void *, void (*)(void *), void *) __attribute__((noreturn));
int exit(int) __attribute__((noreturn));
int wait(int *);
int waitpid(int pid, int *status, int options);
int pipe(int *);
// Anonymous PTY: fds[0] is master, fds[1] is slave.
int openpty(int fds[2]);
int posix_openpt(int flags);
int grantpt(int fd);
int unlockpt(int fd);
int ptsname_r(int fd, char *buffer, unsigned length);
int poll(struct pollfd *fds, unsigned long count, int timeout_ms);
int fcntl(int fd, int command, ...);
ssize_t write(int, const void *, size_t);
ssize_t read(int, void *, size_t);
int close(int);
int kill(int);
sighandler_t signal(int signal, sighandler_t handler);
int sigaction(int signal, const struct sigaction *action, struct sigaction *old);
int sigprocmask(int how, const sigset_t *set, sigset_t *old);
int sigpending(sigset_t *set);
int sigsuspend(const sigset_t *set);
int sigreturn(void);
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signal);
int sigdelset(sigset_t *set, int signal);
int sigismember(const sigset_t *set, int signal);
int sigsend(int target, int signal);
int sigdisposition(int signal, int action);
int tcgetpgrp(int fd);
int tcsetpgrp(int fd, int group);
extern char **environ;
int exec(const char *, char **);
int execve(const char *, char *const *, char *const *);
int execv(const char *, char *const *);
int execvp(const char *, char *const *);
char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);
int clearenv(void);
int open(const char *, int);
int mknod(const char *, short, short);
int unlink(const char *);
int fstat(int fd, struct stat *);
int link(const char *, const char *);
int mkdir(const char *);
int chdir(const char *);
char *getcwd(char *buffer, unsigned capacity);
long lseek(int fd, long offset, int origin);
int ftruncate(int fd, long size);
int getdents64(int fd, void *buffer, int count);
int fsync(int fd);
int symlink(const char *target, const char *path);
int readlink(const char *path, char *buffer, int size);
int lstat(const char *path, struct stat *value);
int rename(const char *old_path, const char *new_path);
int dup(int);
int getpid(void);
int getuid(void); int geteuid(void); int getgid(void); int getegid(void);
int setuid(unsigned uid); int seteuid(unsigned uid); int setgid(unsigned gid); int setegid(unsigned gid);
int getgroups(int count, unsigned *groups); int setgroups(int count, const unsigned *groups);
unsigned umask(unsigned mask);
int chmod(const char *path, unsigned mode);
int chown(const char *path, int uid, int gid);
int access(const char *path, int mode);
int getpgrp(void);
int setpgid(int pid, int pgid);
int getsid(int pid);
int setsid(void);
char *sys_sbrk(int, int);
int pause(int);
int uptime(void);
uint64 clockticks(void);
int sync(void);
int poweroff(void);
int reboot(void);
struct xv_scheduler_status;
int schedctl(unsigned operation, unsigned policy, struct xv_scheduler_status *);
int ioctl(int, unsigned, unsigned long);
int tcgetattr(int, struct termios *);
int tcsetattr(int, int, const struct termios *);
int tcflush(int, int);
void cfmakeraw(struct termios *);
#ifdef LAB_PGTBL
int ugetpid(void);
uint64 pgpte(void *);
void vmprint(void);
int pgaccess(void *base, int len, void *mask);
int ksupernpte(uint64*, uint64 *);
#endif
#ifdef LAB_LOCK
int rwlktest(int resetflag);
int cpupin(int);
#endif

// ulib.c
int stat(const char *, struct stat *);
char *strcpy(char *, const char *);
void *memmove(void *, const void *, int);
char *strchr(const char *, char c);
int strcmp(const char *, const char *);
char *gets(char *, int max);
uint strlen(const char *);
void *memset(void *, int, uint);
int atoi(const char *);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);
char *sbrk(int);
char *sbrklazy(int);
#ifdef LAB_LOCK
int statistics(void *, int);
#endif

// printf.c
void fprintf(int, const char *, ...) __attribute__((format(printf, 2, 3)));
void printf(const char *, ...) __attribute__((format(printf, 1, 2)));

// umalloc.c
void *malloc(uint);
void free(void *);

}
