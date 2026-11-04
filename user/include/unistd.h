#pragma once
#include <sys/types.h>
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifdef __cplusplus
extern "C"
{
#endif
  ssize_t read(int, void *, size_t);
  ssize_t write(int, const void *, size_t);
  int close(int);
  int pipe(int[2]);
  int dup(int);
  pid_t fork(void);
  pid_t getpid(void);
  int ftruncate(int, off_t);
  off_t lseek(int, off_t, int);
  void _exit(int) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif
