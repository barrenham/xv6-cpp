#include "user/services/protocol.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/process/wait/wait.h"

namespace
{
const char *Socket = "/tmp/service-test.sock", *Config = "/tmp/service-test.conf";

void
check(bool okay, const char *why) {
  if (!okay)
  {
    fprintf(2, "servicetest: FAIL: %s errno=%d\n", why, errno);
    exit(1);
  }
}

void
text(const char *path, const char *value) {
  int fd = open(path, O_CREATE | O_TRUNC | O_WRONLY);
  check(fd >= 0 && write(fd, value, strlen(value)) == int(strlen(value)),
        "write configuration");
  close(fd);
}

services::Reply
request(services::Command command, const char *name) {
  services::Request query;
  query.command = command;
  if (name)
  {
    strcpy(command == services::Command::Load ? query.path : query.name, name);
  }
  services::Reply response;
  check(services::exchange(Socket, query, response), "control transport");
  return response;
}

services::Reply
phase(const char *name, services::Phase wanted) {
  auto deadline = clockticks() + 100000000;
  for (;;)
  {
    auto state = request(services::Command::Status, name);
    check(state.result == 0, "named service exists");
    if (state.phase == wanted)
    {
      return state;
    }
    check(clockticks() < deadline, "service phase deadline");
    poll(nullptr, 0, 20);
  }
}

void
completed(int pid) {
  int status;
  check(waitpid(pid, &status, 0) == pid && status == 0, "child completed");
}

bool
logged(const char *path, const char *needle) {
  auto deadline = clockticks() + 20000000;
  do
  {
    char bytes[256]{};
    int fd = open(path, O_RDONLY);
    int count = fd >= 0 ? read(fd, bytes, sizeof(bytes) - 1) : -1;
    if (fd >= 0)
    {
      close(fd);
    }
    unsigned length = strlen(needle);
    for (int i = 0; count > 0 && i + length <= unsigned(count); ++i)
    {
      if (!memcmp(bytes + i, needle, length))
      {
        return true;
      }
    }
    poll(nullptr, 0, 20);
  } while (clockticks() < deadline);
  return false;
}
} // namespace

extern "C" int
main() {
  services::Request core;
  core.command = services::Command::Status;
  strcpy(core.name, "rootfs");
  services::Reply root;
  check(services::exchange(services::DefaultSocket, core, root) && root.result == 0 &&
            root.external && root.phase == services::Phase::Running,
        "bootstrap registered core FS service");
  text(
      Config,
      "[seed]\nexec=/bin/echo seeded\noneshot=yes\nrestart=never\n"
      "[worker]\nexec=/usr/tests/servicefixture hold\nrequires=seed\nready=notify\n"
      "[broken]\nexec=/usr/tests/servicefixture crash\nrestart-limit=2\nrestart-delay-ms=10\n"
      "[bad-ready]\nexec=/usr/tests/servicefixture bad-ready\nready=notify\nrestart=never\n"
      "[blocked]\nexec=/usr/tests/servicefixture hold\nrequires=broken\nready=notify\n"
      "[dependent]\nexec=/usr/tests/servicefixture hold\nrequires=external\nready=notify\n");
  int ready[2];
  check(pipe(ready) == 0, "daemon readiness pipe");
  int daemon = fork();
  check(daemon >= 0, "daemon fork");
  if (!daemon)
  {
    close(ready[0]);
    char number[16], reversed[16];
    unsigned value = ready[1], n = 0, at = 0;
    do
    {
      reversed[n++] = '0' + value % 10;
      value /= 10;
    } while (value);
    while (n)
    {
      number[at++] = reversed[--n];
    }
    number[at] = 0;
    char *args[] = {const_cast<char *>("serviced"),
                    const_cast<char *>("--config"),
                    const_cast<char *>(Config),
                    const_cast<char *>("--socket"),
                    const_cast<char *>(Socket),
                    const_cast<char *>("--ready-fd"),
                    number,
                    nullptr};
    exec("/sbin/serviced", args);
    exit(127);
  }
  close(ready[1]);
  pollfd event{ready[0], POLLIN, 0};
  char value;
  check(poll(&event, 1, 5000) > 0 && read(ready[0], &value, 1) == 1 && value == 'R',
        "daemon ready");
  close(ready[0]);
  auto running = phase("worker", services::Phase::Running);
  check(phase("seed", services::Phase::Complete).status == 0,
        "oneshot dependency completed first");
  auto broken = phase("broken", services::Phase::Failed);
  check(broken.restarts == 2 && WIFEXITED(broken.status) &&
            WEXITSTATUS(broken.status) == 42,
        "failure restart budget and exit status");
  phase("bad-ready", services::Phase::Failed);
  auto blocked = phase("blocked", services::Phase::Waiting);
  check(!blocked.pid, "failed dependency prevents activation");
  check(logged(running.log, "fixture: ready"), "per-service output log");
  text("/tmp/service-cycle.conf",
       "[a]\nexec=/bin/echo a\nafter=b\n[b]\nexec=/bin/echo b\nafter=a\n");
  check(request(services::Command::Load, "/tmp/service-cycle.conf").result < 0 &&
            request(services::Command::Status, "worker").pid == running.pid,
        "invalid reload leaves live services untouched");
  int unprivileged = fork();
  check(unprivileged >= 0, "permission probe fork");
  if (!unprivileged)
  {
    check(setuid(1000) == 0, "drop privilege");
    services::Request query;
    services::Reply response;
    exit(services::exchange(Socket, query, response) ? 1 : 0);
  }
  completed(unprivileged);
  check(request(services::Command::Restart, "worker").result == 0, "restart command");
  auto restarted = phase("worker", services::Phase::Running);
  check(restarted.pid != running.pid, "restart creates a fresh process");
  check(request(services::Command::Stop, "seed").result == 0, "stop dependency");
  phase("worker", services::Phase::Stopped);
  check(request(services::Command::Start, "worker").result == 0,
        "start dependencies recursively");
  auto current_worker = phase("worker", services::Phase::Running);
  int external = fork();
  check(external >= 0, "external service fork");
  if (!external)
  {
    for (;;)
    {
      poll(nullptr, 0, 1000);
    }
  }
  services::Request registration;
  registration.command = services::Command::Register;
  registration.pid = external;
  strcpy(registration.name, "external");
  services::Reply response;
  check(services::exchange(Socket, registration, response) && response.result == 0,
        "register externally supervised service");
  auto dependent = phase("dependent", services::Phase::Running);
  check(kill(external) == 0, "terminate external service");
  int status;
  check(waitpid(external, &status, 0) == external, "reap external service");
  phase("external", services::Phase::Unavailable);
  phase("dependent", services::Phase::Waiting);
  check(request(services::Command::Stop, "external").result < 0,
        "external/core service lifecycle is read-only");
  check(sigsend(daemon, SIGTERM) == 0, "stop daemon");
  completed(daemon);
  check(sigsend(restarted.pid, 0) < 0 && sigsend(current_worker.pid, 0) < 0 &&
            sigsend(dependent.pid, 0) < 0,
        "shutdown leaves no worker descendants");
  unlink(Config);
  unlink("/tmp/service-cycle.conf");
  printf(
      "servicetest: PASS: boot integration, dependencies, readiness, logs, restart budgets, permissions and cleanup\n");
  return 0;
}
