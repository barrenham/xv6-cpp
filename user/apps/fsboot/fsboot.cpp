#include "kernel/support/types.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/image_paths.h"
#include "user/runtime/user.h"
#include "user/runtime/userfs/block_io.h"
#include "user/runtime/userfs/ext3.h"
#include "user/runtime/userfs/devices.h"

namespace
{
enum class Service { Root, Devices };

[[noreturn]] void fatal(const char *message)
{
  fprintf(2, "fsboot: %s\n", message);
  if (getpid() != 1) exit(1);
  for (;;) pause(1);
}

[[noreturn]] void serve(int ready, const char *type, userfs::Backend &backend)
{
  int endpoint = userfs::register_backend(type, backend);
  if (endpoint < 0) fatal("backend registration failed");
  if (write(ready, "R", 1) != 1) fatal("supervisor notification failed");
  close(ready);
  int result = userfs::serve(endpoint, backend);
  close(endpoint);
  exit(result < 0);
}

[[noreturn]] void worker(Service kind, int ready, const char *type)
{
  if (kind == Service::Root)
  {
    close(4);
    userfs::BlockIo io(3);
    auto *storage = malloc(sizeof(userfs::Ext3Backend));
    if (!storage) fatal("volume allocation failed");
    auto *backend = new (storage) userfs::Ext3Backend(io);
    if (!backend->initialize()) fatal("ext3 mount or recovery failed");
    serve(ready, type, *backend);
  }
  userfs::DeviceBackend backend(4);
  serve(ready, type, backend);
}

int start(Service kind, const char *type, const char *path)
{
  int channel[2];
  if (pipe(channel) < 0) fatal("cannot create service channel");
  int pid = fork();
  if (pid < 0) fatal("cannot fork service");
  if (!pid)
  {
    close(channel[0]);
    worker(kind, channel[1], type);
  }
  close(channel[1]);
  char ready;
  int received = read(channel[0], &ready, 1);
  close(channel[0]);
  if (received != 1 || ready != 'R' || mount(type, path) < 0)
    fatal("cannot mount registered service");
  printf("fsboot: %s mounted at %s, server pid=%d\n", type, path, pid);
  return pid;
}

int start_init()
{
  int pid = fork();
  if (pid < 0) fatal("cannot fork init");
  if (!pid)
  {
    // init deliberately opens its own console and owns descriptors 0..2.
    for (int descriptor = 0; descriptor < 3; ++descriptor) close(descriptor);
    char *arguments[] = {const_cast<char *>(XV_INIT_PATH), nullptr};
    exec(arguments[0], arguments);
    int console = open("/dev/console", O_RDWR);
    if (console == 0) { dup(0); dup(0); }
    fatal("cannot execute init");
  }
  return pid;
}
} // namespace

extern "C" int main(int, char **)
{
  if (getpid() != 1) fatal("must run as the bootstrap process");
  int root = start(Service::Root, "ext3", "/");
  close(3);
  if (chdir("/") < 0) fatal("cannot select root cwd");
  int devices = start(Service::Devices, "devfs", "/dev");
  close(4);
  // procfs, ramfs and the alternate disk format are added in the next migration
  // steps; this executable is not the default QEMU boot path until all are ready.
  int init = start_init();
  for (;;)
  {
    int status;
    int pid = wait(&status);
    if (pid == init)
    {
      fprintf(2, "fsboot: init exited status=%d; restarting\n", status);
      init = start_init();
    }
    else if (pid == root || pid == devices)
    {
      fprintf(2, "fsboot: %s service pid=%d exited status=%d; mount unavailable\n",
              pid == root ? "root" : "device", pid, status);
    }
    else if (pid < 0) pause(1);
  }
}
