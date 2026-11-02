#include "kernel/support/types.h"
#include "kernel/io/fs_mount.h"
#include "kernel/io/fs_resources.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/io_status.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"

namespace
{
void check(bool okay, const char *message)
{
  if (!okay)
  {
    printf("fsmgrboottest: FAIL: %s\n", message);
    exit(1);
  }
}
int client(int endpoint)
{
  int pid = fork();
  check(pid >= 0, "fork request client");
  if (!pid)
  {
    xv::fsmount::Request request;
    check(read(endpoint, &request, sizeof(request)) < 0, "inherited endpoint cannot impersonate server");
    close(endpoint);
    check(mount("unknown-plugin", "/not-mounted") < 0, "negative result or disconnect reaches client");
    exit(0);
  }
  return pid;
}
void reap(int pid)
{
  int status;
  check(waitpid(pid, &status, 0) == pid && status == 0, "request client exits cleanly");
}
}
extern "C" int main()
{
  using namespace xv::fsservice;
  using xv::fsmount::Request;
  check(getpid() == 1, "bootstrap process");
  check(ioctl(4, StorageIoctl, 0) < 0 && ioctl(4, StorageIoctl, 2) < 0,
        "boot disk lease and invalid storage index");
  int endpoint = ioctl(4, ManagerIoctl, 0);
  check(endpoint >= 0 && ioctl(4, ManagerIoctl, 0) < 0, "one active manager");
  check(fcntl(endpoint, F_SETFL, O_NONBLOCK) == 0, "nonblocking manager endpoint");
  Request request;
  check(read(endpoint, &request, sizeof(request)) == -EAGAIN, "idle nonblocking receive");
  check(fcntl(endpoint, F_SETFL, 0) == 0, "restore blocking endpoint");
  int pid = client(endpoint);
  pollfd watched{endpoint, POLLIN, 0};
  check(poll(&watched, 1, 10000) == 1 && (watched.revents & POLLIN), "request wakes manager poll");
  check(read(endpoint, &request, sizeof(request) - 1) < 0 &&
        read(endpoint, reinterpret_cast<void *>(~0UL), sizeof(request)) < 0,
        "bad receive does not consume request");
  check(read(endpoint, &request, sizeof(request)) == sizeof(request) &&
        request.pid == pid && !strcmp(request.type, "unknown-plugin") &&
        !strcmp(request.path, "/not-mounted"), "generic request and original caller identity");
  check(ioctl(4, ManagerIoctl, 0) < 0, "duplicate manager claim rejects even with a pending client");
  check(mount("recursive-plugin", "/not-mounted") < 0, "manager recursion rejected without waiting");
  auto valid = request;
  ++request.id;
  check(write(endpoint, &request, sizeof(request)) < 0, "reject stale response identity");
  request = valid;
  ++request.version;
  check(write(endpoint, &request, sizeof(request)) < 0, "reject incompatible response");
  request = valid;
  request.result = 17;
  check(write(endpoint, &request, sizeof(request)) < 0, "reject invalid response status");
  request = valid;
  request.result = -1;
  check(write(endpoint, &request, sizeof(request)) == sizeof(request), "complete failed mount request");
  reap(pid);
  pid = client(endpoint);
  check(read(endpoint, &request, sizeof(request)) == sizeof(request), "next request");
  close(endpoint);
  reap(pid);
  endpoint = ioctl(4, ManagerIoctl, 0);
  check(endpoint >= 0, "register replacement after disconnect");
  pid = client(endpoint);
  check(read(endpoint, &request, sizeof(request)) == sizeof(request), "replacement channel request");
  close(endpoint);
  reap(pid);
  printf("fsmgrboottest: PASS: generic mount RPC, validation, recursion and disconnect\n");
  for (;;) pause(1);
}
