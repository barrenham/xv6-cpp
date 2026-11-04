module;
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/io/fs_resources.h"
#include "kernel/process/wait/wait.h"
module xv.userfs.launcher;
import xv.userfs.plugin;

namespace userfs::launcher
{
namespace
{
bool descriptors(int source, bool system, int ready, int activation, int image)
{
  // Keep the five inputs above the final ABI slots before closing anything.
  // NOFILE is 16; filling slots 0..9 leaves exactly 10..14 for these references.
  int input[5] = {source, system ? 4 : -1, ready, activation, image};
  int kept[5];
  int padding;
  do { padding = dup(0); if (padding < 0) return false; } while (padding < 10);
  if (padding != 10) return false;
  close(padding);
  for (unsigned i = 0; i < 5; ++i)
  {
    kept[i] = dup(input[i] >= 0 ? input[i] : 0);
    if (kept[i] != int(10 + i)) return false;
  }
  for (int fd = 3; fd < 10; ++fd) close(fd);
  for (unsigned i = 0; i < 5; ++i) if (dup(kept[i]) != int(3 + i)) return false;
  for (int fd = 10; fd < 16; ++fd) close(fd);
  for (unsigned i = 0; i < 5; ++i) if (input[i] < 0) close(3 + i);
  return true;
}
[[noreturn]] void execute(const catalog::Plugin &plugin, const char *type,
                         const char *options, unsigned mode, bool probing,
                         bool bundled, int source, int ready, int activation,
                         int image, int control)
{
  if (control >= 0) close(control);
  if (!descriptors(source, plugin.resource == catalog::Resource::System || plugin.system,
                   ready, activation, image)) exit(1);
  char permissions[6];
  permissions[5] = 0;
  for (int i = 4; i >= 0; --i) { permissions[i] = '0' + (mode & 7); mode >>= 3; }
  char *arguments[] = {const_cast<char *>(plugin.executable),
    const_cast<char *>(probing ? "--fs-probe" : "--fs-plugin"),
    const_cast<char *>("1"), const_cast<char *>(type),
    const_cast<char *>(probing ? options : permissions),
    probing ? nullptr : const_cast<char *>(options), nullptr};
  if (bundled) fexecve(userfs::plugin::Image, arguments, nullptr);
  else exec(plugin.executable, arguments);
  fprintf(2, "fsplugin: cannot execute %s\n", plugin.executable);
  exit(1);
}
void retire(int pid)
{
  kill(pid);
  waitpid(pid, nullptr, 0);
}
}
int source(const catalog::Plugin &plugin, const char *override_source)
{
  const char *name = override_source && *override_source ? override_source : plugin.source;
  if (plugin.resource == catalog::Resource::Block)
  {
    if (name[1] || (name[0] != '0' && name[0] != '1')) return -1;
    return ioctl(4, xv::fsservice::StorageIoctl, name[0] - '0');
  }
  if (plugin.resource == catalog::Resource::File)
  {
    if ((strlen(name) >= 5 && !memcmp(name, "boot:", 5)))
      return plugin.writable ? -1 : catalog::asset(name + 5);
    return name[0] == '/' ? open(name, plugin.writable ? O_RDWR : O_RDONLY) : -1;
  }
  return -1;
}
int probe(const catalog::Plugin &plugin, int source)
{
  int image = catalog::asset(plugin.image);
  if (image < 0) return -1;
  int pid = fork();
  if (!pid) execute(plugin, plugin.type, plugin.options, plugin.mode, true, true,
                    source, -1, -1, image, -1);
  close(image);
  if (pid < 0) return -1;
  for (unsigned attempt = 0; attempt < 100; ++attempt)
  {
    int status = 0, result = waitpid(pid, &status, WNOHANG);
    if (result > 0) return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : -1;
    if (result < 0) return -1;
    pause(1);
  }
  retire(pid);
  return -1;
}
int start(const catalog::Plugin &plugin, const char *type, const char *path,
          int source, bool bundled, unsigned mode, int control, const char *options,
          bool (*progress)(void *), void *context)
{
  int image = bundled ? catalog::asset(plugin.image) : -1;
  if (bundled && image < 0) return -1;
  int ready[2], activation[2];
  if (pipe(ready) < 0) { if (image >= 0) close(image); return -1; }
  if (pipe(activation) < 0)
  { close(ready[0]); close(ready[1]); if (image >= 0) close(image); return -1; }
  int pid = fork();
  if (!pid)
  {
    close(ready[0]); close(activation[1]);
    execute(plugin, type, options && *options ? options : plugin.options, mode,
            false, bundled, source, ready[1], activation[0], image, control);
  }
  if (image >= 0) close(image);
  close(ready[1]); close(activation[0]);
  if (pid < 0) { close(ready[0]); close(activation[1]); return -1; }
  pollfd watched{ready[0], POLLIN, 0};
  int available = 0;
  for (unsigned long waited = 0; waited < plugin.startup_ms;)
  {
    unsigned slice = plugin.startup_ms - waited < 250 ? plugin.startup_ms - waited : 250;
    available = poll(&watched, 1, slice);
    if (available) break;
    waited += slice;
    if (progress && !progress(context)) { available = -1; break; }
  }
  char marker = 0;
  int received = available > 0 ? read(ready[0], &marker, 1) : -1;
  close(ready[0]);
  if (received != 1 || marker != 'R' || mount(type, path) < 0)
  { close(activation[1]); retire(pid); return -1; }
  int approved = write(activation[1], "G", 1);
  close(activation[1]);
  if (approved != 1) { umount(path); retire(pid); return -1; }
  printf("fsboot: %s mounted at %s, server pid=%d\n", type, path, pid);
  return pid;
}
}
