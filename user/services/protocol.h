#pragma once

namespace services
{
inline constexpr const char *DefaultSocket = "/tmp/serviced.sock";
inline constexpr unsigned Version = 2, Maximum = 24, ManagedMaximum = 16;
enum class Command : unsigned
{
  List,
  Status,
  Start,
  Stop,
  Restart,
  Load,
  Register,
  DependencyState,
  Retire
};
enum class Phase : unsigned
{
  Stopped,
  Waiting,
  Starting,
  Running,
  Stopping,
  Backoff,
  Complete,
  Failed,
  Unavailable
};

struct Request
{
  unsigned version = Version;
  Command command = Command::List;
  unsigned index = 0;
  int pid = 0;
  char name[32]{}, path[128]{};
};

struct Reply
{
  unsigned version = Version;
  int result = -1, pid = 0, status = 0;
  unsigned restarts = 0;
  Phase phase = Phase::Stopped;
  bool external = false;
  bool wanted = false;
  int guardian = 0;
  unsigned long definition_id = 0;
  unsigned long log_captured = 0, log_rotations = 0;
  char name[32]{}, log[128]{}, detail[96]{};
  char guardian_socket[108]{};
};

const char *phase_name(Phase);
bool exchange(const char *socket, const Request &, Reply &);
int listen(const char *socket);
int accept_request(int listener, Request &);
bool reply(int client, const Reply &);
int launch_daemon();
int open_manager_log();
bool register_external(const char *name, int pid);
} // namespace services
