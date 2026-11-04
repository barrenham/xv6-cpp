module;
#include "user/runtime/user.h"
#include "user/fs/supervisor/workspace.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/image_paths.h"
module xv.userfs.bootstrap;
import xv.userfs.catalog;
import xv.userfs.launcher;
import xv.userfs.manager;
namespace userfs::bootstrap
{
namespace
{
struct Mount { char type[32]{}, path[128]{}; unsigned mode = 0755; };
struct Configuration
{
  char root[32] = "auto", init[128] = XV_INIT_PATH;
  Mount mounts[16]{};
  unsigned count = 0;
};
[[noreturn]] void fatal(const char *message)
{
  fprintf(2, "fsboot: %s\n", message);
  if (getpid() != 1) exit(1);
  for (;;) pause(1);
}
bool configuration(Configuration &out)
{
  int fd = catalog::asset("boot.cfg");
  if (fd < 0) return false;
  struct Text { char bytes[4097]; };
  detail::Workspace<Text> storage;
  if (!storage) { close(fd); return false; }
  auto &bytes = storage->bytes;
  int used = 0, n = 0;
  while (used < 4096 && (n = read(fd, bytes + used, 4096 - used)) > 0) used += n;
  close(fd);
  if (n < 0 || used == 4096) return false;
  bytes[used] = 0;
  for (char *line = bytes; *line;)
  {
    char *next = line;
    while (*next && *next != '\n') ++next;
    if (*next) *next++ = 0;
    if (!*line || *line == '#') { line = next; continue; }
    char *value = line;
    while (*value && *value != '=') ++value;
    if (!*value) return false;
    *value++ = 0;
    if (!strcmp(line, "root"))
    { if (!*value || strlen(value) >= sizeof(out.root)) return false; strcpy(out.root, value); }
    else if (!strcmp(line, "init"))
    { if (*value != '/' || strlen(value) >= sizeof(out.init)) return false; strcpy(out.init, value); }
    else if (!strcmp(line, "mount"))
    {
      if (out.count == 16) return false;
      auto &mount = out.mounts[out.count++];
      char *path = value;
      while (*path && *path != ':') ++path;
      if (!*path) return false;
      *path++ = 0;
      char *mode = path;
      while (*mode && *mode != ':') ++mode;
      if (!*mode) return false;
      *mode++ = 0;
      if (!*value || strlen(value) >= sizeof(mount.type) || *path != '/' ||
          strlen(path) >= sizeof(mount.path) || !catalog::mode(mode, mount.mode)) return false;
      strcpy(mount.type, value); strcpy(mount.path, path);
    }
    else return false;
    line = next;
  }
  return true;
}
bool bundled(const char *type, catalog::Plugin &out)
{
  for (unsigned i = 0; catalog::bundled(i, out); ++i)
    if (!strcmp(type, out.type) || !strcmp(type, out.alias)) return true;
  return false;
}
int initialize(const char *path)
{
  int pid = fork();
  if (pid < 0) fatal("cannot fork init");
  if (!pid)
  {
    for (int fd = 0; fd < 16; ++fd) close(fd);
    char *arguments[] = {const_cast<char *>(path), nullptr};
    exec(path, arguments);
    int console = open("/dev/console", O_RDWR);
    if (console == 0) { dup(0); dup(0); }
    fatal("cannot execute init");
  }
  return pid;
}
}
[[noreturn]] void run()
{
  if (getpid() != 1) fatal("must run as the bootstrap process");
  struct State { Configuration config; catalog::Plugin plugin; int services[16]{}; };
  detail::Workspace<State> storage;
  if (!storage) fatal("configuration allocation failed");
  auto *config = &storage->config;
  if (!configuration(*config)) fatal("invalid boot plugin configuration");
  auto &root = storage->plugin;
  bool selected = false;
  if (strcmp(config->root, "auto")) selected = bundled(config->root, root);
  else
    for (unsigned i = 0; catalog::bundled(i, root); ++i)
      if (root.resource == catalog::Resource::Block && launcher::probe(root, 3) == 0)
      { selected = true; break; }
  if (!selected) fatal("no root filesystem plugin recognized the source");
  int root_source = root.resource == catalog::Resource::Block ? 3 : launcher::source(root);
  if (root.resource == catalog::Resource::File && root_source < 0) fatal("root source unavailable");
  int root_pid = launcher::start(root, root.type, "/", root_source, true, root.mode);
  if (root_source >= 0 && root_source != 3) close(root_source);
  if (root_pid < 0) fatal("cannot mount root plugin");
  close(3);
  if (chdir("/") < 0) fatal("cannot select root cwd");
  auto &services = storage->services;
  for (unsigned i = 0; i < config->count; ++i)
  {
    auto &plugin = storage->plugin;
    const auto &entry = config->mounts[i];
    if (!bundled(entry.type, plugin)) fatal("boot mount plugin missing");
    int source = launcher::source(plugin);
    if ((plugin.resource == catalog::Resource::Block || plugin.resource == catalog::Resource::File) && source < 0)
      fatal("boot mount source unavailable");
    services[i] = launcher::start(plugin, entry.type, entry.path, source, true, entry.mode);
    if (source >= 0) close(source);
    if (services[i] < 0) fatal("cannot mount boot plugin");
  }
  int manager_pid = manager::start();
  if (manager_pid < 0) fatal("cannot start mount manager");
  close(4);
  int init = initialize(config->init);
  for (;;)
  {
    int status = 0;
    int pid = wait(&status);
    if (pid == init)
    {
      fprintf(2, "fsboot: init exited status=%d; restarting\n", status);
      init = initialize(config->init);
    }
    else
    {
      bool service = pid == root_pid || pid == manager_pid;
      for (unsigned i = 0; i < config->count; ++i) service |= pid == services[i];
      if (service)
        fprintf(2, "fsboot: %s service pid=%d exited status=%d; unavailable\n",
                pid == root_pid ? "root" : pid == manager_pid ? "mount manager" : "filesystem", pid, status);
      else if (pid < 0) pause(1);
    }
  }
}
}
