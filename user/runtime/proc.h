#pragma once
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
namespace proc
{
inline int read(const char *path, char *buffer, unsigned capacity)
{
  int fd = open(path, O_RDONLY);
  if (fd < 0) return -1;
  unsigned used = 0;
  while (used + 1 < capacity)
  {
    int count = ::read(fd, buffer + used, capacity - used - 1);
    if (count < 0) { close(fd); return -1; }
    if (!count) break;
    used += count;
  }
  close(fd); buffer[used] = 0; return used;
}
inline char *field(char *text, const char *name)
{
  unsigned length = strlen(name);
  for (char *line = text; line && *line;)
  {
    unsigned equal = 0;
    while (equal < length && line[equal] && line[equal] == name[equal]) ++equal;
    if (equal == length && line[length] == ':')
    { char *value = line + length + 1; while (*value == ' ') ++value; return value; }
    char *next = strchr(line, '\n'); line = next ? next + 1 : nullptr;
  }
  return nullptr;
}
inline int number(char *text, const char *name)
{ char *value = field(text, name); return value ? atoi(value) : -1; }
inline void path(int pid, char *output)
{
  strcpy(output, "/proc/");
  char digits[12]; unsigned count = 0;
  do { digits[count++] = '0' + pid % 10; pid /= 10; } while (pid);
  unsigned used = 6;
  while (count) output[used++] = digits[--count];
  strcpy(output + used, "/status");
}
}
