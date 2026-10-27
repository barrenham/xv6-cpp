#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"

namespace shell
{
static bool same_directory(const struct stat &left, const struct stat &right)
{
  return left.dev == right.dev && left.ino == right.ino;
}

static bool current_path(char *output)
{
  char ancestor[MAXPATH] = ".";
  unsigned length = 1, used = 0;
  output[0] = 0;
  struct stat current, root;
  if (stat(".", &current) < 0 || stat("/", &root) < 0) return false;
  while (!same_directory(current, root))
  {
    if (length + 3 + 1 + DIRSIZ + 1 > sizeof(ancestor)) return false;
    strcpy(ancestor + length, "/..");
    length += 3;
    struct stat parent;
    int fd = open(ancestor, O_RDONLY);
    if (fd < 0) return false;
    if (fstat(fd, &parent) < 0 || same_directory(current, parent))
    { close(fd); return false; }
    bool found = false;
    struct dirent entry;
    while (read(fd, &entry, sizeof(entry)) == sizeof(entry))
    {
      if (!entry.inum) continue;
      char name[DIRSIZ + 1];
      memmove(name, entry.name, DIRSIZ); name[DIRSIZ] = 0;
      if (!strcmp(name, ".") || !strcmp(name, "..")) continue;
      ancestor[length] = '/';
      strcpy(ancestor + length + 1, name);
      struct stat candidate;
      if (stat(ancestor, &candidate) < 0 || !same_directory(candidate, current)) continue;
      unsigned size = strlen(name);
      if (used + size + 1 >= MAXPATH) break;
      memmove(output + size + 1, output, used + 1);
      output[0] = '/'; memmove(output + 1, name, size);
      used += size + 1;
      found = true;
      break;
    }
    close(fd);
    ancestor[length] = 0;
    if (!found) return false;
    current = parent;
  }
  if (!used) strcpy(output, "/");
  return true;
}

void prompt()
{
  char output[MAXPATH + 32], path[MAXPATH];
  account::User user{};
  unsigned uid = geteuid();
  if (account::find(uid, user)) strcpy(output, user.name);
  else
  {
    char digits[12]; unsigned count = 0, used = 0;
    do { digits[count++] = '0' + uid % 10; uid /= 10; } while (uid);
    while (count) output[used++] = digits[--count];
    output[used] = 0;
  }
  unsigned used = strlen(output);
  output[used++] = ':';
  if (!current_path(path)) strcpy(path, "?");
  strcpy(output + used, path); used += strlen(path);
  strcpy(output + used, "$ "); used += 2;
  write(2, output, used);
}
}
