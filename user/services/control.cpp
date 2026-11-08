#include "protocol.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/stat.h"

namespace services
{
const char *
phase_name(Phase phase) {
  switch (phase)
  {
  case Phase::Stopped:
    return "stopped";
  case Phase::Waiting:
    return "waiting";
  case Phase::Starting:
    return "starting";
  case Phase::Running:
    return "running";
  case Phase::Stopping:
    return "stopping";
  case Phase::Backoff:
    return "backoff";
  case Phase::Complete:
    return "complete";
  case Phase::Failed:
    return "failed";
  case Phase::Unavailable:
    return "unavailable";
  }
  return "unknown";
}

namespace
{
bool
address(const char *path, sockaddr_un &out) {
  if (!path || path[0] != '/' || strlen(path) >= sizeof(out.sun_path))
  {
    return false;
  }
  out.sun_family = AF_UNIX;
  strcpy(out.sun_path, path);
  return true;
}

bool
wait_io(int fd, short events, int timeout) {
  pollfd watched{fd, events, 0};
  int result;
  do
  {
    result = poll(&watched, 1, timeout);
  } while (result < 0 && errno == EINTR);
  if (result == 0) errno = ETIMEDOUT;
  else if (result > 0 && !(watched.revents & events)) errno = ECONNRESET;
  return result > 0 && (watched.revents & events);
}
} // namespace

bool
exchange(const char *path, const Request &request, Reply &out) {
  sockaddr_un target{};
  if (!address(path, target))
  {
    return false;
  }
  int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
  if (fd < 0)
  {
    return false;
  }
  bool okay = connect(fd, reinterpret_cast<sockaddr *>(&target), sizeof(target)) == 0 &&
              write(fd, &request, sizeof(request)) == sizeof(request) &&
              wait_io(fd, POLLIN, 5000);
  if (okay) {
    int count = read(fd, &out, sizeof(out));
    okay = count == sizeof(out) && out.version == Version;
    if (!okay && count >= 0) errno = count == 0 ? ECONNRESET : EIO;
  }
  int error = errno;
  close(fd);
  errno = error;
  return okay;
}

int
listen(const char *path) {
  sockaddr_un target{};
  if (!address(path, target))
  {
    return -1;
  }
  int fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);
  if (fd < 0)
  {
    return -1;
  }
  // A live manager owns its pathname. Remove only a root-owned stale socket.
  if (bind(fd, reinterpret_cast<sockaddr *>(&target), sizeof(target)) < 0)
  {
    int probe = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);
    int connected = probe >= 0 ? connect(probe, reinterpret_cast<sockaddr *>(&target),
                                         sizeof(target))
                               : -1;
    bool live = connected == 0 || (errno != ECONNREFUSED && errno != ENOENT);
    if (probe >= 0)
    {
      close(probe);
    }
    struct stat st{};
    if (live || stat(path, &st) < 0 || st.type != T_SOCKET || st.uid != 0 ||
        unlink(path) < 0 ||
        bind(fd, reinterpret_cast<sockaddr *>(&target), sizeof(target)) < 0)
    {
      close(fd);
      return -1;
    }
  }
  if (chmod(path, 0600) < 0 || ::listen(fd, 8) < 0)
  {
    close(fd);
    unlink(path);
    return -1;
  }
  return fd;
}

int
accept_request(int listener, Request &request) {
  int fd = accept(listener, nullptr, nullptr);
  if (fd < 0)
  {
    return -1;
  }
  fcntl(fd, F_SETFL, O_NONBLOCK);
  if (!wait_io(fd, POLLIN, 1000) ||
      read(fd, &request, sizeof(request)) != sizeof(request) ||
      request.version != Version || request.name[31] || request.path[127])
  {
    close(fd);
    return -1;
  }
  return fd;
}

bool
reply(int fd, const Reply &response) {
  bool okay = write(fd, &response, sizeof(response)) == sizeof(response);
  close(fd);
  return okay;
}

bool
register_external(const char *name, int pid) {
  if (!name || strlen(name) >= 32 || pid < 0)
  {
    return false;
  }
  Request request;
  request.command = Command::Register;
  request.pid = pid;
  strcpy(request.name, name);
  Reply response;
  return exchange(DefaultSocket, request, response) && response.result == 0;
}
} // namespace services
