#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/process/signals/signal_abi.h"
#include "user/runtime/user.h"
#include <sys/un.h>

static void
check(bool condition, const char *message) {
  if (!condition)
  {
    fprintf(2, "unixsockettest: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

static sockaddr_un
address(const char *path) {
  sockaddr_un value{};
  value.sun_family = AF_UNIX;
  check(strlen(path) < sizeof(value.sun_path), "test pathname fits");
  strcpy(value.sun_path, path);
  return value;
}

static int
bind_to(int fd, const char *path) {
  auto value = address(path);
  return bind(fd, reinterpret_cast<sockaddr *>(&value), 2 + strlen(path) + 1);
}

static int
connect_to(int fd, const char *path) {
  auto value = address(path);
  return connect(fd, reinterpret_cast<sockaddr *>(&value), 2 + strlen(path) + 1);
}

static int
listening(const char *path, int options = 0, int type = SOCK_STREAM) {
  int fd = socket(AF_UNIX, type | options, 0);
  check(fd >= 0 && bind_to(fd, path) == 0 && listen(fd, 1) == 0, "bind/listen");
  return fd;
}

static void
send_right(int socket, int file) {
  char byte = 'f';
  iovec vector{&byte, 1};
  alignas(unsigned long) char control[CMSG_SPACE(sizeof(int))]{};
  auto *header = reinterpret_cast<cmsghdr *>(control);
  header->cmsg_len = CMSG_LEN(sizeof(int));
  header->cmsg_level = SOL_SOCKET;
  header->cmsg_type = SCM_RIGHTS;
  memmove(CMSG_DATA(header), &file, sizeof(file));
  msghdr message{};
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  message.msg_control = control;
  message.msg_controllen = sizeof(control);
  check(sendmsg(socket, &message, MSG_NOSIGNAL) == 1, "send descriptor");
}

static int
receive_right(int socket) {
  char byte = 0;
  iovec vector{&byte, 1};
  alignas(unsigned long) char control[CMSG_SPACE(sizeof(int))]{};
  msghdr message{};
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  message.msg_control = control;
  message.msg_controllen = sizeof(control);
  check(recvmsg(socket, &message, MSG_CMSG_CLOEXEC) == 1 && byte == 'f' &&
            !(message.msg_flags & MSG_CTRUNC),
        "receive descriptor");
  auto *header = reinterpret_cast<cmsghdr *>(control);
  check(header->cmsg_level == SOL_SOCKET && header->cmsg_type == SCM_RIGHTS &&
            header->cmsg_len == CMSG_LEN(sizeof(int)),
        "rights header");
  int file;
  memmove(&file, CMSG_DATA(header), sizeof(file));
  return file;
}

static void
child_ok(int child) {
  int status = -1;
  check(waitpid(child, &status, 0) == child && status == 0, "child succeeded");
}

static void
caught(int) {
}

static void
basic(const char *path, const char *client_path) {
  unlink(path);
  unlink(client_path);
  int listener = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  check(listener >= 0 && fcntl(listener, F_GETFD) == FD_CLOEXEC &&
            (fcntl(listener, F_GETFL) & O_NONBLOCK),
        "atomic socket flags");
  auto invalid_address = address(path);
  check(bind(-1, reinterpret_cast<sockaddr *>(&invalid_address),
             sizeof(invalid_address)) == -1 &&
            errno == EBADF,
        "invalid socket descriptor");
  int plain = open("/dev/null", O_RDONLY);
  check(plain >= 0 && listen(plain, 1) == -1 && errno == ENOTSOCK,
        "ordinary descriptor is not a socket");
  close(plain);
  sockaddr_un value{};
  socklen_t length = sizeof(value);
  check(getsockname(listener, reinterpret_cast<sockaddr *>(&value), &length) == 0 &&
            length == 2 && value.sun_family == AF_UNIX,
        "unbound name");
  check(getpeername(listener, reinterpret_cast<sockaddr *>(&value), &length) == -1 &&
            errno == ENOTCONN,
        "unconnected peer name");
  check(listen(listener, 1) == -1 && errno == EINVAL, "listen requires bind");
  auto old_mask = umask(0027);
  check(bind_to(listener, path) == 0, "pathname bind");
  umask(old_mask);
  struct stat metadata{};
  check(stat(path, &metadata) == 0 && metadata.type == T_SOCKET &&
            (metadata.mode & 0777) == 0750,
        "real socket inode and umask");
  check(open(path, O_RDONLY) == -1, "socket node cannot open as ordinary file");
  int other = socket(AF_UNIX, SOCK_STREAM, 0);
  check(other >= 0 && bind_to(other, path) == -1 && errno == EADDRINUSE,
        "existing pathname is exclusive");
  close(other);
  check(bind_to(listener, path) == -1 && errno == EINVAL, "cannot bind twice");
  check(listen(listener, 1) == 0, "listen bound socket");
  check(accept(listener, nullptr, nullptr) == -1 && errno == EAGAIN,
        "empty nonblocking accept");
  pollfd event{listener, POLLIN, 0};
  check(poll(&event, 1, 0) == 0, "listener not readable before connect");
  int client = socket(AF_UNIX, SOCK_STREAM, 0);
  check(client >= 0 && bind_to(client, client_path) == 0 &&
            connect_to(client, path) == 0,
        "bound client connects");
  check(connect_to(client, path) == -1 && errno == EISCONN, "already connected");
  check(poll(&event, 1, 0) == 1 && (event.revents & POLLIN), "listener ready");
  length = 2;
  int accepted = accept(listener, reinterpret_cast<sockaddr *>(&value), &length);
  check(accepted >= 0 && length == 2 + strlen(client_path) + 1 &&
            value.sun_family == AF_UNIX,
        "accept reports peer length with truncated address");
  check(fcntl(accepted, F_GETFD) == 0 && !(fcntl(accepted, F_GETFL) & O_NONBLOCK),
        "accept creates blocking descriptor without CLOEXEC");
  length = sizeof(value);
  check(getsockname(accepted, reinterpret_cast<sockaddr *>(&value), &length) == 0 &&
            !strcmp(value.sun_path, path),
        "accepted local name");
  length = sizeof(value);
  check(getpeername(accepted, reinterpret_cast<sockaddr *>(&value), &length) == 0 &&
            !strcmp(value.sun_path, client_path),
        "accepted peer name");
  int type = 0;
  length = sizeof(type);
  check(getsockopt(client, SOL_SOCKET, SO_TYPE, &type, &length) == 0 &&
            type == SOCK_STREAM,
        "socket type option");
  check(write(client, "hello", 5) == 5, "connected write");
  char bytes[16]{};
  check(read(accepted, bytes, sizeof(bytes)) == 5 && !memcmp(bytes, "hello", 5),
        "connected read");
  int null = open("/dev/null", O_RDONLY);
  check(null >= 0, "open right source");
  send_right(client, null);
  int passed = receive_right(accepted);
  check(passed >= 0 && fcntl(passed, F_GETFD) == FD_CLOEXEC &&
            read(passed, bytes, 1) == 0,
        "passed descriptor remains functional");
  close(null);
  close(passed);
  check(shutdown(client, SHUT_WR) == 0 && read(accepted, bytes, sizeof(bytes)) == 0,
        "half close and EOF");
  check(write(accepted, "back", 4) == 4 && read(client, bytes, sizeof(bytes)) == 4,
        "reverse half remains usable");
  close(accepted);
  close(client);
  close(listener);
  check(stat(path, &metadata) == 0 && metadata.type == T_SOCKET,
        "close preserves pathname");
  client = socket(AF_UNIX, SOCK_STREAM, 0);
  check(client >= 0 && connect_to(client, path) == -1 && errno == ECONNREFUSED,
        "stale socket node refuses connection");
  close(client);
  check(unlink(path) == 0 && unlink(client_path) == 0, "unlink socket nodes");
}

static void
backlog_and_names(const char *path, const char *alias) {
  int listener = listening(path, SOCK_NONBLOCK);
  int first = socket(AF_UNIX, SOCK_STREAM, 0);
  int second = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
  check(first >= 0 && second >= 0 && connect_to(first, path) == 0,
        "fill listener backlog");
  check(connect_to(second, path) == -1 && errno == EAGAIN, "full backlog nonblocking");
  int peer = accept(listener, nullptr, nullptr);
  check(peer >= 0 && connect_to(second, path) == 0, "retry after accept frees backlog");
  int other_peer = accept(listener, nullptr, nullptr);
  check(other_peer >= 0, "accept second queued connection");
  close(first);
  close(second);
  close(peer);
  close(other_peer);
  check(symlink(path, alias) == 0, "socket alias symlink");
  first = socket(AF_UNIX, SOCK_STREAM, 0);
  check(first >= 0 && connect_to(first, alias) == 0, "connect follows symlink");
  peer = accept(listener, nullptr, nullptr);
  check(peer >= 0, "accept alias connection");
  sockaddr_un name{};
  socklen_t capacity = sizeof(name);
  check(getpeername(first, reinterpret_cast<sockaddr *>(&name), &capacity) == 0 &&
            !strcmp(name.sun_path, path),
        "peer reports bound name rather than symlink alias");
  check(unlink(alias) == 0 && rename(path, alias) == 0, "rename bound node");
  second = socket(AF_UNIX, SOCK_STREAM, 0);
  check(second >= 0 && connect_to(second, alias) == 0,
        "registry follows inode identity");
  other_peer = accept(listener, nullptr, nullptr);
  check(other_peer >= 0, "accept renamed connection");
  close(other_peer);
  close(second);
  check(unlink(alias) == 0, "unlink while listener remains open");
  int replacement = listening(alias, SOCK_NONBLOCK);
  second = socket(AF_UNIX, SOCK_STREAM, 0);
  check(second >= 0 && connect_to(second, alias) == 0, "rebind same name to new inode");
  other_peer = accept(replacement, nullptr, nullptr);
  check(other_peer >= 0 && accept(listener, nullptr, nullptr) == -1 && errno == EAGAIN,
        "new inode does not route to old listener");
  check(write(first, "old", 3) == 3, "old connection survives unlink/rebind");
  char bytes[8];
  check(read(peer, bytes, sizeof(bytes)) == 3, "old connection still receives");
  close(first);
  close(peer);
  close(second);
  close(other_peer);
  close(replacement);
  close(listener);
  unlink(alias);
}

static void
independently_started(const char *path) {
  int listener = listening(path, SOCK_CLOEXEC);
  int children[4];
  for (unsigned i = 0; i < 4; ++i)
  {
    children[i] = fork();
    check(children[i] >= 0, "fork independent exec client");
    if (!children[i])
    {
      char *arguments[] = {const_cast<char *>("unixsockettest"),
                           const_cast<char *>("client"), const_cast<char *>(path),
                           nullptr};
      exec("/usr/tests/unixsockettest", arguments);
      exit(2);
    }
  }
  for (unsigned i = 0; i < 4; ++i)
  {
    int connection = accept(listener, nullptr, nullptr);
    check(connection >= 0, "blocking accept wakes for concurrent exec clients");
    char bytes[16];
    check(read(connection, bytes, sizeof(bytes)) == 11 &&
              !memcmp(bytes, "exec-client", 11),
          "independent client sends");
    check(write(connection, "ok", 2) == 2, "reply to independent client");
    close(connection);
  }
  for (int child : children)
  {
    child_ok(child);
  }
  struct sigaction action{};
  action.sa_handler = caught;
  check(sigaction(SIGUSR1, &action, nullptr) == 0, "accept signal handler");
  int parent = getpid();
  int child = fork();
  check(child >= 0, "fork accept interrupter");
  if (!child)
  {
    close(listener);
    pause(1);
    sigsend(parent, SIGUSR1);
    exit(0);
  }
  check(accept(listener, nullptr, nullptr) == -1 && errno == EINTR,
        "signal interrupts accept");
  child_ok(child);
  close(listener);
  unlink(path);
}

static void
queue_closing(const char *path) {
  int listener = listening(path);
  int first = socket(AF_UNIX, SOCK_STREAM, 0);
  check(first >= 0 && connect_to(first, path) == 0, "queued connection");
  int parent = getpid();
  int child = fork();
  check(child >= 0, "fork blocked connector");
  if (!child)
  {
    close(listener);
    close(first);
    int next = socket(AF_UNIX, SOCK_STREAM, 0);
    check(next >= 0, "new blocking connector");
    check(sigsend(parent, SIGUSR1) == 0, "connector started");
    auto started = clockticks();
    check(connect_to(next, path) == -1 && errno == ECONNREFUSED,
          "listener close wakes blocked connector");
    check(clockticks() - started >= 500000, "connector actually waited for backlog");
    close(next);
    exit(0);
  }
  auto started = clockticks();
  while (clockticks() - started < 2000000)
  {
    pause(1);
  }
  close(listener);
  child_ok(child);
  char bytes[4];
  check(read(first, bytes, sizeof(bytes)) == 0,
        "unaccepted client receives EOF on listener close");
  close(first);
  unlink(path);
}

static void
permissions(const char *path) {
  int listener = listening(path, SOCK_CLOEXEC);
  check(chmod(path, 0600) == 0, "restrict socket node");
  int child = fork();
  check(child >= 0, "fork permission probe");
  if (!child)
  {
    check(setuid(1000) == 0, "drop identity");
    int client = socket(AF_UNIX, SOCK_STREAM, 0);
    check(client >= 0 && connect_to(client, path) == -1 && errno == EACCES,
          "connect checks directory search and socket write permission");
    close(client);
    exit(0);
  }
  child_ok(child);
  close(listener);
  unlink(path);
}

static void
descriptor_cycles(const char *path, const char *other) {
  for (unsigned iteration = 0; iteration < 120; ++iteration)
  {
    int listener = listening(path);
    int client = socket(AF_UNIX, SOCK_STREAM, 0);
    check(client >= 0 && connect_to(client, path) == 0, "pending cycle connection");
    send_right(client, listener);
    close(listener);
    close(client);
    unlink(path);
  }
  for (unsigned iteration = 0; iteration < 60; ++iteration)
  {
    int a = listening(path), b = listening(other);
    int left = socket(AF_UNIX, SOCK_STREAM, 0), right = socket(AF_UNIX, SOCK_STREAM, 0);
    check(left >= 0 && right >= 0 && connect_to(left, path) == 0 &&
              connect_to(right, other) == 0,
          "cross-listener cycle connections");
    send_right(left, b);
    send_right(right, a);
    close(a);
    close(b);
    close(left);
    close(right);
    unlink(path);
    unlink(other);
  }
}

// A provider cannot synchronously send Forget to itself from close().
// Transfer the last descriptor into the provider, then require it to observe
// the asynchronous Forget before acknowledging the close to its client.
static void
provider_reclamation() {
  const char *mountpoint = "/tmp/uds-fs";
  check(mkdir(mountpoint) == 0, "provider mountpoint");
  int control[2];
  check(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, control) == 0, "provider control pair");
  int child = fork();
  check(child >= 0, "provider fork");
  if (!child)
  {
    close(control[0]);
    int service = fsserve(mountpoint);
    check(service >= 0 && write(control[1], "r", 1) == 1, "provider mounted");
    bool exists = false, closing = false;
    int retained = -1;
    xv::userfs::Attributes root{1, 0, 0, 0, 0777, T_DIR, 2, 0, 0}, socket{};
    static xv::userfs::Message message;
    for (;;)
    {
      pollfd events[2]{{service, POLLIN, 0}, {control[1], POLLIN, 0}};
      check(poll(events, 2, -1) > 0, "provider poll");
      if (events[0].revents & POLLHUP)
      {
        break;
      }
      if (events[0].revents & POLLIN)
      {
        if (read(service, &message, sizeof(message)) == 0)
        {
          break;
        }
        bool acknowledge = false;
        message.result = 0;
        switch (message.operation)
        {
        case xv::userfs::Operation::Getattr:
          message.attributes = message.node == 1 ? root : socket;
          break;
        case xv::userfs::Operation::Lookup:
          if (message.node == 1 && exists && !strcmp(message.name, "s"))
          {
            message.attributes = socket;
          }
          else
          {
            message.result = -1;
          }
          break;
        case xv::userfs::Operation::Create:
          check(!exists && message.attributes.type == T_SOCKET,
                "provider socket creation");
          socket = message.attributes;
          socket.node = 2;
          socket.links = 1;
          message.attributes = socket;
          exists = true;
          break;
        case xv::userfs::Operation::Forget:
          acknowledge = closing && message.node == 2;
          break;
        case xv::userfs::Operation::Sync:
          break;
        default:
          message.result = -1;
          break;
        }
        check(write(service, &message, sizeof(message)) == sizeof(message),
              "provider reply");
        if (acknowledge)
        {
          check(write(control[1], "d", 1) == 1, "provider handled deferred Forget");
          closing = false;
        }
      }
      if (events[1].revents & POLLIN)
      {
        if (retained < 0)
        {
          retained = receive_right(control[1]);
          check(write(control[1], "k", 1) == 1, "provider owns transferred descriptor");
        }
        else
        {
          char command;
          check(read(control[1], &command, 1) == 1 && command == 'c',
                "provider close command");
          check(close(retained) == 0, "provider closes socket on its own filesystem");
          retained = -1;
          closing = true;
        }
      }
    }
    close(service);
    close(control[1]);
    exit(0);
  }
  close(control[1]);
  char byte = 0;
  check(read(control[0], &byte, 1) == 1 && byte == 'r', "provider ready");
  int listener = listening("/tmp/uds-fs/s");
  send_right(control[0], listener);
  check(read(control[0], &byte, 1) == 1 && byte == 'k', "provider retained descriptor");
  close(listener);
  check(write(control[0], "c", 1) == 1, "request provider close");
  pollfd ready{control[0], POLLIN, 0};
  check(poll(&ready, 1, 4000) == 1 && read(control[0], &byte, 1) == 1 && byte == 'd',
        "collector releases provider-owned inode without self-RPC deadlock");
  check(umount(mountpoint) == 0, "deferred inode does not pin unmount");
  child_ok(child);
  close(control[0]);
  check(unlink(mountpoint) == 0, "provider mountpoint cleanup");
}

static void
records(const char *path) {
  int listener = listening(path, 0, SOCK_SEQPACKET);
  int client = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  check(client >= 0 && connect_to(client, path) == 0, "named seqpacket connects");
  int peer = accept(listener, nullptr, nullptr);
  check(peer >= 0 && send(client, "first", 5, 0) == 5 &&
            send(client, "second", 6, 0) == 6,
        "send seqpacket records");
  char bytes[16];
  check(recv(peer, bytes, 2, MSG_TRUNC) == 5 &&
            recv(peer, bytes, sizeof(bytes), 0) == 6 && !memcmp(bytes, "second", 6),
        "named seqpacket preserves record boundaries");
  int mismatch = socket(AF_UNIX, SOCK_STREAM, 0);
  check(mismatch >= 0 && connect_to(mismatch, path) == -1 && errno == EPROTOTYPE,
        "connection type mismatch");
  close(mismatch);
  close(client);
  close(peer);
  close(listener);
  unlink(path);
}

extern "C" int
main(int argc, char **argv) {
  if (argc == 3 && !strcmp(argv[1], "client"))
  {
    int client = socket(AF_UNIX, SOCK_STREAM, 0);
    check(client >= 0 && connect_to(client, argv[2]) == 0,
          "exec client connects by path");
    check(write(client, "exec-client", 11) == 11, "exec client writes");
    char bytes[8];
    check(read(client, bytes, sizeof(bytes)) == 2 && !memcmp(bytes, "ok", 2),
          "exec client reply");
    close(client);
    exit(0);
  }
  const char *path = argc > 1 ? argv[1] : "/tmp/uds";
  char other[108];
  check(strlen(path) + 3 < sizeof(other), "secondary path fits");
  strcpy(other, path);
  strcpy(other + strlen(path), ".c");
  basic(path, other);
  backlog_and_names(path, other);
  independently_started(path);
  queue_closing(path);
  permissions(path);
  records(path);
  descriptor_cycles(path, other);
  provider_reclamation();
  printf("unixsockettest: PASS\n");
  exit(0);
}
