#include "kernel/support/types.h"
#include "kernel/fs/image_paths.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "accounts.h"

namespace account
{
namespace
{
constexpr unsigned Capacity = 16384;

struct Text
{
  char *data = static_cast<char *>(malloc(Capacity));
  unsigned used = 0;
  bool good = data != nullptr;

  Text()
  {
    if (data)
    {
      data[0] = 0;
    }
  }

  ~Text()
  {
    free(data);
  }

  bool append(const char *text)
  {
    unsigned size = strlen(text);
    if (!good || size >= Capacity - used)
    {
      return good = false;
    }
    memmove(data + used, text, size + 1);
    used += size;
    return true;
  }

  bool read(const char *leaf, bool optional = false)
  {
    if (!good)
    {
      return false;
    }
    char filename[128];
    path(leaf, filename);
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
      return optional;
    }
    while (good && used < Capacity - 1)
    {
      int n = ::read(fd, data + used, Capacity - used - 1);
      if (n < 0)
      {
        good = false;
        break;
      }
      if (!n)
      {
        break;
      }
      for (int i = 0; i < n; ++i)
      {
        if (!data[used + i])
        {
          good = false;
        }
      }
      used += n;
    }
    char extra;
    if (used == Capacity - 1 && ::read(fd, &extra, 1) != 0)
    {
      good = false;
    }
    close(fd);
    data[used] = 0;
    return good;
  }

  bool save(const char *leaf, unsigned mode)
  {
    if (!good)
    {
      return false;
    }
    char filename[128], temporary[128];
    path(leaf, filename);
    path(".acct.new", temporary);
    unsigned mask = umask(0077);
    int fd = open(temporary, O_CREATE | O_WRONLY | O_TRUNC);
    umask(mask);
    bool result = fd >= 0 && chmod(temporary, mode) == 0;
    unsigned done = 0;
    while (result && done < used)
    {
      int n = write(fd, data + done, used - done);
      if (n <= 0)
      {
        result = false;
      }
      else
      {
        done += n;
      }
    }
    if (result)
    {
      result = fsync(fd) == 0;
    }
    if (fd >= 0)
    {
      close(fd);
    }
    if (result)
    {
      result = rename(temporary, filename) == 0;
    }
    if (!result)
    {
      unlink(temporary);
    }
    return result;
  }
};

struct Lock
{
  char name[128];
  bool held;

  Lock()
  {
    path(".users.lock", name);
    held = geteuid() == 0 && mkdir(name) == 0;
  }

  ~Lock()
  {
    if (held)
    {
      unlink(name);
    }
  }
};

bool valid_name(const char *name)
{
  if (!name || !*name || strlen(name) >= 14)
  {
    return false;
  }
  for (unsigned i = 0; name[i]; ++i)
  {
    if (!((name[i] >= 'a' && name[i] <= 'z') || (name[i] >= 'A' && name[i] <= 'Z') ||
          name[i] == '_' ||
          (i && ((name[i] >= '0' && name[i] <= '9') || name[i] == '-'))))
    {
      return false;
    }
  }
  return true;
}

void number(unsigned value, char *output)
{
  char reversed[12];
  unsigned n = 0, used = 0;
  do
  {
    reversed[n++] = '0' + value % 10;
    value /= 10;
  } while (value);
  while (n)
  {
    output[used++] = reversed[--n];
  }
  output[used] = 0;
}

char *next(char *&cursor)
{
  if (!*cursor)
  {
    return nullptr;
  }
  char *line = cursor, *end = strchr(cursor, '\n');
  if (end)
  {
    *end = 0;
    cursor = end + 1;
  }
  else
  {
    cursor += strlen(cursor);
  }
  return line;
}

bool member(const char *list, const char *name)
{
  unsigned size = strlen(name);
  while (*list)
  {
    const char *end = list;
    while (*end && *end != ',')
    {
      ++end;
    }
    if (static_cast<unsigned>(end - list) == size && !memcmp(list, name, size))
    {
      return true;
    }
    list = *end ? end + 1 : end;
  }
  return false;
}

bool group_fields(char *line, char *&name, unsigned &gid, char *&members)
{
  name = line;
  char *password = strchr(line, ':'),
       *id = password ? strchr(password + 1, ':') : nullptr;
  members = id ? strchr(id + 1, ':') : nullptr;
  if (!password || !id || !members)
  {
    return false;
  }
  *password = *id = *members++ = 0;
  return valid_name(name) && numeric(id + 1, gid) && !strchr(members, ':');
}

bool gid_exists(unsigned gid)
{
  Text text;
  if (!text.read("group"))
  {
    return true;
  }
  char *cursor = text.data;
  while (char *line = next(cursor))
  {
    char *name, *members;
    unsigned id;
    if (!group_fields(line, name, id, members))
    {
      return true;
    }
    if (id == gid)
    {
      return true;
    }
  }
  return false;
}

int reserve_id()
{
  Text counter;
  if (!counter.read("idnext", true))
  {
    return -1;
  }
  unsigned value = 1000;
  if (counter.used)
  {
    if (counter.data[counter.used - 1] == '\n')
    {
      counter.data[--counter.used] = 0;
    }
    if (!numeric(counter.data, value))
    {
      return -1;
    }
  }
  User user{};
  while (value < 65534 && (find(value, user) || gid_exists(value)))
  {
    ++value;
  }
  if (value == 65534)
  {
    return -1;
  }
  Text next_value;
  char digits[12];
  number(value + 1, digits);
  next_value.append(digits);
  next_value.append("\n");
  return next_value.save("idnext", 0600) ? static_cast<int>(value) : -1;
}

bool filter(const char *leaf, const char *name, unsigned mode, bool optional = false)
{
  Text input, output;
  if (!input.read(leaf, optional))
  {
    return false;
  }
  char *cursor = input.data;
  while (char *line = next(cursor))
  {
    unsigned size = strlen(name);
    if (strlen(line) > size && !memcmp(line, name, size) && line[size] == ':')
    {
      continue;
    }
    output.append(line);
    output.append("\n");
  }
  return output.save(leaf, mode);
}

bool rewrite_members(const char *user, const char *list, bool append, bool deleting)
{
  Text input, output;
  if (!input.read("group"))
  {
    return false;
  }
  char *cursor = input.data;
  while (char *line = next(cursor))
  {
    char *name, *members;
    unsigned gid;
    if (!group_fields(line, name, gid, members))
    {
      return false;
    }
    bool wanted = !deleting && member(list, name), was = member(members, user);
    char digits[12];
    number(gid, digits);
    output.append(name);
    output.append(":!:");
    output.append(digits);
    output.append(":");
    bool first = true;
    for (char *p = members; *p;)
    {
      char *end = strchr(p, ',');
      if (end)
      {
        *end = 0;
      }
      if (strcmp(p, user))
      {
        if (!first)
        {
          output.append(",");
        }
        output.append(p);
        first = false;
      }
      if (!end)
      {
        break;
      }
      p = end + 1;
    }
    if (wanted || (append && was && !deleting))
    {
      if (!first)
      {
        output.append(",");
      }
      output.append(user);
    }
    output.append("\n");
  }
  return output.save("group", 0644);
}
} // namespace

bool initialize_groups()
{
  char filename[128];
  path("group", filename);
  int fd = open(filename, O_RDONLY);
  if (fd >= 0)
  {
    close(fd);
    return chown(filename, 0, 0) == 0 && chmod(filename, 0644) == 0;
  }
  Text data;
  data.append("root:!:0:\nnobody:!:65534:\n");
  return data.save("group", 0644);
}

bool group(const char *target, unsigned &gid)
{
  Text text;
  if (!text.read("group"))
  {
    return false;
  }
  char *cursor = text.data;
  while (char *line = next(cursor))
  {
    char *name, *members;
    unsigned value;
    if (!group_fields(line, name, value, members))
    {
      return false;
    }
    if (!strcmp(target, name))
    {
      gid = value;
      return true;
    }
  }
  return false;
}

int groups(const User &user, unsigned *output)
{
  Text text;
  if (!text.read("group"))
  {
    return -1;
  }
  char *cursor = text.data;
  int count = 0;
  while (char *line = next(cursor))
  {
    char *name, *members;
    unsigned gid;
    if (!group_fields(line, name, gid, members))
    {
      return -1;
    }
    if (gid != user.gid && member(members, user.name))
    {
      if (count == 16)
      {
        return -1;
      }
      output[count++] = gid;
    }
  }
  return count;
}

int group_add(const char *name)
{
  if (!valid_name(name))
  {
    return -1;
  }
  Lock lock;
  if (!lock.held)
  {
    return -1;
  }
  unsigned ignored;
  if (group(name, ignored))
  {
    return -1;
  }
  int reserved = reserve_id();
  if (reserved < 0)
  {
    return -1;
  }
  unsigned gid = reserved;
  Text text;
  if (!text.read("group"))
  {
    return -1;
  }
  char digits[12];
  number(gid, digits);
  text.append(name);
  text.append(":!:");
  text.append(digits);
  text.append(":\n");
  return text.save("group", 0644) ? gid : -1;
}

int group_remove(const char *name)
{
  Lock lock;
  unsigned gid;
  if (!lock.held || !group(name, gid) || gid == 0)
  {
    return -1;
  }
  Text accounts;
  if (!accounts.read("passwd"))
  {
    return -1;
  }
  char *cursor = accounts.data;
  while (char *line = next(cursor))
  {
    char *p = line;
    for (unsigned i = 0; i < 3 && p; ++i)
    {
      p = strchr(p, ':') ? strchr(p, ':') + 1 : nullptr;
    }
    if (!p)
    {
      return -1;
    }
    char *end = strchr(p, ':');
    if (!end)
    {
      return -1;
    }
    *end = 0;
    unsigned primary;
    if (!numeric(p, primary) || primary == gid)
    {
      return -1;
    }
  }
  return filter("group", name, 0644) ? 0 : -1;
}

int memberships(const char *name, const char *list, bool append)
{
  Lock lock;
  User user{};
  if (!lock.held || !find(name, user))
  {
    return -1;
  }
  Text existing;
  if (!existing.read("group"))
  {
    return -1;
  }
  unsigned requested = 0;
  const char *p = list;
  while (*p)
  {
    char item[14];
    unsigned n = 0;
    while (*p && *p != ',')
    {
      if (n == 13)
      {
        return -1;
      }
      item[n++] = *p++;
    }
    item[n] = 0;
    unsigned gid;
    if (!n || !group(item, gid) || ++requested > 16)
    {
      return -1;
    }
    if (*p)
    {
      ++p;
      if (!*p)
      {
        return -1;
      }
    }
  }
  if (append)
  {
    unsigned current[16];
    int count = groups(user, current);
    if (count < 0 || count + requested > 16)
    {
      return -1;
    }
  }
  return rewrite_members(name, list, append, false) ? 0 : -1;
}

int remove(const char *name)
{
  Lock lock;
  User user{};
  if (!lock.held || !find(name, user) || user.uid == 0)
  {
    return -1;
  }
  if (!filter("passwd", name, 0644) || !filter("shadow", name, 0600, true) ||
      !rewrite_members(name, "", false, true))
  {
    return -1;
  }
  return 0; // Keep the home directory and never reuse its UID.
}

int add(const char *name)
{
  if (!valid_name(name))
  {
    return -1;
  }
  Lock lock;
  User old{};
  if (!lock.held || find(name, old))
  {
    return -1;
  }
  int reserved = reserve_id();
  if (reserved < 0)
  {
    return -1;
  }
  unsigned uid = reserved;
  char digits[12];
  char home[64];
  strcpy(home, base());
  strcpy(home + strlen(home), "/home/");
  strcpy(home + strlen(home), name);
  if (mkdir(home) < 0 || chown(home, uid, uid) < 0 || chmod(home, 0750) < 0)
  {
    return -1;
  }
  Text users, group_data;
  if (!users.read("passwd") || !group_data.read("group"))
  {
    return -1;
  }
  number(uid, digits);
  users.append(name);
  users.append(":!:");
  users.append(digits);
  users.append(":");
  users.append(digits);
  users.append(":");
  users.append(name);
  users.append(":");
  users.append(home);
  users.append(":");
  users.append(XV_SHELL_PATH);
  users.append("\n");
  unsigned gid;
  if (group(name, gid))
  {
    return -1;
  }
  group_data.append(name);
  group_data.append(":!:");
  group_data.append(digits);
  group_data.append(":\n");
  if (!group_data.save("group", 0644) || !users.save("passwd", 0644))
  {
    return -1;
  }
  return uid;
}
} // namespace account
