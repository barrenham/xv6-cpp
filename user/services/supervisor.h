#pragma once
#include "config.h"

namespace services
{
class Supervisor
{
public:

  bool configure(const char *socket);
  bool load(const char *, char error[96]);
  void tick();
  void request(const Request &, Reply &);
  void shutdown();

private:

  struct Service
  {
    Definition definition;
    Phase phase = Phase::Stopped;
    int pid = 0, status = 0, ready = -1;
    unsigned restarts = 0;
    unsigned long deadline = 0;
    bool wanted = false, external = false, restart_after_stop = false;
    bool unconfigured = false;
    char detail[96]{};
    int guardian = 0, output = -1, log_fd = -1;
    unsigned long log_bytes = 0;
    unsigned long log_captured = 0, log_rotations = 0;
  } entries_[Maximum]{};

  unsigned count_ = 0, tick_cursor_ = 0;
  char socket_[128]{};
  bool local_ = false, permitted_ = false, retiring_ = false;
  void guardian_path(unsigned, char *) const;
  void core_path(char *) const;
  bool restore_external();
  bool save_external();
  bool discover_guardians();
  bool ensure_guardian(unsigned);
  bool forward(unsigned, Command, int argument = 0);
  bool refresh_guardian(unsigned);
  void retire_guardians();
  struct GuardianStart;
  [[noreturn]]
  void launch_guardian(unsigned, int listener, const char *path);
  [[noreturn]]
  static void guardian_entry(void *);
  [[noreturn]]
  void run_guardian(unsigned, int listener, const char *path);
  static bool open_log(Service &);
  static bool rotate_log(Service &);
  static bool drain_log(Service &, unsigned budget = 16384);
  static void close_log(Service &);
  friend int open_manager_log();
  int find(const char *) const;
  bool activate(unsigned, bool manual, unsigned depth = 0);
  bool dependencies(unsigned) const;
  void spawn(unsigned);
  void stop(unsigned, bool restart = false, unsigned depth = 0);
  void exited(Service &, int status);
  void describe(unsigned, Reply &) const;
};

int run(const char *config, const char *socket, int ready_fd = -1);
} // namespace services
