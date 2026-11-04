#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/process/signals/signal_abi.h"
#include "user/runtime/user.h"

static __attribute__((noinline)) void
check(bool condition, const char *message) {
  if (!condition)
  {
    fprintf(2, "posixipc: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

static long
put(int fd, const void *data, unsigned count, int flags = 0) {
  iovec vector{const_cast<void *>(data), count};
  msghdr message{};
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  return sendmsg(fd, &message, flags);
}

static long
get(int fd, void *data, unsigned count, int flags = 0, unsigned *out = nullptr) {
  iovec vector{data, count};
  msghdr message{};
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  long result = recvmsg(fd, &message, flags);
  if (out)
  {
    *out = message.msg_flags;
  }
  return result;
}

static __attribute__((noinline)) void
basic() {
  const int types[] = {SOCK_STREAM, SOCK_DGRAM, SOCK_SEQPACKET};
  for (int type : types)
  {
    int fds[2];
    check(socketpair(AF_UNIX, type | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds) == 0,
          "socketpair");
    check(fcntl(fds[0], F_GETFD) == FD_CLOEXEC, "atomic close-on-exec");
    check(fcntl(fds[0], F_SETFD, 0) == 0 && fcntl(fds[0], F_GETFD) == 0,
          "per-descriptor flags");
    char bytes[32]{};
    check(get(fds[0], bytes, sizeof(bytes)) == -1 && errno == EAGAIN,
          "POSIX errno for empty receive");
    pollfd poller{fds[0], POLLIN | POLLOUT, 0};
    check(poll(&poller, 1, 0) == 1 && (poller.revents & POLLOUT) &&
              !(poller.revents & POLLIN),
          "poll writable");
    check(put(fds[1], "hello", 5) == 5, "message send");
    check(get(fds[0], bytes, 2, MSG_PEEK) == 2 && !memcmp(bytes, "he", 2),
          "peek preserves queued data");
    unsigned output = 0;
    check(get(fds[0], bytes, 2, 0, &output) == 2 && !memcmp(bytes, "he", 2),
          "bounded receive");
    if (type == SOCK_STREAM)
    {
      check(get(fds[0], bytes, sizeof(bytes)) == 3 && !memcmp(bytes, "llo", 3),
            "stream preserves remainder");
    }
    else
    {
      check(output & MSG_TRUNC, "datagram truncation flag");
      check(get(fds[0], bytes, sizeof(bytes)) == -1 && errno == EAGAIN,
            "record boundary consumes truncated tail");
      check(put(fds[1], bytes, 0) == 0 && get(fds[0], bytes, sizeof(bytes)) == 0,
            "zero length record");
    }
    check(shutdown(fds[1], SHUT_WR) == 0 && get(fds[0], bytes, sizeof(bytes)) == 0,
          "half close and EOF");
    check(put(fds[0], "back", 4, MSG_NOSIGNAL) == 4, "reverse half remains open");
    check(get(fds[1], bytes, sizeof(bytes)) == 4, "reverse half receive");
    close(fds[1]);
    check(put(fds[0], "x", 1, MSG_NOSIGNAL) == -1 && errno == EPIPE,
          "closed peer and EPIPE");
    close(fds[0]);
  }
}

static __attribute__((noinline)) void
descriptors() {
  int pair[2];
  check(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, pair) == 0, "rights pair");
  int fd = open("/tmp/ipc-right", O_CREATE | O_TRUNC | O_RDWR);
  check(fd >= 0 && write(fd, "data", 4) == 4 && lseek(fd, 0, SEEK_SET) == 0,
        "rights fixture");
  alignas(unsigned long) unsigned char control[CMSG_SPACE(sizeof(int))]{};
  auto *header = reinterpret_cast<cmsghdr *>(control);
  header->cmsg_len = CMSG_LEN(sizeof(int));
  header->cmsg_level = SOL_SOCKET;
  header->cmsg_type = SCM_RIGHTS;
  memmove(CMSG_DATA(header), &fd, sizeof(fd));
  char data = '!';
  iovec vector{&data, 1};
  msghdr message{};
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  message.msg_control = control;
  message.msg_controllen = sizeof(control);
  int sender = fork();
  check(sender >= 0, "SCM_RIGHTS sender fork");
  if (!sender)
  {
    close(pair[1]);
    check(sendmsg(pair[0], &message, 0) == 1, "SCM_RIGHTS send");
    close(fd);
    close(pair[0]);
    exit(0);
  }
  close(pair[0]);
  close(fd);
  memset(control, 0, sizeof(control));
  check(recvmsg(pair[1], &message, MSG_CMSG_CLOEXEC) == 1 &&
            !(message.msg_flags & MSG_CTRUNC),
        "SCM_RIGHTS receive");
  header = CMSG_FIRSTHDR(&message);
  check(header && header->cmsg_type == SCM_RIGHTS &&
            header->cmsg_len == CMSG_LEN(sizeof(int)),
        "control message shape");
  memmove(&fd, CMSG_DATA(header), sizeof(fd));
  char bytes[4];
  check(fcntl(fd, F_GETFD) == FD_CLOEXEC && read(fd, bytes, 4) == 4 &&
            !memcmp(bytes, "data", 4),
        "received reference survives sender close");
  close(fd);
  close(pair[1]);
  int status;
  check(wait(&status) == sender && status == 0, "SCM_RIGHTS worker completion");
  unlink("/tmp/ipc-right");
}

static __attribute__((noinline)) void
shared_memory() {
  const char *name = "/posixipc-test";
  shm_unlink(name);
  int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
  check(fd >= 0 && fcntl(fd, F_GETFD) == FD_CLOEXEC, "shm_open CLOEXEC");
  check(shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600) == -1 && errno == EEXIST,
        "exclusive shared name");
  struct stat status{};
  check(fstat(fd, &status) == 0 && status.size == 0, "new shared object is empty");
  check(ftruncate(fd, 12288) == 0, "shared resize");
  auto *shared = static_cast<unsigned char *>(
      mmap(nullptr, 12288, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  check(shared != MAP_FAILED && shared[0] == 0 && shared[12287] == 0,
        "zero filled mapping");
  shared[0] = 23;
  auto *private_copy = static_cast<unsigned char *>(
      mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0));
  check(private_copy != MAP_FAILED && private_copy[0] == 23,
        "private mapping initial content");
  int child = fork();
  check(child >= 0, "shared mapping fork");
  if (!child)
  {
    shared[4096] = 71;
    private_copy[0] = 99;
    exit(0);
  }
  int code = -1;
  check(wait(&code) == child && code == 0 && shared[4096] == 71 &&
            private_copy[0] == 23 && shared[0] == 23,
        "fork shares shared pages and isolates private pages");
  int readonly = shm_open(name, O_RDONLY, 0);
  check(readonly >= 0 && ftruncate(readonly, 4096) == -1 && errno == EBADF,
        "read only resize rejected");
  check(mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, readonly, 0) ==
                MAP_FAILED &&
            errno == EACCES,
        "read only shared write mapping rejected");
  close(readonly);
  check(shm_unlink(name) == 0 && shm_open(name, O_RDWR, 0) == -1 && errno == ENOENT,
        "unlink removes name");
  close(fd);
  check(shared[4096] == 71, "mapping survives unlink and close");
  check(munmap(shared + 4096, 4096) == 0, "partial unmap");
  check(shared[0] == 23 && shared[8192] == 0, "partial unmap keeps both sides");
  check(munmap(shared, 4096) == 0 && munmap(shared + 8192, 4096) == 0,
        "split mapping cleanup");
  check(munmap(private_copy, 4096) == 0, "private mapping cleanup");
}

static __attribute__((noinline)) void
errors_and_rights() {
  int pair[2];
  check(socketpair(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0, pair) == 0, "error fixture");
  char byte = 'a';
  for (unsigned i = 0; i < 16; ++i)
  {
    check(put(pair[0], &byte, 1) == 1, "queue capacity");
  }
  check(put(pair[0], &byte, 1) == -1 && errno == EAGAIN, "bounded queue backpressure");
  for (unsigned i = 0; i < 16; ++i)
  {
    check(get(pair[1], &byte, 1) == 1, "queue drain");
  }
  check(put(pair[0], reinterpret_cast<void *>(~0UL), 1) == -1 && errno == EFAULT,
        "send copy fault");
  check(put(pair[0], &byte, 1) == 1, "send after copy fault");
  check(get(pair[1], reinterpret_cast<void *>(~0UL), 1) == -1 && errno == EFAULT,
        "receive copy fault");
  check(get(pair[1], &byte, 1) == 1, "receive fault preserves message");
  close(pair[0]);
  close(pair[1]);
  check(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0, "partial rights fixture");
  int fd = open("/dev/null", O_RDWR);
  check(fd >= 0, "rights null descriptor");
  alignas(unsigned long) unsigned char control[CMSG_SPACE(sizeof(int))]{};
  auto *header = reinterpret_cast<cmsghdr *>(control);
  header->cmsg_len = CMSG_LEN(sizeof(int));
  header->cmsg_level = SOL_SOCKET;
  header->cmsg_type = SCM_RIGHTS;
  memmove(CMSG_DATA(header), &fd, sizeof(fd));
  char bytes[4] = {'a', 'b', 'c', 'd'};
  iovec vector{bytes, sizeof(bytes)};
  msghdr message{};
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  message.msg_control = control;
  message.msg_controllen = sizeof(control);
  check(sendmsg(pair[0], &message, 0) == 4, "partial stream rights send");
  vector.iov_len = 1;
  message.msg_control = nullptr;
  message.msg_controllen = 0;
  check(recvmsg(pair[1], &message, 0) == 1 && (message.msg_flags & MSG_CTRUNC),
        "missing ancillary capacity truncates rights");
  vector.iov_len = 3;
  message.msg_control = control;
  message.msg_controllen = sizeof(control);
  check(recvmsg(pair[1], &message, 0) == 3 && message.msg_controllen == 0 &&
            !(message.msg_flags & MSG_CTRUNC),
        "stream rights delivered once");
  close(fd);
  close(pair[0]);
  close(pair[1]);
}

static volatile int pipe_signal;

static __attribute__((noinline)) void
caught_pipe(int) {
  pipe_signal = 1;
}

static __attribute__((noinline)) void
blocking_and_signals() {
  int pair[2];
  check(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0, "blocking stream pair");
  int child = fork();
  check(child >= 0, "stream producer fork");
  if (!child)
  {
    close(pair[0]);
    static char bytes[4096];
    memset(bytes, 'x', sizeof(bytes));
    for (unsigned i = 0; i < 32; ++i)
    {
      check(write(pair[1], bytes, sizeof(bytes)) == sizeof(bytes), "blocking producer");
    }
    close(pair[1]);
    exit(0);
  }
  close(pair[1]);
  unsigned done = 0;
  static char bytes[1777];
  for (;;)
  {
    long count = recv(pair[0], bytes, sizeof(bytes), MSG_WAITALL);
    check(count >= 0, "blocking consumer");
    if (!count)
    {
      break;
    }
    for (long i = 0; i < count; ++i)
    {
      check(bytes[i] == 'x', "stream content");
    }
    done += count;
  }
  int status;
  check(done == 32 * 4096 && wait(&status) == child && status == 0,
        "stream backpressure and EOF");
  close(pair[0]);
  check(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0, "signal pair");
  close(pair[1]);
  auto previous = signal(SIGPIPE, caught_pipe);
  pipe_signal = 0;
  check(send(pair[0], "x", 1, 0) == -1 && errno == EPIPE && pipe_signal,
        "SIGPIPE delivered outside locks");
  pipe_signal = 0;
  check(send(pair[0], "x", 1, MSG_NOSIGNAL) == -1 && errno == EPIPE && !pipe_signal,
        "MSG_NOSIGNAL");
  signal(SIGPIPE, previous);
  close(pair[0]);
  child = fork();
  check(child >= 0, "default SIGPIPE worker");
  if (!child)
  {
    int pipe_fds[2];
    signal(SIGPIPE, SIG_DFL);
    check(pipe(pipe_fds) == 0, "default SIGPIPE pipe");
    close(pipe_fds[0]);
    write(pipe_fds[1], "x", 1);
    exit(99);
  }
  check(wait(&status) == child && status == SIGNAL_EXIT_STATUS(SIGPIPE),
        "default SIGPIPE terminates writer");
}

static void
caught_interrupt(int) {
}

static __attribute__((noinline)) void
interrupted_receive(bool restart) {
  int pair[2], ready[2];
  check(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0 && pipe(ready) == 0,
        "interrupt fixture");
  int child = fork();
  check(child >= 0, "interrupt child fork");
  if (!child)
  {
    close(ready[0]);
    close(pair[1]);
    struct sigaction action{};
    action.sa_handler = caught_interrupt;
    action.sa_flags = restart ? SA_RESTART : 0;
    check(sigaction(SIGUSR1, &action, nullptr) == 0 && write(ready[1], "r", 1) == 1,
          "interrupt worker ready");
    char byte;
    int received = get(pair[0], &byte, 1);
    check(restart ? received == 1 && byte == 'x' : received == -1 && errno == EINTR,
          "recvmsg interruption and SA_RESTART");
    exit(0);
  }
  close(ready[1]);
  close(pair[0]);
  char byte;
  check(read(ready[0], &byte, 1) == 1, "interrupt worker started");
  pause(2);
  check(sigsend(child, SIGUSR1) == 0, "interrupt receive signal");
  if (restart)
  {
    check(put(pair[1], "x", 1) == 1, "restart receive delivery");
  }
  int code;
  check(wait(&code) == child && code == 0, "interrupted receive worker completion");
  close(ready[0]);
  close(pair[1]);
}

static __attribute__((noinline)) void
exec_flags() {
  int pair[2];
  check(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, pair) == 0, "exec pair");
  int child = fork();
  check(child >= 0, "exec worker fork");
  if (!child)
  {
    check(fcntl(pair[1], F_SETFD, 0) == 0, "clear one close-on-exec flag");
    // Stable fd numbers avoid importing formatting helpers into the test.
    char closed[4] = {char('0' + pair[0] / 10), char('0' + pair[0] % 10), 0};
    char retained[4] = {char('0' + pair[1] / 10), char('0' + pair[1] % 10), 0};
    char *args[] = {const_cast<char *>("posixipc"), const_cast<char *>("exec"), closed,
                    retained, nullptr};
    exec("/usr/tests/posixipc", args);
    exit(1);
  }
  int code;
  check(wait(&code) == child && code == 0,
        "CLOEXEC closes selected descriptor after exec");
  close(pair[0]);
  close(pair[1]);
}

static __attribute__((noinline)) void
cycles() {
  for (unsigned iteration = 0; iteration < 180; ++iteration)
  {
    int pair[2];
    check(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, pair) == 0, "cycle pair allocation");
    alignas(unsigned long) unsigned char control[CMSG_SPACE(sizeof(int))]{};
    auto *header = reinterpret_cast<cmsghdr *>(control);
    header->cmsg_len = CMSG_LEN(sizeof(int));
    header->cmsg_level = SOL_SOCKET;
    header->cmsg_type = SCM_RIGHTS;
    memmove(CMSG_DATA(header), &pair[1], sizeof(int));
    char byte = 'c';
    iovec vector{&byte, 1};
    msghdr message{};
    message.msg_iov = &vector;
    message.msg_iovlen = 1;
    message.msg_control = control;
    message.msg_controllen = sizeof(control);
    check(sendmsg(pair[0], &message, MSG_NOSIGNAL) == 1, "self cycle rights");
    close(pair[0]);
    close(pair[1]);
  }
}

extern "C" int
main(int argc, char **argv) {
  if (argc == 4 && !strcmp(argv[1], "exec"))
  {
    check(fcntl(atoi(argv[2]), F_GETFD) == -1 && errno == EBADF, "exec closed flag");
    check(fcntl(atoi(argv[3]), F_GETFD) == 0, "exec retained flag");
    return 0;
  }
  basic();
  descriptors();
  shared_memory();
  cycles();
  errors_and_rights();
  blocking_and_signals();
  interrupted_receive(false);
  interrupted_receive(true);
  exec_flags();
  printf(
      "posixipc: PASS: Unix sockets, shared memory, fork, exec, signals, descriptor passing and cycle collection\n");
  return 0;
}
