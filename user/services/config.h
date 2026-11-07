#pragma once
#include "protocol.h"

namespace services
{
enum class Restart : unsigned
{
  Never,
  Failure,
  Always
};

struct Definition
{
  char name[32]{}, arguments[8][128]{}, dependencies[8][32]{}, log[128]{};
  unsigned argc = 0, dependency_count = 0, delay = 250, limit = 5, ready_timeout = 5000;
  Restart restart = Restart::Failure;
  bool enabled = true, oneshot = false, notify = false;
};

struct Configuration
{
  Definition entries[ManagedMaximum]{};
  unsigned count = 0;
};

bool valid_name(const char *);
bool load_configuration(const char *path, Configuration &, char error[96]);
} // namespace services
