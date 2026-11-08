#include "supervisor.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"

namespace services
{
int
open_manager_log() {
  Supervisor::Service entry;
  strcpy(entry.definition.log, "/tmp/serviced.log");
  if (!Supervisor::open_log(entry))
  {
    return -1;
  }
  if (entry.log_bytes + 1024 > entry.definition.log_max_bytes &&
      !Supervisor::rotate_log(entry))
  {
    Supervisor::close_log(entry);
    return -1;
  }
  return entry.log_fd;
}

bool
Supervisor::open_log(Service &entry) {
  if (entry.log_fd >= 0)
  {
    return true;
  }
  entry.log_fd =
      open(entry.definition.log, O_CREATE | O_WRONLY | O_APPEND | O_NOFOLLOW);
  struct stat metadata{};
  if (entry.log_fd < 0 || fstat(entry.log_fd, &metadata) < 0 || metadata.type != T_FILE)
  {
    if (entry.log_fd >= 0)
    {
      close(entry.log_fd);
    }
    entry.log_fd = -1;
    return false;
  }
  entry.log_bytes = metadata.size;
  return true;
}

bool
Supervisor::rotate_log(Service &entry) {
  char from[136], to[136];
  unsigned length = strlen(entry.definition.log);
  strcpy(from, entry.definition.log);
  strcpy(to, entry.definition.log);
  to[length] = '.';
  to[length + 1] = '0' + entry.definition.log_backups;
  to[length + 2] = 0;
  struct stat existing{};
  if (lstat(to, &existing) == 0 && unlink(to) < 0)
  {
    return false;
  }
  for (unsigned n = entry.definition.log_backups; n > 1; --n)
  {
    from[length] = '.';
    from[length + 1] = '0' + n - 1;
    from[length + 2] = 0;
    to[length + 1] = '0' + n;
    if (lstat(from, &existing) == 0 && rename(from, to) < 0)
    {
      return false;
    }
  }
  from[length] = 0;
  to[length + 1] = '1';
  if (rename(from, to) < 0)
  {
    return false;
  }
  close(entry.log_fd);
  entry.log_fd = -1;
  if (!open_log(entry))
  {
    return false;
  }
  ++entry.log_rotations;
  return true;
}

bool
Supervisor::drain_log(Service &entry, unsigned budget) {
  char buffer[1024];
  while (entry.output >= 0 && budget)
  {
    unsigned take = budget < sizeof(buffer) ? budget : sizeof(buffer);
    int count = read(entry.output, buffer, take);
    if (count < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      return errno == EAGAIN;
    }
    if (!count)
    {
      close(entry.output);
      entry.output = -1;
      return true;
    }
    budget -= count;
    unsigned done = 0;
    while (done < unsigned(count))
    {
      if (!open_log(entry) ||
          (entry.log_bytes >= entry.definition.log_max_bytes && !rotate_log(entry)))
      {
        return false;
      }
      unsigned remaining = entry.definition.log_max_bytes - entry.log_bytes;
      unsigned bytes = unsigned(count) - done;
      if (bytes > remaining)
      {
        bytes = remaining;
      }
      int written = write(entry.log_fd, buffer + done, bytes);
      if (written <= 0)
      {
        return false;
      }
      done += written;
      entry.log_bytes += written;
      entry.log_captured += written;
    }
  }
  return true;
}

void
Supervisor::close_log(Service &entry) {
  if (entry.output >= 0)
  {
    close(entry.output);
  }
  if (entry.log_fd >= 0)
  {
    close(entry.log_fd);
  }
  entry.output = entry.log_fd = -1;
}
} // namespace services
