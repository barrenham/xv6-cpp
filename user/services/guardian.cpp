#include "supervisor.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/stat.h"
#include "kernel/process/wait/wait.h"
#include "user/runtime/directory.h"
#include "kernel/support/placement_new.hpp"


namespace services
{
namespace
{
unsigned long
hash_bytes(unsigned long value, const char *bytes, unsigned length) {
  for (unsigned i = 0; i < length; ++i)
  {
    value = (value ^ static_cast<unsigned char>(bytes[i])) * 1099511628211UL;
  }
  return value;
}

volatile sig_atomic_t guardian_stopping;

void
stop_guardian(int) {
  guardian_stopping = 1;
}
} // namespace

unsigned long
definition_id(const Definition &definition) {
  unsigned long result = 14695981039346656037UL;
  auto text = [&](const char *value) {
    result = hash_bytes(result, value, strlen(value) + 1);
  };
  auto number = [&](unsigned value) {
    for (unsigned i = 0; i < 4; ++i)
    {
      char byte = value >> (i * 8);
      result = hash_bytes(result, &byte, 1);
    }
  };
  text(definition.name);
  number(definition.argc);
  for (unsigned i = 0; i < definition.argc; ++i)
  {
    text(definition.arguments[i]);
  }
  number(definition.dependency_count);
  for (unsigned i = 0; i < definition.dependency_count; ++i)
  {
    text(definition.dependencies[i]);
  }
  text(definition.log);
  number(definition.delay);
  number(definition.limit);
  number(definition.ready_timeout);
  number(definition.log_max_bytes);
  number(definition.log_backups);
  number(unsigned(definition.restart));
  number(definition.enabled);
  number(definition.oneshot);
  number(definition.notify);
  return result;
}

bool
Supervisor::configure(const char *socket) {
  strcpy(socket_, socket);
  char path[108];
  core_path(path);
  path[25] = 0;
  (void)mkdir(path);
  struct stat metadata{};
  bool okay = lstat(path, &metadata) == 0 && metadata.type == T_DIR &&
              metadata.uid == 0 && !(metadata.mode & 0077);
  int fd = okay ? open(path, O_RDONLY) : -1;
  okay = okay && fd >= 0;
  if (fd >= 0)
  {
    close(fd);
  }
  return okay && restore_external();
}

void
Supervisor::guardian_path(unsigned index, char *path) const {
  strcpy(path, "/tmp/svc-");
  unsigned long identity = hash_bytes(14695981039346656037UL, socket_, strlen(socket_));
  for (unsigned i = 0; i < 16; ++i)
  {
    path[9 + i] = "0123456789abcdef"[(identity >> ((15 - i) * 4)) & 15];
  }
  path[25] = '/';
  strcpy(path + 26, entries_[index].definition.name);
  strcpy(path + strlen(path), ".sock");
}

void
Supervisor::core_path(char *path) const {
  guardian_path(0, path);
  strcpy(path + 26, "core");
}

namespace
{
struct CoreRegistry
{
  unsigned version = Version, count = 0;

  struct Record
  {
    char name[32]{};
    int pid = 0;
  } entries[Maximum]{};
};
} // namespace

bool
Supervisor::restore_external() {
  char path[108];
  core_path(path);
  int fd = open(path, O_RDONLY | O_NOFOLLOW);
  if (fd < 0)
  {
    // Ordinary-file open has a legacy error ABI. Confirm actual absence,
    // rather than accepting I/O or descriptor exhaustion as an empty registry.
    path[25] = 0;
    int directory = open(path, O_RDONLY);
    if (directory < 0)
    {
      return false;
    }
    DirectoryReader reader(directory);
    const linux_dirent64 *entry;
    int result;
    bool found = false;
    while ((result = reader.next(entry)) > 0)
    {
      if (!strcmp(entry->d_name, "core"))
      {
        found = true;
      }
    }
    close(directory);
    return result == 0 && !found;
  }
  CoreRegistry record;
  struct stat metadata{};
  bool okay = fstat(fd, &metadata) == 0 && metadata.type == T_FILE &&
              metadata.uid == 0 && !(metadata.mode & 0077) &&
              metadata.size == sizeof(record) &&
              read(fd, &record, sizeof(record)) == sizeof(record) &&
              record.version == Version && record.count <= Maximum;
  close(fd);
  if (!okay)
  {
    return false;
  }
  for (unsigned i = 0; i < record.count; ++i)
  {
    const auto &source = record.entries[i];
    if (source.name[31] || !valid_name(source.name) || source.pid < 0 ||
        find(source.name) >= 0)
    {
      return false;
    }
    auto &entry = entries_[count_++];
    strcpy(entry.definition.name, source.name);
    entry.external = true;
    entry.pid = source.pid;
    entry.phase = !source.pid                   ? Phase::Complete
                  : sigsend(source.pid, 0) == 0 ? Phase::Running
                                                : Phase::Unavailable;
    strcpy(entry.detail, "core service; lifecycle owned by bootstrap");
  }
  return true;
}

bool
Supervisor::save_external() {
  CoreRegistry record;
  for (unsigned i = 0; i < count_; ++i)
  {
    if (entries_[i].external)
    {
      auto &destination = record.entries[record.count++];
      strcpy(destination.name, entries_[i].definition.name);
      destination.pid = entries_[i].pid;
    }
  }
  char path[108], temporary[112];
  core_path(path);
  strcpy(temporary, path);
  strcpy(temporary + strlen(temporary), ".new");
  int fd = open(temporary, O_CREATE | O_TRUNC | O_WRONLY | O_NOFOLLOW);
  bool okay = fd >= 0 && write(fd, &record, sizeof(record)) == sizeof(record);
  if (fd >= 0)
  {
    close(fd);
  }
  if (okay)
  {
    okay = rename(temporary, path) == 0;
  }
  if (!okay)
  {
    unlink(temporary);
  }
  return okay;
}

bool
Supervisor::refresh_guardian(unsigned index) {
  auto &entry = entries_[index];
  char path[108];
  guardian_path(index, path);
  Request request;
  request.command = Command::Status;
  strcpy(request.name, entry.definition.name);
  Reply response;
  if (!exchange(path, request, response) || response.result < 0)
  {
    return false;
  }
  entry.guardian = response.guardian;
  entry.pid = response.pid;
  entry.status = response.status;
  entry.restarts = response.restarts;
  entry.log_captured = response.log_captured;
  entry.log_rotations = response.log_rotations;
  if (entry.unconfigured)
  {
    strcpy(entry.definition.log, response.log);
  }
  if (response.definition_id != definition_id(entry.definition))
  {
    entry.phase = Phase::Unavailable;
    entry.wanted = false;
    strcpy(entry.detail,
           "surviving guardian configuration differs; stop before reload");
    return false;
  }
  entry.phase = response.phase;
  entry.wanted = response.wanted;
  strcpy(entry.detail, response.detail);
  return true;
}

bool
Supervisor::ensure_guardian(unsigned index) {
  auto &entry = entries_[index];
  if (refresh_guardian(index))
  {
    return true;
  }
  if (entry.unconfigured)
  {
    return false;
  }
  char path[108];
  guardian_path(index, path);
  struct stat present{};
  // A dead guardian can leave a live service behind. Its stale pathname is a
  // tombstone, not permission to start another copy. Only clean retirement
  // removes the pathname. Never infer service death from manager/guardian death.
  if (entry.guardian || stat(path, &present) == 0)
  {
    entry.phase = Phase::Unavailable;
    if (!entry.detail[0])
    {
      strcpy(entry.detail, "guardian unavailable; duplicate launch refused");
    }
    return false;
  }
  int listener = services::listen(path);
  if (listener < 0)
  {
    return false;
  }
  int pid = fork();
  if (!pid)
  {
    launch_guardian(index, listener, path);
  }
  close(listener);
  if (pid < 0)
  {
    unlink(path);
    strcpy(entry.detail, "cannot fork guardian");
    return false;
  }
  entry.guardian = pid;
  return refresh_guardian(index);
}

bool
Supervisor::discover_guardians() {
  char path[108];
  core_path(path);
  path[25] = 0;
  int fd = open(path, O_RDONLY);
  if (fd < 0)
  {
    return false;
  }
  DirectoryReader reader(fd);
  const linux_dirent64 *file;
  int result;
  bool okay = true;
  while ((result = reader.next(file)) > 0)
  {
    unsigned length = strlen(file->d_name);
    if (length <= 5 || strcmp(file->d_name + length - 5, ".sock"))
    {
      continue;
    }
    if (length > 36)
    {
      okay = false;
      break;
    }
    char name[32]{};
    memmove(name, file->d_name, length - 5);
    if (!valid_name(name))
    {
      okay = false;
      break;
    }
    int existing = find(name);
    if (existing >= 0)
    {
      if (entries_[existing].external)
      {
        okay = false;
        break;
      }
      continue;
    }
    if (count_ == Maximum)
    {
      okay = false;
      break;
    }
    unsigned index = count_++;
    auto &entry = entries_[index];
    entry.~Service();
    new (&entry) Service{};
    strcpy(entry.definition.name, name);
    entry.unconfigured = true;
    entry.phase = Phase::Unavailable;
    (void)refresh_guardian(index);
    entry.phase = Phase::Unavailable;
    entry.wanted = false;
    strcpy(entry.detail, "unconfigured surviving service; stop before reload");
  }
  close(fd);
  return okay && result == 0;
}

bool
Supervisor::forward(unsigned index, Command command, int argument) {
  char path[108];
  guardian_path(index, path);
  Request request;
  request.command = command;
  request.pid = argument;
  strcpy(request.name, entries_[index].definition.name);
  Reply response;
  if (!exchange(path, request, response) || response.result < 0)
  {
    return false;
  }
  return command == Command::Retire || refresh_guardian(index);
}

void
Supervisor::retire_guardians() {
  for (unsigned i = count_; i; --i)
  {
    if (!entries_[i - 1].external && entries_[i - 1].guardian)
    {
      (void)forward(i - 1, Command::Retire);
    }
  }
  auto deadline = clockticks() / 10000 + 3000;
  for (;;)
  {
    bool pending = false;
    for (unsigned i = 0; i < count_; ++i)
    {
      auto &entry = entries_[i];
      if (entry.external || !entry.guardian)
      {
        continue;
      }
      char path[108];
      guardian_path(i, path);
      struct stat present{};
      if (stat(path, &present) == 0)
      {
        pending = true;
      }
      else
      {
        (void)waitpid(entry.guardian, nullptr, WNOHANG);
        entry.guardian = entry.pid = 0;
      }
    }
    if (!pending || clockticks() / 10000 >= deadline)
    {
      break;
    }
    poll(nullptr, 0, 10);
  }
}

struct Supervisor::GuardianStart
{
  Supervisor *owner;
  unsigned index;
  int listener;
  char path[108];
};

[[noreturn]]
void
Supervisor::launch_guardian(unsigned index, int listener, const char *path) {
  constexpr unsigned Page = 4096, StackBytes = 4 * Page;
  auto *context = static_cast<GuardianStart *>(malloc(sizeof(GuardianStart)));
  auto *stack =
      static_cast<char *>(mmap(nullptr, StackBytes + Page, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  if (!context || stack == MAP_FAILED || mprotect(stack, Page, PROT_NONE) < 0)
  {
    close(listener);
    unlink(path);
    exit(1);
  }
  context->owner = this;
  context->index = index;
  context->listener = listener;
  strcpy(context->path, path);
  // A fork inside dispatch otherwise retains every manager frame. An anonymous
  // mapping gives the guardian its own bounded stack and an inaccessible guard.
  xv_user_enter(stack + Page + StackBytes, guardian_entry, context);
}

[[noreturn]]
void
Supervisor::guardian_entry(void *argument) {
  auto &context = *static_cast<GuardianStart *>(argument);
  context.owner->run_guardian(context.index, context.listener, context.path);
}

[[noreturn]]
void
Supervisor::run_guardian(unsigned index, int listener, const char *path) {
  if (index)
  {
    entries_[0] = entries_[index];
  }
  count_ = 1;
  local_ = true;
  permitted_ = false;
  entries_[0].guardian = getpid();
  for (int fd = 0; fd < 16; ++fd)
  {
    if (fd != listener)
    {
      close(fd);
    }
  }
  if (open("/dev/null", O_RDWR) != 0 || dup(0) != 1 || dup(0) != 2 || setsid() < 0)
  {
    unlink(path);
    exit(1);
  }
  guardian_stopping = 0;
  struct sigaction stop{stop_guardian, 0, 0};
  sigaction(SIGTERM, &stop, nullptr);
  sigaction(SIGINT, &stop, nullptr);
  signal(SIGPIPE, SIG_IGN);
  while (!retiring_ && !guardian_stopping)
  {
    tick();
    pollfd watched[2]{{listener, POLLIN, 0}, {entries_[0].output, POLLIN | POLLHUP, 0}};
    (void)poll(watched, 2, 20);
    if (watched[0].revents & POLLIN)
    {
      for (unsigned budget = 0; budget < 8; ++budget)
      {
        Request query;
        int client = accept_request(listener, query);
        if (client < 0)
        {
          break;
        }
        Reply response;
        request(query, response);
        services::reply(client, response);
      }
    }
  }
  shutdown();
  close_log(entries_[0]);
  close(listener);
  unlink(path);
  exit(0);
}
} // namespace services
