#include "user/runtime/ui.h"
#include "supervisor.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/process/wait/wait.h"
#include "kernel/support/placement_new.hpp"

namespace services
{
namespace
{
unsigned long
milliseconds() {
  return clockticks() / 10000;
}

void
decimal(unsigned value, char *text) {
  char reversed[16];
  unsigned n = 0;
  do
  {
    reversed[n++] = '0' + value % 10;
    value /= 10;
  } while (value);
  unsigned used = 0;
  while (n)
  {
    text[used++] = reversed[--n];
  }
  text[used] = 0;
}
} // namespace

int
Supervisor::find(const char *name) const {
  for (unsigned i = 0; i < count_; ++i)
  {
    if (!strcmp(entries_[i].definition.name, name))
    {
      return i;
    }
  }
  return -1;
}

bool
Supervisor::load(const char *path, char error[96]) {
  auto *storage = malloc(sizeof(Configuration));
  if (!storage)
  {
    strcpy(error, "out of memory");
    return false;
  }
  auto *config = new (storage) Configuration{};
  bool good = load_configuration(path, *config, error);
  unsigned external_count = 0;
  for (unsigned i = 0; good && i < count_; ++i)
  {
    if (!entries_[i].external && entries_[i].pid)
    {
      good = false;
      strcpy(error, "stop running services before loading configuration");
    }
    if (entries_[i].external)
    {
      ++external_count;
      for (unsigned j = 0; j < config->count; ++j)
      {
        if (!strcmp(entries_[i].definition.name, config->entries[j].name))
        {
          good = false;
          strcpy(error, "configuration conflicts with a core service");
        }
      }
    }
  }
  if (good && external_count + config->count > Maximum)
  {
    good = false;
    strcpy(error, "too many services");
  }
  if (good && !local_)
  {
    retire_guardians();
    for (unsigned i = 0; i < count_; ++i)
    {
      if (!entries_[i].external && entries_[i].guardian)
      {
        good = false;
        strcpy(error, "cannot retire previous service guardians");
      }
    }
  }
  if (good)
  {
    unsigned kept = 0;
    for (unsigned i = 0; i < count_; ++i)
    {
      if (entries_[i].external)
      {
        entries_[kept++] = entries_[i];
      }
    }
    count_ = kept;
    {
      for (unsigned i = 0; i < config->count; ++i)
      {
        auto &entry = entries_[count_++];
        entry.~Service();
        new (&entry) Service{};
        entry.definition = config->entries[i];
        entry.wanted = entry.definition.enabled;
        entry.phase = entry.wanted ? Phase::Waiting : Phase::Stopped;
      }
    }
  }
  config->~Configuration();
  free(config);
  if (good && !local_ && !discover_guardians())
  {
    strcpy(error, "cannot enumerate surviving service guardians");
    good = false;
  }
  return good;
}

bool
Supervisor::dependencies(unsigned index) const {
  if (local_)
  {
    return permitted_;
  }
  for (unsigned d = 0; d < entries_[index].definition.dependency_count; ++d)
  {
    int dependency = find(entries_[index].definition.dependencies[d]);
    if (dependency < 0)
    {
      return false;
    }
    const auto &required = entries_[dependency];
    if (required.phase != Phase::Complete &&
        (required.phase != Phase::Running || required.definition.oneshot))
    {
      return false;
    }
  }
  return true;
}

bool
Supervisor::activate(unsigned index, bool manual, unsigned depth) {
  if (depth > Maximum)
  {
    return false;
  }
  auto &entry = entries_[index];
  if (entry.unconfigured)
  {
    return false;
  }
  if (entry.external)
  {
    return entry.phase == Phase::Running || entry.phase == Phase::Complete;
  }
  if (!manual && (entry.phase == Phase::Failed || entry.phase == Phase::Unavailable))
  {
    return false;
  }
  for (unsigned d = 0; !local_ && d < entry.definition.dependency_count; ++d)
  {
    int dependency = find(entry.definition.dependencies[d]);
    if (dependency < 0 || !activate(dependency, false, depth + 1))
    {
      strcpy(entry.detail, "dependency unavailable");
      return false;
    }
  }
  if (!local_)
  {
    if (!ensure_guardian(index))
    {
      return false;
    }
    return (!manual && entry.wanted) || forward(index, Command::Start);
  }
  if (manual)
  {
    entry.restarts = 0;
  }
  entry.wanted = true;
  if (entry.phase == Phase::Stopped || entry.phase == Phase::Failed ||
      entry.phase == Phase::Unavailable || (manual && entry.phase == Phase::Complete))
  {
    entry.phase = Phase::Waiting;
  }
  return true;
}

void
Supervisor::spawn(unsigned index) {
  auto &entry = entries_[index];
  entry.detail[0] = 0;
  int notification[2]{-1, -1}, executed[2]{-1, -1}, output[2]{-1, -1};
  if (!open_log(entry) || pipe(executed) < 0 || pipe(output) < 0 ||
      (entry.definition.notify && pipe(notification) < 0))
  {
    for (int fd : executed)
    {
      if (fd >= 0)
      {
        close(fd);
      }
    }
    for (int fd : notification)
    {
      if (fd >= 0)
      {
        close(fd);
      }
    }
    for (int fd : output)
    {
      if (fd >= 0)
      {
        close(fd);
      }
    }
    strcpy(entry.detail, "cannot allocate launch channels");
    exited(entry, 1 << 8);
    return;
  }
  fcntl(executed[1], F_SETFD, FD_CLOEXEC);
  int child = fork();
  if (!child)
  {
    close(executed[0]);
    if (notification[0] >= 0)
    {
      close(notification[0]);
    }
    for (int fd = 0; fd < 16; ++fd)
    {
      if (fd != executed[1] && fd != notification[1] && fd != output[1])
      {
        close(fd);
      }
    }
    if (setsid() < 0)
    {
      write(executed[1], "E", 1);
      exit(1);
    }
    if (open("/dev/zero", O_RDONLY) != 0 || dup(output[1]) != 1 || dup(1) != 2)
    {
      write(executed[1], "E", 1);
      exit(1);
    }
    close(output[1]);
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    if (entry.definition.notify)
    {
      char descriptor[16];
      decimal(notification[1], descriptor);
      if (setenv("SERVICE_READY_FD", descriptor, 1) < 0)
      {
        write(executed[1], "E", 1);
        exit(1);
      }
    }
    else
    {
      unsetenv("SERVICE_READY_FD");
    }
    char *arguments[9]{};
    for (unsigned i = 0; i < entry.definition.argc; ++i)
    {
      arguments[i] = entry.definition.arguments[i];
    }
    exec(arguments[0], arguments);
    write(executed[1], "E", 1);
    exit(127);
  }
  close(executed[1]);
  close(output[1]);
  if (notification[1] >= 0)
  {
    close(notification[1]);
  }
  if (child < 0)
  {
    close(executed[0]);
    close(output[0]);
    if (notification[0] >= 0)
    {
      close(notification[0]);
    }
    strcpy(entry.detail, "fork failed");
    exited(entry, 1 << 8);
    return;
  }
  entry.pid = child;
  entry.output = output[0];
  fcntl(entry.output, F_SETFL, O_NONBLOCK);
  entry.ready = notification[0];
  entry.phase = Phase::Starting;
  entry.deadline = milliseconds() + entry.definition.ready_timeout;
  pollfd outcome{executed[0], POLLIN | POLLHUP, 0};
  int waited;
  do
  {
    waited = poll(&outcome, 1, 2000);
  } while (waited < 0 && errno == EINTR);
  char failed;
  int received = waited > 0 ? read(executed[0], &failed, 1) : -1;
  close(executed[0]);
  if (received != 0)
  {
    strcpy(entry.detail, "exec or log setup failed");
    sigsend(-child, SIGKILL);
    kill(child);
  }
  else if (!entry.definition.notify)
  {
    entry.phase = Phase::Running;
  }
  ui::info("serviced: start %s pid=%d\n", entry.definition.name, child);
}

void
Supervisor::exited(Service &entry, int status) {
  if (entry.definition.notify && entry.phase == Phase::Starting && status == 0)
  {
    status = 1 << 8;
    strcpy(entry.detail, "exited before readiness notification");
  }
  int old = entry.pid;
  entry.pid = 0;
  entry.status = status;
  if (entry.ready >= 0)
  {
    close(entry.ready);
    entry.ready = -1;
  }
  if (old > 0)
  {
    sigsend(-old, SIGKILL);
  }
  if (!drain_log(entry))
  {
    strcpy(entry.detail, "log write or rotation failed");
  }
  close_log(entry);
  bool success = WIFEXITED(status) && WEXITSTATUS(status) == 0;
  if (!entry.wanted && !entry.restart_after_stop)
  {
    entry.phase = Phase::Stopped;
    return;
  }
  if (entry.restart_after_stop)
  {
    entry.restart_after_stop = false;
    entry.wanted = true;
    entry.phase = Phase::Waiting;
    return;
  }
  if (success && entry.definition.oneshot)
  {
    entry.phase = Phase::Complete;
    return;
  }
  bool retry = entry.definition.restart == Restart::Always ||
               (!success && entry.definition.restart == Restart::Failure);
  if (retry && entry.restarts < entry.definition.limit)
  {
    unsigned shift = entry.restarts < 7 ? entry.restarts : 7;
    unsigned delay = entry.definition.delay << shift;
    if (delay > 30000)
    {
      delay = 30000;
    }
    ++entry.restarts;
    entry.deadline = milliseconds() + delay;
    entry.phase = Phase::Backoff;
  }
  else
  {
    entry.phase = success ? Phase::Complete : Phase::Failed;
  }
  printf("serviced: exit %s status=%d state=%s\n", entry.definition.name, status,
         phase_name(entry.phase));
}

void
Supervisor::stop(unsigned index, bool restart, unsigned depth) {
  if (depth > Maximum)
  {
    return;
  }
  auto &entry = entries_[index];
  if (entry.external)
  {
    return;
  }
  for (unsigned i = 0; i < count_; ++i)
  {
    for (unsigned d = 0; d < entries_[i].definition.dependency_count; ++d)
    {
      if (!strcmp(entries_[i].definition.dependencies[d], entry.definition.name))
      {
        stop(i, false, depth + 1);
      }
    }
  }
  if (!local_)
  {
    if (entry.guardian)
    {
      (void)forward(index, restart ? Command::Restart : Command::Stop);
    }
    else
    {
      entry.wanted = restart;
      entry.phase = restart ? Phase::Waiting : Phase::Stopped;
    }
    return;
  }
  entry.wanted = false;
  entry.restart_after_stop = restart;
  if (entry.pid)
  {
    entry.phase = Phase::Stopping;
    entry.deadline = milliseconds() + 500;
    sigsend(-entry.pid, SIGTERM);
  }
  else
  {
    entry.phase = restart ? Phase::Waiting : Phase::Stopped;
    entry.wanted = restart;
  }
}

void
Supervisor::tick() {
  // A guardian handshake may wait for exec/IO. Return to the control listener
  // between managed entries instead of accumulating N synchronous handshakes.
  unsigned budget = local_ ? count_ : (count_ ? 1 : 0);
  for (unsigned step = 0; step < budget; ++step)
  {
    unsigned i = local_ ? step : tick_cursor_++ % count_;
    auto &entry = entries_[i];
    if (entry.external)
    {
      if (milliseconds() < entry.deadline)
      {
        continue;
      }
      entry.deadline = milliseconds() + 1000;
      if (entry.pid > 0 && sigsend(entry.pid, 0) < 0)
      {
        entry.phase = Phase::Unavailable;
        strcpy(entry.detail, "core service exited; existing mounts are not recovered");
      }
      continue;
    }
    if (!local_)
    {
      if (!ensure_guardian(i))
      {
        continue;
      }
      if (entry.wanted && entry.phase == Phase::Waiting)
      {
        (void)activate(i, false);
      }
      (void)forward(i, Command::DependencyState, dependencies(i));
      continue;
    }
    if (!drain_log(entry) && entry.pid)
    {
      strcpy(entry.detail, "log write or rotation failed");
      sigsend(-entry.pid, SIGKILL);
      kill(entry.pid);
    }
    if (entry.pid)
    {
      if ((entry.phase == Phase::Running || entry.phase == Phase::Starting) &&
          !dependencies(i))
      {
        strcpy(entry.detail, "dependency lost");
        stop(i, true);
      }
      if (entry.phase == Phase::Starting && entry.ready >= 0)
      {
        pollfd ready{entry.ready, POLLIN | POLLHUP, 0};
        if (poll(&ready, 1, 0) > 0)
        {
          char value = 0;
          if (read(entry.ready, &value, 1) == 1 && value == 'R')
          {
            entry.phase = Phase::Running;
            close(entry.ready);
            entry.ready = -1;
          }
          else
          {
            strcpy(entry.detail, "readiness channel closed");
            kill(entry.pid);
          }
        }
      }
      int status = 0;
      int reaped = waitpid(entry.pid, &status, WNOHANG);
      if (reaped == entry.pid)
      {
        exited(entry, status);
        continue;
      }
      if ((entry.phase == Phase::Starting || entry.phase == Phase::Stopping) &&
          milliseconds() >= entry.deadline)
      {
        strcpy(entry.detail, "readiness/stop deadline expired");
        sigsend(-entry.pid, SIGKILL);
        kill(entry.pid);
      }
      continue;
    }
    if (entry.phase == Phase::Backoff && milliseconds() >= entry.deadline)
    {
      entry.phase = Phase::Waiting;
    }
    if (entry.wanted && entry.phase == Phase::Waiting && activate(i, false) &&
        dependencies(i))
    {
      spawn(i);
    }
  }
}

void
Supervisor::describe(unsigned index, Reply &reply) const {
  auto &entry = entries_[index];
  reply.result = 0;
  reply.pid = entry.pid;
  reply.status = entry.status;
  reply.restarts = entry.restarts;
  reply.phase = entry.phase;
  reply.external = entry.external;
  reply.wanted = entry.wanted;
  reply.guardian = entry.guardian;
  reply.definition_id = definition_id(entry.definition);
  reply.log_captured = entry.log_captured;
  reply.log_rotations = entry.log_rotations;
  if (entry.guardian)
  {
    guardian_path(index, reply.guardian_socket);
  }
  strcpy(reply.name, entry.definition.name);
  strcpy(reply.log, entry.definition.log);
  strcpy(reply.detail, entry.detail);
}

void
Supervisor::request(const Request &request, Reply &reply) {
  if (local_ && (request.command == Command::DependencyState ||
                 request.command == Command::Retire))
  {
    if (strcmp(request.name, entries_[0].definition.name))
    {
      return;
    }
    if (request.command == Command::Retire)
    {
      retiring_ = true;
      stop(0);
    }
    else
    {
      permitted_ = request.pid != 0;
    }
    describe(0, reply);
    return;
  }
  if (request.command == Command::Load)
  {
    if (local_)
    {
      return;
    }
    reply.result = load(request.path, reply.detail) ? 0 : -1;
    return;
  }
  if (request.command == Command::Register)
  {
    if (local_)
    {
      return;
    }
    int existing = find(request.name);
    if (!valid_name(request.name) || request.pid < 0 ||
        (existing < 0 && count_ == Maximum) ||
        (existing >= 0 && !entries_[existing].external))
    {
      return;
    }
    auto &entry = entries_[existing >= 0 ? unsigned(existing) : count_++];
    if (existing < 0)
    {
      entry.~Service();
      new (&entry) Service{};
    }
    int previous_pid = entry.pid;
    Phase previous_phase = entry.phase;
    strcpy(entry.definition.name, request.name);
    entry.external = true;
    entry.pid = request.pid;
    entry.phase = request.pid ? Phase::Running : Phase::Complete;
    strcpy(entry.detail, "core service; lifecycle owned by bootstrap");
    if (!save_external())
    {
      if (existing < 0)
      {
        --count_;
      }
      else
      {
        entry.pid = previous_pid;
        entry.phase = previous_phase;
      }
      strcpy(reply.detail, "cannot save core service registry");
      return;
    }
    reply.result = 0;
    return;
  }
  int index =
      request.command == Command::List ? int(request.index) : find(request.name);
  if (index < 0 || unsigned(index) >= count_)
  {
    return;
  }
  if (request.command == Command::Start)
  {
    if (!activate(index, true))
    {
      return;
    }
  }
  else if (request.command == Command::Stop || request.command == Command::Restart)
  {
    if (entries_[index].external)
    {
      strcpy(reply.detail, "core service is read-only");
      return;
    }
    if (request.command == Command::Restart)
    {
      entries_[index].restarts = 0;
    }
    stop(index, request.command == Command::Restart);
  }
  else if (request.command != Command::List && request.command != Command::Status)
  {
    return;
  }
  describe(index, reply);
}

void
Supervisor::shutdown() {
  if (!local_)
  {
    retire_guardians();
    return;
  }
  for (unsigned i = count_; i; --i)
  {
    if (!entries_[i - 1].external)
    {
      stop(i - 1);
    }
  }
  auto deadline = milliseconds() + 2000;
  while (milliseconds() < deadline)
  {
    bool pending = false;
    tick();
    for (unsigned i = 0; i < count_; ++i)
    {
      pending |= !entries_[i].external && entries_[i].pid;
    }
    if (!pending)
    {
      break;
    }
    poll(nullptr, 0, 10);
  }
}
} // namespace services
