module;
#include "user/runtime/ui.h"
#include "user/runtime/user.h"
#include "user/fs/supervisor/workspace.h"
#include "kernel/io/fs_mount.h"
#include "kernel/io/fs_resources.h"
#include "kernel/fs/poll.h"
#include "kernel/process/wait/wait.h"
module xv.userfs.manager;
import xv.userfs.catalog;
import xv.userfs.launcher;

namespace userfs::manager
{
namespace
{
struct Progress
{
  int control;
  const xv::fsmount::Request &request;
};

bool
notify(void *context) {
  const auto &state = *static_cast<Progress *>(context);
  auto message = state.request;
  message.result = xv::fsmount::Progress;
  return write(state.control, &message, sizeof(message)) == sizeof(message);
}

int
mount_request(const xv::fsmount::Request &request, int control,
              catalog::Plugin &plugin) {
  if (!request.type[0] || request.type[31] || request.path[0] != '/' ||
      !request.path[1] || request.path[127] || request.source[127] ||
      request.options[255])
  {
    return -1;
  }
  // Pre-registered instances remain available independently of the catalog.
  if (!request.source[0] && !request.options[0] &&
      mount(request.type, request.path) == 0)
  {
    return 0;
  }
  if (!catalog::installed(request.type, plugin))
  {
    return -1;
  }
  int source = launcher::source(plugin, request.source);
  if ((plugin.resource == catalog::Resource::Block ||
       plugin.resource == catalog::Resource::File) &&
      source < 0)
  {
    return -1;
  }
  Progress progress{control, request};
  int pid = launcher::start(plugin, request.type, request.path, source, false,
                            plugin.mode, control, request.options, notify, &progress);
  if (source >= 0)
  {
    close(source);
  }
  return pid < 0 ? -1 : 0;
}

[[noreturn]]
void
serve(int ready) {
  struct State
  {
    xv::fsmount::Request request;
    catalog::Plugin plugin;
  };

  detail::Workspace<State> storage;
  if (!storage)
  {
    exit(1);
  }
  int control = ioctl(4, xv::fsservice::ManagerIoctl, 0);
  if (control < 0 || write(ready, "R", 1) != 1)
  {
    exit(1);
  }
  close(ready);
  for (;;)
  {
    while (waitpid(-1, nullptr, WNOHANG) > 0)
    {
    }
    pollfd watched{control, POLLIN, 0};
    if (poll(&watched, 1, 250) < 0)
    {
      continue;
    }
    if (watched.revents & (POLLHUP | POLLERR | POLLNVAL))
    {
      break;
    }
    if (!(watched.revents & POLLIN))
    {
      continue;
    }
    auto &request = storage->request;
    if (read(control, &request, sizeof(request)) != sizeof(request))
    {
      break;
    }
    request.result = mount_request(request, control, storage->plugin);
    if (write(control, &request, sizeof(request)) != sizeof(request))
    {
      break;
    }
  }
  close(control);
  close(4);
  exit(1);
}
} // namespace

int
start() {
  int channel[2];
  if (pipe(channel) < 0)
  {
    return -1;
  }
  int pid = fork();
  if (!pid)
  {
    close(channel[0]);
    serve(channel[1]);
  }
  close(channel[1]);
  if (pid < 0)
  {
    close(channel[0]);
    return -1;
  }
  pollfd watched{channel[0], POLLIN, 0};
  char ready = 0;
  int received = poll(&watched, 1, 5000) > 0 ? read(channel[0], &ready, 1) : -1;
  close(channel[0]);
  if (received != 1 || ready != 'R')
  {
    kill(pid);
    waitpid(pid, nullptr, 0);
    return -1;
  }
  ui::info("fsboot: mount manager ready, pid=%d\n", pid);
  return pid;
}
} // namespace userfs::manager
