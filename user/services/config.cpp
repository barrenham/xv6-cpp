#include "config.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/support/placement_new.hpp"

namespace services
{
bool
valid_name(const char *name) {
  unsigned used = 0;
  for (; name[used]; ++used)
  {
    char c = name[used];
    if (used == 31 || !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                        (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.'))
    {
      return false;
    }
  }
  return used != 0;
}

namespace
{
char *
trim(char *text) {
  while (*text == ' ' || *text == '\t' || *text == '\r')
  {
    ++text;
  }
  unsigned n = strlen(text);
  while (n && (text[n - 1] == ' ' || text[n - 1] == '\t' || text[n - 1] == '\r'))
  {
    text[--n] = 0;
  }
  return text;
}

bool
number(const char *text, unsigned &out, unsigned maximum) {
  if (!*text)
  {
    return false;
  }
  unsigned value = 0;
  for (; *text; ++text)
  {
    if (*text < '0' || *text > '9' || value > maximum / 10)
    {
      return false;
    }
    value = value * 10 + *text - '0';
    if (value > maximum)
    {
      return false;
    }
  }
  out = value;
  return true;
}

bool
boolean(const char *text, bool &out) {
  if (!strcmp(text, "yes"))
  {
    out = true;
  }
  else if (!strcmp(text, "no"))
  {
    out = false;
  }
  else
  {
    return false;
  }
  return true;
}

bool
words(char *text, char output[][128], unsigned &count) {
  count = 0;
  while (*(text = trim(text)))
  {
    if (count == 8)
    {
      return false;
    }
    char quote = 0;
    unsigned n = 0;
    while (*text && (quote || (*text != ' ' && *text != '\t')))
    {
      char c = *text++;
      if (c == '\\')
      {
        if (!*text)
        {
          return false;
        }
        c = *text++;
      }
      else if (c == '\'' || c == '"')
      {
        if (!quote)
        {
          quote = c;
          continue;
        }
        if (quote == c)
        {
          quote = 0;
          continue;
        }
      }
      if (n == 127)
      {
        return false;
      }
      output[count][n++] = c;
    }
    if (quote || !n)
    {
      return false;
    }
    output[count++][n] = 0;
  }
  return count != 0;
}

bool
visit(const Configuration &config, unsigned index, unsigned char *marks) {
  if (marks[index] == 1)
  {
    return false;
  }
  if (marks[index] == 2)
  {
    return true;
  }
  marks[index] = 1;
  auto &entry = config.entries[index];
  for (unsigned d = 0; d < entry.dependency_count; ++d)
  {
    // External core dependencies are resolved by the supervisor at activation.
    for (unsigned i = 0; i < config.count; ++i)
    {
      if (!strcmp(entry.dependencies[d], config.entries[i].name) &&
          !visit(config, i, marks))
      {
        return false;
      }
    }
  }
  marks[index] = 2;
  return true;
}
} // namespace

bool
load_configuration(const char *path, Configuration &out, char error[96]) {
  strcpy(error, "invalid configuration");
  int fd = open(path, O_RDONLY);
  if (fd < 0)
  {
    strcpy(error, "cannot open configuration");
    return false;
  }
  constexpr unsigned Capacity = 32768;
  auto *text = static_cast<char *>(malloc(Capacity + 1));
  if (!text)
  {
    close(fd);
    return false;
  }
  unsigned used = 0;
  int count = 0;
  while (used < Capacity && (count = read(fd, text + used, Capacity - used)) > 0)
  {
    used += count;
  }
  close(fd);
  bool okay = count >= 0 && used < Capacity;
  text[used] = 0;
  out.count = 0;
  Definition *current = nullptr;
  for (char *line = text; okay && *line;)
  {
    char *next = line;
    while (*next && *next != '\n')
    {
      ++next;
    }
    if (*next)
    {
      *next++ = 0;
    }
    line = trim(line);
    if (!*line || *line == '#')
    {
      line = next;
      continue;
    }
    unsigned length = strlen(line);
    if (*line == '[' && length > 2 && line[length - 1] == ']')
    {
      line[length - 1] = 0;
      ++line;
      if (!valid_name(line) || out.count == ManagedMaximum)
      {
        okay = false;
        break;
      }
      for (unsigned i = 0; i < out.count; ++i)
      {
        if (!strcmp(out.entries[i].name, line))
        {
          okay = false;
        }
      }
      current = &out.entries[out.count++];
      current->~Definition();
      new (current) Definition{};
      strcpy(current->name, line);
      strcpy(current->log, "/tmp/service-");
      strcpy(current->log + strlen(current->log), line);
      strcpy(current->log + strlen(current->log), ".log");
    }
    else
    {
      char *value = strchr(line, '=');
      if (!current || !value)
      {
        okay = false;
        break;
      }
      *value++ = 0;
      char *key = trim(line);
      value = trim(value);
      if (!strcmp(key, "exec"))
      {
        okay = words(value, current->arguments, current->argc) &&
               current->arguments[0][0] == '/';
      }
      else if (!strcmp(key, "after") || !strcmp(key, "requires"))
      {
        current->dependency_count = 0;
        for (char *word = value; *word;)
        {
          char *end = word;
          while (*end && *end != ' ' && *end != '\t')
          {
            ++end;
          }
          if (*end)
          {
            *end++ = 0;
          }
          if (current->dependency_count == 8 || !valid_name(word))
          {
            okay = false;
            break;
          }
          strcpy(current->dependencies[current->dependency_count++], word);
          word = trim(end);
        }
      }
      else if (!strcmp(key, "enabled"))
      {
        okay = boolean(value, current->enabled);
      }
      else if (!strcmp(key, "oneshot"))
      {
        okay = boolean(value, current->oneshot);
      }
      else if (!strcmp(key, "ready"))
      {
        current->notify = !strcmp(value, "notify");
        okay = current->notify || !strcmp(value, "exec");
      }
      else if (!strcmp(key, "restart"))
      {
        if (!strcmp(value, "never"))
        {
          current->restart = Restart::Never;
        }
        else if (!strcmp(value, "on-failure"))
        {
          current->restart = Restart::Failure;
        }
        else if (!strcmp(value, "always"))
        {
          current->restart = Restart::Always;
        }
        else
        {
          okay = false;
        }
      }
      else if (!strcmp(key, "restart-delay-ms"))
      {
        okay = number(value, current->delay, 30000) && current->delay >= 10;
      }
      else if (!strcmp(key, "restart-limit"))
      {
        okay = number(value, current->limit, 100);
      }
      else if (!strcmp(key, "ready-timeout-ms"))
      {
        okay = number(value, current->ready_timeout, 60000) &&
               current->ready_timeout >= 10;
      }
      else if (!strcmp(key, "log"))
      {
        okay = value[0] == '/' && strlen(value) < sizeof(current->log);
        if (okay)
        {
          strcpy(current->log, value);
        }
      }
      else
      {
        okay = false;
      }
    }
    line = next;
  }
  free(text);
  unsigned char marks[ManagedMaximum]{};
  for (unsigned i = 0; okay && i < out.count; ++i)
  {
    okay = out.entries[i].argc && visit(out, i, marks);
  }
  if (okay)
  {
    error[0] = 0;
  }
  return okay;
}
} // namespace services
