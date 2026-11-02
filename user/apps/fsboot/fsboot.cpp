#include "kernel/support/types.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/image_paths.h"
#include "kernel/fs/poll.h"
#include "kernel/io/fs_mount.h"
#include "kernel/io/fs_resources.h"
#include "kernel/process/wait/wait.h"
#include "user/runtime/user.h"
#include "user/runtime/userfs/block_io.h"
#include "user/runtime/userfs/ext3.h"
#include "user/runtime/userfs/xv6.h"
#include "user/runtime/userfs/devices.h"
#include "user/runtime/userfs/memory.h"
#include "user/runtime/userfs/processes.h"

namespace
{
enum class Service { Disk, Devices, Memory, Processes };

[[noreturn]] void fatal(const char *message)
{
  fprintf(2, "fsboot: %s\n", message);
  if (getpid() != 1) exit(1);
  for (;;) pause(1);
}

[[noreturn]] void serve(int ready, int activation, const char *type, userfs::Backend &backend)
{
  int endpoint = userfs::register_backend(type, backend);
  if (endpoint < 0) fatal("backend registration failed");
  if (write(ready, "R", 1) != 1) fatal("supervisor notification failed");
  close(ready);
  char approved;
  // Until publication, the parent owns the activation pipe. If it dies during
  // registration or mounting, EOF retires this unpublished service and its
  // storage lease instead of leaving an orphan waiting forever for requests.
  int received = read(activation, &approved, 1);
  close(activation);
  if (received != 1 || approved != 'G') fatal("service publication canceled");
  int result = userfs::serve(endpoint, backend);
  close(endpoint);
  exit(result < 0);
}

[[noreturn]] void worker(Service kind, int ready, int activation, const char *type, int disk, unsigned mode)
{
  if (kind == Service::Disk)
  {
    close(4);
    userfs::BlockIo io(disk);
    if (!strcmp(type, "xv6"))
    {
      auto *storage = malloc(sizeof(userfs::Xv6Backend));
      if (!storage) fatal("volume allocation failed");
      auto *backend = new (storage) userfs::Xv6Backend(io);
      if (!backend->initialize()) fatal("xv6 mount or recovery failed");
      serve(ready, activation, type, *backend);
    }
    auto *storage = malloc(sizeof(userfs::Ext3Backend));
    if (!storage) fatal("volume allocation failed");
    auto *backend = new (storage) userfs::Ext3Backend(io);
    if (!backend->initialize()) fatal("ext3 mount or recovery failed");
    serve(ready, activation, type, *backend);
  }
  if (kind == Service::Memory)
  {
    close(4);
    userfs::MemoryBackend backend(userfs::heap_storage(), mode);
    if (!backend.ready()) fatal("ramfs allocation failed");
    serve(ready, activation, type, backend);
  }
  if (kind == Service::Processes)
  {
    userfs::ProcessBackend backend(4);
    close(4);
    if (!backend.ready()) fatal("process resource allocation failed");
    serve(ready, activation, type, backend);
  }
  userfs::DeviceBackend backend(4);
  close(4);
  if (!backend.ready()) fatal("device resource allocation failed");
  serve(ready, activation, type, backend);
}

int launch(Service kind, const char *type, const char *path, int disk = -1, int control = -1)
{
  int channel[2], activation[2];
  if (pipe(channel) < 0) return -1;
  if (pipe(activation) < 0) { close(channel[0]); close(channel[1]); return -1; }
  int pid = fork();
  if (pid < 0)
  {
    close(channel[0]); close(channel[1]); close(activation[0]); close(activation[1]);
    return -1;
  }
  if (!pid)
  {
    close(channel[0]);
    close(activation[1]);
    if (control >= 0) close(control);
    worker(kind, channel[1], activation[0], type, disk, !strcmp(path, "/tmp") ? 01777 : 0755);
  }
  close(channel[1]);
  close(activation[0]);
  char ready;
  int received = read(channel[0], &ready, 1);
  close(channel[0]);
  if (received != 1 || ready != 'R' || mount(type, path) < 0)
  {
    close(activation[1]);
    kill(pid);
    waitpid(pid, nullptr, 0);
    return -1;
  }
  int published = write(activation[1], "G", 1);
  close(activation[1]);
  if (published != 1)
  {
    umount(path);
    kill(pid);
    waitpid(pid, nullptr, 0);
    return -1;
  }
  printf("fsboot: %s mounted at %s, server pid=%d\n", type, path, pid);
  return pid;
}

int start(Service kind, const char *type, const char *path, int disk = -1)
{
  int pid = launch(kind, type, path, disk);
  if (pid < 0) fatal("cannot mount registered service");
  return pid;
}

const char *disk_format(int descriptor)
{
  userfs::BlockIo disk(descriptor);
  unsigned char signature[64];
  if (!disk.read(1024, signature, sizeof(signature))) return nullptr;
  if (xv::ext3::le32(signature) == FSMAGIC) return "xv6";
  return xv::ext3::le16(signature + 56) == xv::ext3::Magic ? "ext3" : nullptr;
}

int managed_mount(const xv::fsmount::Request &request, int control)
{
  if (!request.type[0] || request.path[0] != '/' || !request.path[1]) return -1;
  // A separately installed plugin may already have registered an instance.
  // The manager's own mount call goes straight to the generic registry.
  if (mount(request.type, request.path) == 0) return 0;
  Service kind;
  int disk = -1;
  if (!strcmp(request.type, "ramfs")) kind = Service::Memory;
  else if (!strcmp(request.type, "proc") || !strcmp(request.type, "procfs")) kind = Service::Processes;
  else if (!strcmp(request.type, "devfs")) kind = Service::Devices;
  else if (!strcmp(request.type, "ext3") || !strcmp(request.type, "xv6"))
  {
    kind = Service::Disk;
    disk = ioctl(4, xv::fsservice::StorageIoctl, 1);
    const char *format = disk >= 0 ? disk_format(disk) : nullptr;
    if (!format || strcmp(request.type, format))
    {
      if (disk >= 0) close(disk);
      return -1;
    }
  }
  else return -1;
  int pid = launch(kind, request.type, request.path, disk, control);
  if (disk >= 0) close(disk);
  return pid < 0 ? -1 : 0;
}

[[noreturn]] void manage(int ready)
{
  int control = ioctl(4, xv::fsservice::ManagerIoctl, 0);
  if (control < 0 || write(ready, "R", 1) != 1) fatal("mount manager registration failed");
  close(ready);
  for (;;)
  {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {}
    pollfd watched{control, POLLIN, 0};
    int events = poll(&watched, 1, 250);
    if (events < 0) continue;
    if (watched.revents & (POLLHUP | POLLERR | POLLNVAL)) break;
    if (!(watched.revents & POLLIN)) continue;
    xv::fsmount::Request request;
    if (read(control, &request, sizeof(request)) != sizeof(request)) break;
    request.result = managed_mount(request, control);
    if (write(control, &request, sizeof(request)) != sizeof(request)) break;
  }
  close(control);
  close(4);
  exit(1);
}

int start_manager()
{
  int channel[2];
  if (pipe(channel) < 0) fatal("cannot create manager channel");
  int pid = fork();
  if (pid < 0) fatal("cannot fork mount manager");
  if (!pid) { close(channel[0]); manage(channel[1]); }
  close(channel[1]);
  char ready;
  int received = read(channel[0], &ready, 1);
  close(channel[0]);
  if (received != 1 || ready != 'R') fatal("cannot start mount manager");
  printf("fsboot: mount manager ready, pid=%d\n", pid);
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
  const char *root_type = disk_format(3);
  if (!root_type) fatal("unsupported root filesystem format");
  int root = start(Service::Disk, root_type, "/", 3);
  close(3);
  if (chdir("/") < 0) fatal("cannot select root cwd");
  int devices = start(Service::Devices, "devfs", "/dev");
  int processes = start(Service::Processes, "proc", "/proc");
  int memory = start(Service::Memory, "ramfs", "/tmp");
  int manager = start_manager();
  close(4);
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
    else if (pid == root || pid == devices || pid == memory || pid == processes || pid == manager)
    {
      fprintf(2, "fsboot: %s service pid=%d exited status=%d; %s\n",
              pid == root ? "root" : pid == devices ? "device" : pid == processes ? "process" :
              pid == manager ? "mount manager" : "memory", pid, status,
              pid == manager ? "automatic mounts unavailable" : "mount unavailable");
    }
    else if (pid < 0) pause(1);
  }
}
