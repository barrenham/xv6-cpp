#include "kernel/support/types.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/image_paths.h"
#include "user/runtime/user.h"
#include "user/runtime/userfs/block_io.h"
#include "user/runtime/userfs/ext3.h"
#include "user/runtime/userfs/xv6.h"
#include "user/runtime/userfs/devices.h"
#include "user/runtime/userfs/memory.h"
#include "user/runtime/userfs/processes.h"

namespace
{
enum class Service { Root, Devices, Memory, Processes };

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
    if (!strcmp(type, "xv6"))
    {
      auto *storage = malloc(sizeof(userfs::Xv6Backend));
      if (!storage) fatal("volume allocation failed");
      auto *backend = new (storage) userfs::Xv6Backend(io);
      if (!backend->initialize()) fatal("xv6 mount or recovery failed");
      serve(ready, type, *backend);
    }
    auto *storage = malloc(sizeof(userfs::Ext3Backend));
    if (!storage) fatal("volume allocation failed");
    auto *backend = new (storage) userfs::Ext3Backend(io);
    if (!backend->initialize()) fatal("ext3 mount or recovery failed");
    serve(ready, type, *backend);
  }
  if (kind == Service::Memory)
  {
    close(4);
    userfs::MemoryBackend backend(userfs::heap_storage(), 01777);
    if (!backend.ready()) fatal("ramfs allocation failed");
    serve(ready, type, backend);
  }
  if (kind == Service::Processes)
  {
    userfs::ProcessBackend backend(4);
    close(4);
    if (!backend.ready()) fatal("process resource allocation failed");
    serve(ready, type, backend);
  }
  userfs::DeviceBackend backend(4);
  close(4);
  if (!backend.ready()) fatal("device resource allocation failed");
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
  userfs::BlockIo disk(3);
  unsigned char signature[64];
  if (!disk.read(1024, signature, sizeof(signature))) fatal("cannot read root signature");
  const char *root_type = xv::ext3::le32(signature) == FSMAGIC ? "xv6" :
                          xv::ext3::le16(signature + 56) == xv::ext3::Magic ? "ext3" : nullptr;
  if (!root_type) fatal("unsupported root filesystem format");
  int root = start(Service::Root, root_type, "/");
  close(3);
  if (chdir("/") < 0) fatal("cannot select root cwd");
  int devices = start(Service::Devices, "devfs", "/dev");
  int processes = start(Service::Processes, "proc", "/proc");
  int memory = start(Service::Memory, "ramfs", "/tmp");
  close(4);
  // Full mount management is added in the next migration step; this executable is not the default QEMU boot path until all are ready.
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
    else if (pid == root || pid == devices || pid == memory || pid == processes)
    {
      fprintf(2, "fsboot: %s service pid=%d exited status=%d; mount unavailable\n",
              pid == root ? "root" : pid == devices ? "device" : pid == processes ? "process" : "memory", pid, status);
    }
    else if (pid < 0) pause(1);
  }
}
