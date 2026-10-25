#pragma once
extern "C" {
#pragma once
#ifdef LAB_MMAP
typedef unsigned long size_t;
typedef long int off_t;
#endif

#define SBRK_ERROR ((char *)-1)

struct stat;
struct termios;
struct pollfd;

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int *);
int waitpid(int pid, int *status, int options);
int pipe(int *);
// Anonymous PTY: fds[0] is master, fds[1] is slave.
int openpty(int fds[2]);
int poll(struct pollfd *fds, int count, int timeout_ms);
int write(int, const void *, int);
int read(int, void *, int);
int close(int);
int kill(int);
int sigsend(int target, int signal);
int sigdisposition(int signal, int action);
int tcgetpgrp(int fd);
int tcsetpgrp(int fd, int group);
int exec(const char *, char **);
int open(const char *, int);
int mknod(const char *, short, short);
int unlink(const char *);
int fstat(int fd, struct stat *);
int link(const char *, const char *);
int mkdir(const char *);
int chdir(const char *);
int dup(int);
int getpid(void);
int getpgrp(void);
int setpgid(int pid, int pgid);
int getsid(int pid);
int setsid(void);
char *sys_sbrk(int, int);
int pause(int);
int uptime(void);
int sync(void);
int ioctl(int, unsigned, unsigned long);
int tcgetattr(int, struct termios *);
int tcsetattr(int, int, const struct termios *);
int tcflush(int, int);
void cfmakeraw(struct termios *);
#ifdef LAB_NET
int bind(uint16);
int unbind(uint16);
int send(uint16, uint32, uint16, char *, uint32);
int recv(uint16, uint32 *, uint16 *, char *, uint32);
#endif
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
