#include "kernel/support/types.h"
#include "kernel/fs/poll.h"
#include "kernel/process/signals/signal_abi.h"
#include "user/runtime/user.h"
#include <mqueue.h>

static void
check(bool condition, const char *message) {
  if (!condition)
  {
    fprintf(2, "mqtest: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

static timespec
after(unsigned milliseconds) {
  timespec time;
  check(clock_gettime(CLOCK_REALTIME, &time) == 0, "read realtime");
  time.tv_nsec += long(milliseconds) * 1000000;
  time.tv_sec += time.tv_nsec / 1000000000;
  time.tv_nsec %= 1000000000;
  return time;
}

static void
child_ok(int child) {
  int status = -1;
  check(waitpid(child, &status, 0) == child && status == 0, "child succeeded");
}

static void
signal_handler(int) {
}

extern "C" int
main(int argc, char **argv) {
  if (argc == 3 && !strcmp(argv[1], "sender"))
  {
    int fd = mq_open(argv[2], O_WRONLY);
    check(fd >= 0, "independent exec open");
    check(mq_send(fd, "exec-message", 12, 13) == 0, "independent exec send");
    mq_close(fd);
    exit(0);
  }
  const char *name = "/mq-functional";
  mq_unlink(name);
  mq_attr create{};
  create.mq_maxmsg = 3;
  create.mq_msgsize = 32;
  int fd = mq_open(name, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0600, &create);
  check(fd >= 0, "create named queue");
  check(fcntl(fd, F_GETFD) == FD_CLOEXEC, "close on exec");
  check(mq_open(name, O_CREAT | O_EXCL | O_RDWR, 0600, &create) == -1 &&
            errno == EEXIST,
        "exclusive create");
  char data[32]{};
  unsigned priority = 0;
  check(mq_receive(fd, data, sizeof(data), &priority) == -1 && errno == EAGAIN,
        "empty nonblocking receive");
  check(mq_send(fd, "low", 3, 1) == 0 && mq_send(fd, "first", 5, 9) == 0 &&
            mq_send(fd, "second", 6, 9) == 0,
        "queue messages");
  check(mq_send(fd, "full", 4, 0) == -1 && errno == EAGAIN, "full nonblocking send");
  mq_attr attributes{};
  check(mq_getattr(fd, &attributes) == 0 && attributes.mq_curmsgs == 3 &&
            attributes.mq_maxmsg == 3 && attributes.mq_msgsize == 32 &&
            attributes.mq_flags == O_NONBLOCK,
        "queue attributes");
  check(mq_receive(fd, data, 1, &priority) == -1 && errno == EMSGSIZE,
        "receive capacity uses configured message size");
  check(mq_receive(fd, reinterpret_cast<char *>(~0UL), sizeof(data), &priority) == -1 &&
            errno == EFAULT,
        "invalid receive buffer");
  check(mq_receive(fd, data, sizeof(data), &priority) == 5 && priority == 9 &&
            !memcmp(data, "first", 5),
        "highest priority first, preserve on fault");
  check(mq_receive(fd, data, sizeof(data), &priority) == 6 && priority == 9 &&
            !memcmp(data, "second", 6),
        "FIFO for equal priorities");
  check(mq_receive(fd, data, sizeof(data), &priority) == 3 && priority == 1,
        "lower priority last");
  check(mq_send(fd, data, 33, 0) == -1 && errno == EMSGSIZE, "oversized send");
  check(mq_send(fd, data, 1, MQ_PRIO_MAX) == -1 && errno == EINVAL, "priority bounds");
  int reader = mq_open(name, O_RDONLY | O_NONBLOCK);
  int writer = mq_open(name, O_WRONLY | O_NONBLOCK);
  check(reader >= 0 && writer >= 0, "separate open descriptions");
  check(mq_send(reader, data, 0, 0) == -1 && errno == EBADF &&
            mq_receive(writer, data, sizeof(data), nullptr) == -1 && errno == EBADF,
        "access rights");
  pollfd event{reader, POLLIN, 0};
  check(poll(&event, 1, 0) == 0, "empty poll");
  check(mq_send(writer, "", 0, 0) == 0 && poll(&event, 1, 0) == 1 &&
            (event.revents & POLLIN),
        "zero length message and readable poll");
  check(mq_receive(reader, data, sizeof(data), nullptr) == 0, "zero length receive");
  mq_attr blocking{}, previous{};
  check(mq_setattr(fd, &blocking, &previous) == 0 && previous.mq_flags == O_NONBLOCK,
        "switch to blocking");
  int duplicate = dup(fd);
  check(duplicate >= 0 && mq_getattr(duplicate, &attributes) == 0 &&
            attributes.mq_flags == 0,
        "dup shares open description flags");
  auto start = clockticks();
  auto deadline = after(80);
  check(mq_timedreceive(fd, data, sizeof(data), nullptr, &deadline) == -1 &&
            errno == ETIMEDOUT,
        "absolute receive timeout");
  auto elapsed = clockticks() - start;
  check(elapsed >= 500000 && elapsed < 20000000, "timed receive actually waits");
  timespec expired{0, 0};
  check(mq_send(fd, "ready", 5, 3) == 0 &&
            mq_timedreceive(fd, data, sizeof(data), nullptr, &expired) == 5,
        "expired timeout still allows immediately available receive");
  for (int i = 0; i < 3; ++i)
  {
    check(mq_send(fd, "x", 1, 0) == 0, "fill for timeout");
  }
  deadline = after(60);
  check(mq_timedsend(fd, "x", 1, 0, &deadline) == -1 && errno == ETIMEDOUT,
        "absolute send timeout");
  for (int i = 0; i < 3; ++i)
  {
    check(mq_receive(fd, data, sizeof(data), nullptr) == 1, "drain");
  }
  timespec invalid{0, 1000000000};
  check(mq_timedreceive(fd, data, sizeof(data), nullptr, &invalid) == -1 &&
            errno == EINVAL,
        "invalid nanoseconds");
  int child = fork();
  check(child >= 0, "fork sender");
  if (!child)
  {
    char *args[] = {const_cast<char *>("mqtest"), const_cast<char *>("sender"),
                    const_cast<char *>(name), nullptr};
    exec("/usr/tests/mqtest", args);
    exit(2);
  }
  check(mq_receive(fd, data, sizeof(data), &priority) == 12 && priority == 13 &&
            !memcmp(data, "exec-message", 12),
        "independent process wakes blocked receiver");
  child_ok(child);
  child = fork();
  check(child >= 0, "fork full queue consumer");
  if (!child)
  {
    pause(1);
    check(mq_receive(fd, data, sizeof(data), nullptr) >= 0, "consumer receives");
    exit(0);
  }
  for (int i = 0; i < 4; ++i)
  {
    check(mq_send(fd, "x", 1, 0) == 0, "blocked sender wakes");
  }
  child_ok(child);
  for (int i = 0; i < 3; ++i)
  {
    check(mq_receive(fd, data, sizeof(data), nullptr) == 1, "drain after wake");
  }
  struct sigaction action{};
  action.sa_handler = signal_handler;
  check(sigaction(SIGUSR1, &action, nullptr) == 0, "install signal handler");
  int parent = getpid();
  child = fork();
  check(child >= 0, "fork interrupter");
  if (!child)
  {
    pause(1);
    sigsend(parent, SIGUSR1);
    exit(0);
  }
  check(mq_receive(fd, data, sizeof(data), nullptr) == -1 && errno == EINTR,
        "signal interrupts blocked receiver");
  child_ok(child);
  action.sa_flags = SA_RESTART;
  check(sigaction(SIGUSR1, &action, nullptr) == 0, "install restart handler");
  child = fork();
  check(child >= 0, "fork restart sender");
  if (!child)
  {
    pause(1);
    sigsend(parent, SIGUSR1);
    pause(1);
    check(mq_send(fd, "restart", 7, 2) == 0, "send after restart signal");
    exit(0);
  }
  check(mq_receive(fd, data, sizeof(data), nullptr) == 7 && !memcmp(data, "restart", 7),
        "SA_RESTART retries interrupted receive");
  child_ok(child);
  child = fork();
  check(child >= 0, "fork permission check");
  if (!child)
  {
    check(setuid(1000) == 0, "drop identity");
    check(mq_open(name, O_RDONLY) == -1 && errno == EACCES, "queue permissions");
    exit(0);
  }
  child_ok(child);
  check(mq_unlink(name) == 0 && mq_open(name, O_RDONLY) == -1 && errno == ENOENT,
        "unlink removes name");
  int replacement =
      mq_open(name, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0600, &create);
  check(replacement >= 0 && mq_send(fd, "old", 3, 0) == 0 &&
            mq_receive(replacement, data, sizeof(data), nullptr) == -1 &&
            errno == EAGAIN,
        "unlinked handles survive independently of replacement");
  check(mq_receive(reader, data, sizeof(data), nullptr) == 3,
        "old queue remains usable");
  mq_unlink(name);
  mq_close(replacement);
  mq_close(fd);
  mq_close(reader);
  mq_close(writer);
  mq_close(duplicate);
  check(mq_getattr(fd, &attributes) == -1 && errno == EBADF, "closed descriptor");
  check(mq_open("invalid/name", O_RDONLY) == -1 && errno == EINVAL,
        "queue name syntax");
  create.mq_maxmsg = 0;
  check(mq_open("/mq-invalid", O_CREAT | O_RDWR, 0600, &create) == -1 &&
            errno == EINVAL,
        "queue limits");
  printf("mqtest: PASS\n");
  exit(0);
}
