#include "user/runtime/ui.h"
#include "application.h"
#include "user/services/protocol.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"

namespace
{
void
show(const services::Reply &entry) {
  auto tone = entry.phase == services::Phase::Running ||
                      entry.phase == services::Phase::Complete
                  ? ui::Tone::Success
              : entry.phase == services::Phase::Failed ||
                      entry.phase == services::Phase::Unavailable
                  ? ui::Tone::Error
              : entry.phase == services::Phase::Stopped ? ui::Tone::Muted
                                                        : ui::Tone::Warning;
  ui::print(1, tone, "%-24s %-12s %6d %8d %6d  %s\n", entry.name,
            services::phase_name(entry.phase), entry.pid, entry.restarts, entry.status,
            entry.external ? "core" : entry.log);
}

int
log(const char *path) {
  int fd = open(path, O_RDONLY);
  if (fd < 0)
  {
    return 1;
  }
  char buffer[512];
  int count;
  while ((count = read(fd, buffer, sizeof(buffer))) > 0)
  {
    if (write(1, buffer, count) != count)
    {
      close(fd);
      return 1;
    }
  }
  close(fd);
  return count < 0;
}
} // namespace

extern "C" int
app_main(int argc, char **argv) {
  const char *socket = services::DefaultSocket;
  int first = 1;
  if (argc > 3 && !strcmp(argv[1], "--socket"))
  {
    socket = argv[2];
    first = 3;
  }
  if (first >= argc)
  {
    ui::usage(
        "usage: servicectl [--socket PATH] list | status/start/stop/restart/log NAME | load FILE\n");
    return 1;
  }
  const char *command = argv[first];
  services::Request request;
  bool list = !strcmp(command, "list"), logs = !strcmp(command, "log");
  if (list)
  {
    request.command = services::Command::List;
  }
  else
  {
    if (first + 2 != argc || strlen(argv[first + 1]) >= sizeof(request.path))
    {
      return 1;
    }
    if (!strcmp(command, "load"))
    {
      request.command = services::Command::Load;
      strcpy(request.path, argv[first + 1]);
    }
    else
    {
      if (strlen(argv[first + 1]) >= sizeof(request.name))
      {
        return 1;
      }
      strcpy(request.name, argv[first + 1]);
      if (!strcmp(command, "status") || logs)
      {
        request.command = services::Command::Status;
      }
      else if (!strcmp(command, "start"))
      {
        request.command = services::Command::Start;
      }
      else if (!strcmp(command, "stop"))
      {
        request.command = services::Command::Stop;
      }
      else if (!strcmp(command, "restart"))
      {
        request.command = services::Command::Restart;
      }
      else
      {
        return 1;
      }
    }
  }
  if (!logs)
  {
    ui::heading("%-24s %-12s %6s %8s %6s  %s\n", "NAME", "STATE", "PID", "RESTARTS",
                "STATUS", "LOG / OWNER");
  }
  for (;; ++request.index)
  {
    services::Reply response;
    if (!services::exchange(socket, request, response))
    {
      ui::error("servicectl: manager unavailable\n");
      return 1;
    }
    if (response.result < 0)
    {
      if (list)
      {
        return 0;
      }
      ui::error("servicectl: %s\n", response.detail[0]
                                        ? response.detail
                                        : "service or operation unavailable");
      return 1;
    }
    if (logs)
    {
      return log(response.log);
    }
    if (request.command == services::Command::Load)
    {
      ui::success("configuration loaded\n");
      return 0;
    }
    show(response);
    if (!list)
    {
      if (!response.external && request.command == services::Command::Status)
      {
        ui::info("Guardian PID: %d  Captured: %lu bytes  Rotations: %lu\n",
                 response.guardian, response.log_captured, response.log_rotations);
      }
      if (response.detail[0])
      {
        ui::info("%s\n", response.detail);
      }
      return 0;
    }
  }
}
