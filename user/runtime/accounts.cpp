#include "kernel/fs/image_paths.h"
#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "accounts.h"
namespace account
{
bool numeric(const char *text, unsigned &value)
{
  if (!text || !*text) return false;
  value = 0;
  while (*text)
  { if (*text < '0' || *text > '9' || value > (65534U - (*text - '0')) / 10) return false; value = value * 10 + *text++ - '0'; }
  return true;
}
static bool next(int fd, User &user)
{
  char line[256]; unsigned used = 0;
  for (;;)
  {
    char value; int count = read(fd, &value, 1);
    if (count < 0) return false;
    if (!count) { if (!used) return false; break; }
    if (value == '\n') break;
    if (used == sizeof(line) - 1) return false;
    line[used++] = value;
  }
  line[used] = 0; char *fields[7] = {line}; unsigned count = 1;
  for (unsigned i = 0; i < used; ++i) if (line[i] == ':')
  { line[i] = 0; if (count == 7) return false; fields[count++] = line + i + 1; }
  if (count != 7 || !*fields[0] || strlen(fields[0]) >= sizeof(user.name) ||
      !numeric(fields[2], user.uid) || !numeric(fields[3], user.gid) ||
      strlen(fields[5]) >= sizeof(user.home) || strlen(fields[6]) >= sizeof(user.shell) ||
      fields[5][0] != '/' || fields[6][0] != '/') return false;
  strcpy(user.name, fields[0]); strcpy(user.home, fields[5]); strcpy(user.shell, fields[6]); return true;
}
static bool lookup(const char *name, unsigned uid, bool by_id, User &user)
{
  int fd = open("/etc/passwd", O_RDONLY); if (fd < 0) return false;
  bool found = false;
  while (next(fd, user)) if (by_id ? user.uid == uid : !strcmp(user.name, name)) { found = true; break; }
  close(fd); return found;
}
bool find(const char *name, User &user) { return lookup(name, 0, false, user); }
bool find(unsigned uid, User &user) { return lookup(nullptr, uid, true, user); }
bool initialize()
{
  if (geteuid() != 0) return false;
  mkdir("/etc"); mkdir("/home"); mkdir("/root");
  if (chown("/etc", 0, 0) < 0 || chmod("/etc", 0755) < 0 || chown("/home", 0, 0) < 0 ||
      chmod("/home", 0755) < 0 || chown("/root", 0, 0) < 0 || chmod("/root", 0700) < 0) return false;
  int fd = open("/etc/passwd", O_RDONLY);
  if (fd < 0)
  {
    fd = open("/etc/passwd", O_CREATE | O_WRONLY);
    const char *data = "root:!:0:0:root:/root:" XV_SHELL_PATH "\nnobody:!:65534:65534:nobody:/tmp:" XV_SHELL_PATH "\n";
    if (fd < 0) return false;
    bool written = write(fd, data, strlen(data)) == static_cast<int>(strlen(data)); close(fd);
    if (!written) return false;
  }
  else close(fd);
  return chown("/etc/passwd", 0, 0) == 0 && chmod("/etc/passwd", 0644) == 0;
}
static void decimal(unsigned value, char *output)
{
  char digits[12]; unsigned count = 0;
  do { digits[count++] = '0' + value % 10; value /= 10; } while (value);
  unsigned used = 0; while (count) output[used++] = digits[--count]; output[used] = 0;
}
int add(const char *name)
{
  if (geteuid() != 0 || !*name || strlen(name) >= 14) return -1;
  for (unsigned i = 0; name[i]; ++i)
    if (!((name[i] >= 'a' && name[i] <= 'z') || (name[i] >= 'A' && name[i] <= 'Z') || name[i] == '_' ||
          (i && ((name[i] >= '0' && name[i] <= '9') || name[i] == '-')))) return -1;
  if (mkdir("/etc/.users.lock") < 0) return -1;
  int result = -1; User existing;
  if (!find(name, existing))
  {
    unsigned uid = 1000; while (uid < 65534 && find(uid, existing)) ++uid;
    char home[64]; strcpy(home, "/home/"); strcpy(home + 6, name);
    if (uid < 65534 && mkdir(home) == 0)
    {
      if (chown(home, uid, uid) == 0 && chmod(home, 0750) == 0)
      {
        int fd = open("/etc/passwd", O_RDWR);
        if (fd >= 0)
        {
          char discard[256]; int count;
          while ((count = read(fd, discard, sizeof(discard))) > 0) {}
          if (!count)
          {
            char line[256], number[12]; decimal(uid, number);
            strcpy(line, name); strcpy(line + strlen(line), ":!:"); strcpy(line + strlen(line), number);
            strcpy(line + strlen(line), ":"); strcpy(line + strlen(line), number); strcpy(line + strlen(line), ":");
            strcpy(line + strlen(line), name); strcpy(line + strlen(line), ":"); strcpy(line + strlen(line), home);
            strcpy(line + strlen(line), ":" XV_SHELL_PATH "\n");
            if (write(fd, line, strlen(line)) == static_cast<int>(strlen(line))) result = uid;
          }
          close(fd);
        }
      }
      if (result < 0) unlink(home);
    }
  }
  unlink("/etc/.users.lock"); return result;
}
}
