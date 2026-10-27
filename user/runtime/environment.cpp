#include "kernel/fs/image_paths.h"
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"
namespace
{
char *empty[1]{};
void *owned = nullptr;
bool name_valid(const char *name)
{ if (!name || !*name) return false; for (; *name; ++name) if (*name == '=') return false; return true; }
bool matches(const char *entry, const char *name)
{ while (*name && *entry == *name) { ++entry; ++name; } return !*name && *entry == '='; }
int change(const char *name, const char *value)
{
  unsigned count = 0, bytes = 0;
  if (environ) for (unsigned i = 0; environ[i]; ++i)
  {
    if (i >= MAXENV - 1) return -1;
    if (matches(environ[i], name)) continue;
    unsigned length = strlen(environ[i]) + 1;
    if (length > 65536 - bytes) return -1;
    bytes += length; ++count;
  }
  if (value)
  {
    unsigned length = strlen(name) + strlen(value) + 2;
    if (count >= MAXENV - 1 || length > 65536 - bytes) return -1;
    bytes += length; ++count;
  }
  auto *block = static_cast<char *>(malloc((count + 1) * sizeof(char *) + bytes));
  if (!block) return -1;
  auto **vector = reinterpret_cast<char **>(block);
  char *destination = block + (count + 1) * sizeof(char *);
  unsigned index = 0;
  if (environ) for (unsigned i = 0; environ[i]; ++i)
    if (!matches(environ[i], name))
    { vector[index++] = destination; strcpy(destination, environ[i]); destination += strlen(destination) + 1; }
  if (value)
  {
    vector[index++] = destination; strcpy(destination, name); destination += strlen(name);
    *destination++ = '='; strcpy(destination, value);
  }
  vector[index] = nullptr;
  void *old = owned; owned = block; environ = vector; if (old) free(old);
  return 0;
}
}
extern "C"
{
char **environ = empty;
void __environment_init(char **environment) { environ = environment ? environment : empty; owned = nullptr; }
char *getenv(const char *name)
{
  if (!name_valid(name) || !environ) return nullptr;
  for (unsigned i = 0; environ[i]; ++i) if (matches(environ[i], name)) return environ[i] + strlen(name) + 1;
  return nullptr;
}
int setenv(const char *name, const char *value, int overwrite)
{
  if (!name_valid(name) || !value) return -1;
  if (!overwrite && getenv(name)) return 0;
  return change(name, value);
}
int unsetenv(const char *name) { return name_valid(name) ? change(name, nullptr) : -1; }
int clearenv()
{ if (owned) free(owned); owned = nullptr; environ = empty; return 0; }
int execv(const char *path, char *const *arguments) { return execve(path, arguments, environ); }
int exec(const char *path, char **arguments) { return execv(path, arguments); }
int execvp(const char *file, char *const *arguments)
{
  if (!file || !*file) return -1;
  if (strchr(file, '/')) return execv(file, arguments);
  const char *search = getenv("PATH"); if (!search) search = XV_DEFAULT_PATH;
  for (;;)
  {
    const char *end = search; while (*end && *end != ':') ++end;
    unsigned prefix = end - search, name = strlen(file);
    if (prefix + name + 2 <= MAXPATH)
    {
      char path[MAXPATH]; unsigned used = 0;
      for (; used < prefix; ++used) path[used] = search[used];
      if (used) path[used++] = '/';
      strcpy(path + used, file); execv(path, arguments);
    }
    if (!*end) return -1;
    search = end + 1;
  }
}
}
