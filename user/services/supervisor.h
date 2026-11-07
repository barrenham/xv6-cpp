#pragma once
#include "config.h"

namespace services
{
class Supervisor
{
public:

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
    char detail[96]{};
  } entries_[Maximum]{};

  unsigned count_ = 0;
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
