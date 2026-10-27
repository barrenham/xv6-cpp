#include "kernel/fs/image_paths.h"
#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "accounts.h"
#include "kernel/fs/stat.h"

namespace account
{
bool numeric(const char *text, unsigned &value)
{
  if (!text || !*text)
  {
    return false;
  }
  value = 0;
  while (*text)
  {
    if (*text < '0' || *text > '9' || value > (65534U - (*text - '0')) / 10)
    {
      return false;
    }
    value = value * 10 + *text++ - '0';
  }
  return true;
}

static bool next(int fd, User &user)
{
  char line[256];
  unsigned used = 0;
  for (;;)
  {
    char value;
    int count = read(fd, &value, 1);
    if (count < 0)
    {
      return false;
    }
    if (!count)
    {
      if (!used)
      {
        return false;
      }
      break;
    }
    if (value == '\n')
    {
      break;
    }
    if (used == sizeof(line) - 1)
    {
      return false;
    }
    line[used++] = value;
  }
  line[used] = 0;
  char *fields[7] = {line};
  unsigned count = 1;
  for (unsigned i = 0; i < used; ++i)
  {
    if (line[i] == ':')
    {
      line[i] = 0;
      if (count == 7)
      {
        return false;
      }
      fields[count++] = line + i + 1;
    }
  }
  if (count != 7 || !*fields[0] || strlen(fields[0]) >= sizeof(user.name) ||
      !numeric(fields[2], user.uid) || !numeric(fields[3], user.gid) ||
      strlen(fields[5]) >= sizeof(user.home) ||
      strlen(fields[6]) >= sizeof(user.shell) || fields[5][0] != '/' ||
      fields[6][0] != '/')
  {
    return false;
  }
  strcpy(user.name, fields[0]);
  strcpy(user.home, fields[5]);
  strcpy(user.shell, fields[6]);
  return true;
}

static bool lookup(const char *name, unsigned uid, bool by_id, User &user)
{
  char database[128];
  path("passwd", database);
  int fd = open(database, O_RDONLY);
  if (fd < 0)
  {
    return false;
  }
  bool found = false;
  while (next(fd, user))
  {
    if (by_id ? user.uid == uid : !strcmp(user.name, name))
    {
      found = true;
      break;
    }
  }
  close(fd);
  return found;
}

bool find(const char *name, User &user)
{
  return lookup(name, 0, false, user);
}

bool find(unsigned uid, User &user)
{
  return lookup(nullptr, uid, true, user);
}

const char *base()
{
  return "";
}

void path(const char *leaf, char *output)
{
  strcpy(output, base());
  strcpy(output + strlen(output), "/etc/");
  strcpy(output + strlen(output), leaf);
}

bool initialize()
{
  if (geteuid() != 0)
  {
    return false;
  }
  char directory[128], home[64], root[64], database[128];
  strcpy(directory, base());
  strcpy(directory + strlen(directory), "/etc");
  strcpy(home, base());
  strcpy(home + strlen(home), "/home");
  strcpy(root, base());
  strcpy(root + strlen(root), "/root");
  mkdir(directory);
  mkdir(home);
  mkdir(root);
  if (chown(directory, 0, 0) < 0 || chmod(directory, 0755) < 0 ||
      chown(home, 0, 0) < 0 || chmod(home, 0755) < 0 || chown(root, 0, 0) < 0 ||
      chmod(root, 0700) < 0)
  {
    return false;
  }
  path("passwd", database);
  int fd = open(database, O_RDONLY);
  if (fd < 0)
  {
    fd = open(database, O_CREATE | O_WRONLY);
    char data[256];
    strcpy(data, "root:!:0:0:root:");
    strcpy(data + strlen(data), root);
    strcpy(data + strlen(data),
           ":" XV_SHELL_PATH "\nnobody:!:65534:65534:nobody:/tmp:" XV_SHELL_PATH "\n");
    if (fd < 0)
    {
      return false;
    }
    bool written = write(fd, data, strlen(data)) == static_cast<int>(strlen(data)) &&
                   fsync(fd) == 0;
    close(fd);
    if (!written)
    {
      return false;
    }
  }
  else
  {
    close(fd);
  }
  return chown(database, 0, 0) == 0 && chmod(database, 0644) == 0 &&
         initialize_groups();
}

} // namespace account
