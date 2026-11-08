#include "user/services/protocol.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/stat.h"
#include "kernel/process/wait/wait.h"
#include "kernel/support/placement_new.hpp"

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
request(services::Command command, const char *name, const char *control = Socket) {
  services::Request query;
  query.command = command;
  if (name)
  {
    strcpy(command == services::Command::Load ? query.path : query.name, name);
  }
  services::Reply response;
  bool okay = services::exchange(control, query, response);
  if (!okay) fprintf(2, "servicetest: command=%u name=%s socket=%s errno=%d\n",
                     unsigned(command), name ? name : "", control, errno);
  check(okay, "control transport");
  return response;
}

services::Reply
phase(const char *name, services::Phase wanted, const char *control = Socket) {
  auto deadline = clockticks() + 100000000;
  for (;;)
  {
    auto state = request(services::Command::Status, name, control);
    check(state.result == 0, "named service exists");
    if (state.phase == wanted)
    {
      return state;
    }
    if (clockticks() >= deadline)
    {
      fprintf(2,
              "servicetest: name=%s wanted=%s actual=%s pid=%d guardian=%d detail=%s\n",
              name, services::phase_name(wanted), services::phase_name(state.phase),
              state.pid, state.guardian, state.detail);
      check(false, "service phase deadline");
    }
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

void
output_done(unsigned wanted, const char *control = Socket) {
  auto deadline = clockticks() + 100000000;
  for (;;)
  {
    unsigned completed = 0;
    int fd = open("/tmp/service-logger.done", O_RDONLY);
    if (fd >= 0)
    {
      (void)read(fd, &completed, sizeof(completed));
      close(fd);
    }
    if (completed >= wanted)
    {
      break;
    }
    check(clockticks() < deadline, "logger burst completion");
    poll(nullptr, 0, 20);
  }
  unsigned header = sizeof("fixture: ready pid=\n") - 1;
  unsigned long total = 0;
  for (;;)
  {
    auto state = request(services::Command::Status, "logger", control);
    unsigned digits = 1;
    for (unsigned pid = state.pid; pid >= 10; pid /= 10)
    {
      ++digits;
    }
    total = wanted * 8192UL + header + digits;
    if (state.log_captured >= total && state.log_rotations >= wanted * 8)
    {
      break;
    }
    check(state.phase == services::Phase::Running && clockticks() < deadline,
          "guardian drains complete stdout/stderr burst before inspecting rotation");
    poll(nullptr, 0, 20);
  }
  // The capture counter gives a stable barrier after the writer's burst.
  const char *paths[] = {"/tmp/service-logger.log.2", "/tmp/service-logger.log.1",
                         "/tmp/service-logger.log"};
  unsigned long retained = 0;
  for (const char *path : paths)
  {
    struct stat metadata{};
    int inspected = stat(path, &metadata);
    if (inspected < 0 || !metadata.size || metadata.size > 1024)
    {
      fprintf(2, "servicetest: log=%s stat=%d size=%lu\n", path, inspected,
              metadata.size);
    }
    check(inspected == 0 && metadata.size > 0 && metadata.size <= 1024,
          "active and archived log files stay bounded");
    retained += metadata.size;
  }
  unsigned long position = total - retained;
  for (const char *path : paths)
  {
    int fd = open(path, O_RDONLY);
    check(fd >= 0, "read rotated tail");
    char bytes[256];
    int count;
    while ((count = read(fd, bytes, sizeof(bytes))) > 0)
    {
      for (int i = 0; i < count; ++i, ++position)
      {
        unsigned long body = position - (total - wanted * 8192UL);
        char expected = body % 256 == 255 ? '\n' : 'A' + (body / 256) % 26;
        check(
            bytes[i] == expected,
            "rotation preserves ordered stdout/stderr tail without loss or duplication");
      }
    }
    close(fd);
    check(count == 0, "read complete rotated log");
  }
  check(position == total, "retained logs reach the captured stream end");
  struct stat absent{};
  check(stat("/tmp/service-logger.log.3", &absent) < 0,
        "log rotation retains only configured archives");
}

void
crash_manager(int daemon) {
  check(kill(daemon) == 0, "force manager failure");
  int status = 0;
  check(waitpid(daemon, &status, 0) == daemon && WIFSIGNALED(status) &&
            WTERMSIG(status) == SIGKILL,
        "failed manager was reaped");
}

int
launch() {
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
  return daemon;
}
} // namespace

extern "C" int
main() {
  struct Observed
  {
    services::Reply root, response, running, broken, blocked, logger, adopted,
        adopted_logger, restarted, current_worker, dependent, signal_probe, resilient,
        resilient_after;
  };

  void *memory = malloc(sizeof(Observed));
  check(memory != nullptr, "allocate observed service state");
  auto &state = *new (memory) Observed{};
  services::Request core;
  core.command = services::Command::Status;
  strcpy(core.name, "rootfs");
  auto &root = state.root;
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
      "[dependent]\nexec=/usr/tests/servicefixture hold\nrequires=external\nready=notify\n"
      "[logger]\nexec=/usr/tests/servicefixture logger /tmp/service-logger.done\nready=notify\n"
      "log-max-bytes=1024\nlog-backups=2\n"
      "[signal-probe]\nexec=/usr/tests/servicefixture hold\nready=notify\n"
      "[resilient]\nexec=/usr/tests/servicefixture hold\nready=notify\n");
  int daemon = launch();
  auto &running = state.running;
  running = phase("worker", services::Phase::Running);
  check(phase("seed", services::Phase::Complete).status == 0,
        "oneshot dependency completed first");
  auto &broken = state.broken;
  broken = phase("broken", services::Phase::Failed);
  check(broken.restarts == 2 && WIFEXITED(broken.status) &&
            WEXITSTATUS(broken.status) == 42,
        "failure restart budget and exit status");
  phase("bad-ready", services::Phase::Failed);
  auto &blocked = state.blocked;
  blocked = phase("blocked", services::Phase::Waiting);
  check(!blocked.pid, "failed dependency prevents activation");
  check(logged(running.log, "fixture: ready"), "per-service output log");
  auto &logger = state.logger;
  logger = phase("logger", services::Phase::Running);
  auto &resilient = state.resilient;
  auto &resilient_after = state.resilient_after;
  resilient = phase("resilient", services::Phase::Running);
  output_done(1);
  crash_manager(daemon);
  check(kill(resilient.pid) == 0, "fail an application while its manager is down");
  auto recovery_deadline = clockticks() + 100000000;
  do
  {
    resilient_after =
        phase("resilient", services::Phase::Running, resilient.guardian_socket);
    if (resilient_after.pid != resilient.pid)
    {
      break;
    }
    check(clockticks() < recovery_deadline,
          "guardian restarts application without manager");
    poll(nullptr, 0, 20);
  } while (true);
  check(resilient_after.restarts == 1 && resilient_after.guardian == resilient.guardian,
        "guardian owns restart policy during manager outage");
  check(sigsend(running.pid, 0) == 0 && sigsend(logger.pid, SIGUSR1) == 0,
        "services survive manager crash");
  output_done(2, logger.guardian_socket);
  daemon = launch();
  auto &adopted = state.adopted;
  adopted = phase("worker", services::Phase::Running);
  auto &adopted_logger = state.adopted_logger;
  adopted_logger = phase("logger", services::Phase::Running);
  check(
      adopted.pid == running.pid && adopted.guardian == running.guardian &&
          adopted_logger.pid == logger.pid &&
          adopted_logger.guardian == logger.guardian,
      "new manager adopts original services and guardians without duplicate launches");
  check(phase("resilient", services::Phase::Running).pid == resilient_after.pid &&
            phase("resilient", services::Phase::Running).restarts == 1,
        "adoption retains the restart performed during manager outage");
  check(phase("broken", services::Phase::Failed).restarts == 2 &&
            phase("seed", services::Phase::Complete).status == 0,
        "adoption preserves failed budgets and completed oneshot state");
  int duplicate = fork();
  check(duplicate >= 0, "duplicate manager probe");
  if (!duplicate)
  {
    char *arguments[] = {const_cast<char *>("serviced"), const_cast<char *>("--config"),
                         const_cast<char *>(Config),     const_cast<char *>("--socket"),
                         const_cast<char *>(Socket),     nullptr};
    exec("/sbin/serviced", arguments);
    exit(127);
  }
  int duplicate_status = 0;
  check(waitpid(duplicate, &duplicate_status, 0) == duplicate &&
            WIFEXITED(duplicate_status) && WEXITSTATUS(duplicate_status) == 1 &&
            sigsend(running.pid, 0) == 0,
        "second manager cannot acquire ownership or stop existing services");
  check(sigsend(logger.pid, SIGUSR1) == 0, "produce logs after adoption");
  output_done(3);
  text("/tmp/service-cycle.conf",
       "[a]\nexec=/bin/echo a\nafter=b\n[b]\nexec=/bin/echo b\nafter=a\n");
  check(request(services::Command::Load, "/tmp/service-cycle.conf").result < 0 &&
            request(services::Command::Status, "worker").pid == running.pid,
        "invalid reload leaves live services untouched");
  text("/tmp/service-cycle.conf",
       "[a]\nexec=/bin/echo a\nlog=/tmp/shared-service.log\n"
       "[b]\nexec=/bin/echo b\nlog=/tmp/shared-service.log\n");
  check(request(services::Command::Load, "/tmp/service-cycle.conf").result < 0 &&
            request(services::Command::Status, "worker").pid == running.pid,
        "two services cannot race rotation of the same log path");
  int unprivileged = fork();
  check(unprivileged >= 0, "permission probe fork");
  if (!unprivileged)
  {
    check(setuid(1000) == 0, "drop privilege");
    services::Request query;
    auto &response = state.response;
    exit(services::exchange(Socket, query, response) ? 1 : 0);
  }
  completed(unprivileged);
  check(request(services::Command::Restart, "worker").result == 0, "restart command");
  auto &restarted = state.restarted;
  restarted = phase("worker", services::Phase::Running);
  check(restarted.pid != running.pid, "restart creates a fresh process");
  check(request(services::Command::Stop, "seed").result == 0, "stop dependency");
  phase("worker", services::Phase::Stopped);
  check(request(services::Command::Start, "worker").result == 0,
        "start dependencies recursively");
  auto &current_worker = state.current_worker;
  current_worker = phase("worker", services::Phase::Running);
  check(request(services::Command::Stop, "worker").result == 0,
        "stop worker before another manager crash");
  phase("worker", services::Phase::Stopped);
  crash_manager(daemon);
  daemon = launch();
  check(!phase("worker", services::Phase::Stopped).pid,
        "manager restart preserves an explicit stopped state");
  check(request(services::Command::Start, "worker").result == 0,
        "adopted stopped service can be started");
  current_worker = phase("worker", services::Phase::Running);
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
  auto &dependent = state.dependent;
  dependent = phase("dependent", services::Phase::Running);
  crash_manager(daemon);
  daemon = launch();
  check(phase("external", services::Phase::Running).pid == external &&
            phase("dependent", services::Phase::Running).pid == dependent.pid,
        "external dependency registry is restored before adopting dependent services");
  check(kill(external) == 0, "terminate external service");
  int status;
  check(waitpid(external, &status, 0) == external, "reap external service");
  phase("external", services::Phase::Unavailable);
  phase("dependent", services::Phase::Waiting);
  check(request(services::Command::Stop, "external").result < 0,
        "external/core service lifecycle is read-only");
  check(kill(resilient.guardian) == 0, "force guardian failure with live application");
  phase("resilient", services::Phase::Unavailable);
  check(sigsend(resilient_after.pid, 0) == 0,
        "guardian failure does not prove application death");
  crash_manager(daemon);
  daemon = launch();
  phase("resilient", services::Phase::Unavailable);
  check(request(services::Command::Start, "resilient").result < 0 &&
            sigsend(resilient_after.pid, 0) == 0,
        "stale guardian endpoint prevents a duplicate launch after manager restart");
  check(kill(resilient_after.pid) == 0, "explicit cleanup of unowned application");
  auto orphan_deadline = clockticks() + 100000000;
  while (sigsend(resilient_after.pid, 0) == 0)
  {
    check(clockticks() < orphan_deadline, "unowned application exits");
    poll(nullptr, 0, 20);
  }
  text(Config, "# definitions removed while manager was down\n");
  crash_manager(daemon);
  daemon = launch();
  check(phase("worker", services::Phase::Unavailable).pid == current_worker.pid &&
            phase("logger", services::Phase::Unavailable).pid == logger.pid &&
            request(services::Command::Start, "worker").result < 0,
        "unconfigured surviving services remain visible and are not relaunched");
  check(request(services::Command::Stop, "worker").result == 0,
        "unconfigured surviving service can still be stopped");
  auto stop_deadline = clockticks() + 100000000;
  while (request(services::Command::Status, "worker").pid)
  {
    check(clockticks() < stop_deadline, "stop unconfigured service");
    poll(nullptr, 0, 20);
  }
  auto &signal_probe = state.signal_probe;
  signal_probe = phase("signal-probe", services::Phase::Unavailable);
  check(signal_probe.pid > 0 && sigsend(signal_probe.guardian, SIGTERM) == 0,
        "signal guardian on independent mmap stack");
  auto signal_deadline = clockticks() + 100000000;
  while (sigsend(signal_probe.pid, 0) == 0)
  {
    check(clockticks() < signal_deadline,
          "mapped-stack signal handler shuts down its service before guardian exit");
    poll(nullptr, 0, 20);
  }
  check(sigsend(daemon, SIGTERM) == 0, "stop daemon");
  completed(daemon);
  check(unlink(resilient.guardian_socket) == 0,
        "remove failure marker only after confirming unowned application death");
  check(sigsend(restarted.pid, 0) < 0 && sigsend(current_worker.pid, 0) < 0 &&
            sigsend(dependent.pid, 0) < 0 && sigsend(logger.pid, 0) < 0,
        "shutdown leaves no worker descendants");
  int manager_log = open("/tmp/serviced.log", O_WRONLY | O_TRUNC);
  check(manager_log >= 0, "prepare bootstrap manager log rotation");
  char log_block[256];
  memset(log_block, 'Z', sizeof(log_block));
  for (unsigned i = 0; i < 256; ++i)
  {
    check(write(manager_log, log_block, sizeof(log_block)) == sizeof(log_block),
          "fill manager diagnostic log");
  }
  close(manager_log);
  state.~Observed();
  free(memory);
  unlink(Config);
  unlink("/tmp/service-cycle.conf");
  printf(
      "servicetest: PASS: dependencies, readiness, crash adoption, stopped state, bounded live logs, restart budgets, permissions and cleanup\n");
  return 0;
}
