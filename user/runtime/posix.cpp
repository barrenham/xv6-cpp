#include "kernel/support/types.h"
#include "user/runtime/errno.h"
#include "kernel/net/socket/socket.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include <stdarg.h>
#include "kernel/process/signals/signal_abi.h"

namespace
{
int process_errno;

long
result(long value) {
  if (value < 0)
  {
    errno = -value;
    return -1;
  }
  return value;
}
} // namespace

extern "C"
{
  int *
  __errno_location(void) noexcept {
    return &process_errno;
  }

  long __xv_socketpair(int, int, int, int *);
  long __xv_sendmsg(int, const msghdr *, int);
  long __xv_recvmsg(int, msghdr *, int);

  int
  socketpair(int domain, int type, int protocol, int descriptors[2]) {
    return result(__xv_socketpair(domain, type, protocol, descriptors));
  }

  long
  sendmsg(int descriptor, const msghdr *message, int flags) {
    return result(__xv_sendmsg(descriptor, message, flags));
  }

  long
  recvmsg(int descriptor, msghdr *message, int flags) {
    return result(__xv_recvmsg(descriptor, message, flags));
  }
}

extern "C"
{
  long __xv_shm_open(const char *, int, unsigned);
  long __xv_shm_unlink(const char *);
  long __xv_mmap(void *, unsigned long, int, int, int, long);
  long __xv_munmap(void *, unsigned long);

  int
  shm_open(const char *name, int flags, unsigned mode) {
    return result(__xv_shm_open(name, flags, mode));
  }

  int
  shm_unlink(const char *name) {
    return result(__xv_shm_unlink(name));
  }

  void *
  mmap(void *hint, unsigned long bytes, int protection, int flags, int fd,
       long offset) {
    return reinterpret_cast<void *>(
        result(__xv_mmap(hint, bytes, protection, flags, fd, offset)));
  }

  int
  munmap(void *address, unsigned long bytes) {
    return result(__xv_munmap(address, bytes));
  }
}

extern "C"
{
  long __xv_read(int, void *, unsigned long);
  long __xv_write(int, const void *, unsigned long);
  long __xv_close(int);
  long __xv_poll(void *, int, int);
  long __xv_fcntl(int, int, int);
  long __xv_ftruncate(int, long);
  long __xv_lseek(int, long, int);
  long __xv_socket(int, int, int);
  long __xv_sockbind(int, const sockaddr *, socklen_t);
  long __xv_connect(int, const sockaddr *, socklen_t);
  long __xv_listen(int, int);
  long __xv_accept(int, sockaddr *, socklen_t *);
  long __xv_sendto(int, const void *, unsigned long, int, const sockaddr *, socklen_t);
  long __xv_recvfrom(int, void *, unsigned long, int, sockaddr *, socklen_t *);
  long __xv_shutdown(int, int);
  long __xv_getsockname(int, sockaddr *, socklen_t *);
  long __xv_getpeername(int, sockaddr *, socklen_t *);
  long __xv_getsockopt(int, int, int, void *, socklen_t *);

  long
  read(int fd, void *buffer, unsigned long length) {
    return result(length > 0x7fffffffUL ? -EINVAL : __xv_read(fd, buffer, length));
  }

  long
  write(int fd, const void *buffer, unsigned long length) {
    return result(length > 0x7fffffffUL ? -EINVAL : __xv_write(fd, buffer, length));
  }

  int
  close(int fd) {
    return result(__xv_close(fd));
  }

  int
  poll(pollfd *items, unsigned long count, int timeout) {
    return result(count > 0x7fffffffUL ? -EINVAL : __xv_poll(items, count, timeout));
  }

  int
  fcntl(int fd, int command, ...) {
    int argument = 0;
    if (command == F_SETFL || command == F_SETFD || command == F_DUPFD ||
        command == F_DUPFD_CLOEXEC)
    {
      va_list args;
      va_start(args, command);
      argument = va_arg(args, int);
      va_end(args);
    }
    return result(__xv_fcntl(fd, command, argument));
  }

  int
  ftruncate(int fd, long bytes) {
    return result(bytes < 0 ? -EINVAL
                  : static_cast<unsigned long>(bytes) > 0xffffffffUL
                      ? -EFBIG
                      : __xv_ftruncate(fd, bytes));
  }

  long
  lseek(int fd, long offset, int origin) {
    return result(__xv_lseek(fd, offset, origin));
  }

  int
  socket(int domain, int type, int protocol) {
    return result(__xv_socket(domain, type, protocol));
  }

  int
  bind(int fd, const sockaddr *address, socklen_t length) {
    return result(__xv_sockbind(fd, address, length));
  }

  int
  sockbind(int fd, const sockaddr *address, socklen_t length) {
    return bind(fd, address, length);
  }

  int
  connect(int fd, const sockaddr *address, socklen_t length) {
    return result(__xv_connect(fd, address, length));
  }

  int
  listen(int fd, int backlog) {
    return result(__xv_listen(fd, backlog));
  }

  int
  accept(int fd, sockaddr *address, socklen_t *length) {
    return result(__xv_accept(fd, address, length));
  }

  long
  sendto(int fd, const void *data, unsigned long length, int flags,
         const sockaddr *address, socklen_t size) {
    return result(length > 0x7fffffffUL
                      ? -EMSGSIZE
                      : __xv_sendto(fd, data, length, flags, address, size));
  }

  long
  recvfrom(int fd, void *data, unsigned long length, int flags, sockaddr *address,
           socklen_t *size) {
    return result(length > 0x7fffffffUL
                      ? -EMSGSIZE
                      : __xv_recvfrom(fd, data, length, flags, address, size));
  }

  long
  send(int fd, const void *data, unsigned long length, int flags) {
    return sendto(fd, data, length, flags, nullptr, 0);
  }

  long
  recv(int fd, void *data, unsigned long length, int flags) {
    return recvfrom(fd, data, length, flags, nullptr, nullptr);
  }

  int
  shutdown(int fd, int how) {
    return result(__xv_shutdown(fd, how));
  }

  int
  getsockname(int fd, sockaddr *address, socklen_t *size) {
    return result(__xv_getsockname(fd, address, size));
  }

  int
  getpeername(int fd, sockaddr *address, socklen_t *size) {
    return result(__xv_getpeername(fd, address, size));
  }

  int
  getsockopt(int fd, int level, int option, void *value, socklen_t *size) {
    return result(__xv_getsockopt(fd, level, option, value, size));
  }
}

extern "C" void
_exit(int status) {
  extern int exit(int) __attribute__((noreturn));
  exit(status);
}

extern "C"
{
  long __xv_waitpid(int, int *, int);
  long __xv_sigsuspend(const void *);

  int
  waitpid(int pid, int *status, int flags) {
    return result(__xv_waitpid(pid, status, flags));
  }

  int
  sigsuspend(const sigset_t *mask) {
    return result(__xv_sigsuspend(mask));
  }
}
