#include "application.h"
#include "user/services/protocol.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"

namespace
{
void
show(const services::Reply &entry) {
  printf("%-24s %-12s %6d %8d %6d  %s\n", entry.name, services::phase_name(entry.phase),
         entry.pid, entry.restarts, entry.status, entry.external ? "core" : entry.log);
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
    fprintf(
        2,
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
    printf("%-24s %-12s %6s %8s %6s  %s\n", "NAME", "STATE", "PID", "RESTARTS",
           "STATUS", "LOG / OWNER");
  }
  for (;; ++request.index)
  {
    services::Reply response;
    if (!services::exchange(socket, request, response))
    {
      fprintf(2, "servicectl: manager unavailable\n");
      return 1;
    }
    if (response.result < 0)
    {
      if (list)
      {
        return 0;
      }
      fprintf(2, "servicectl: %s\n",
              response.detail[0] ? response.detail
                                 : "service or operation unavailable");
      return 1;
    }
    if (logs)
    {
      return log(response.log);
    }
    if (request.command == services::Command::Load)
    {
      printf("configuration loaded\n");
      return 0;
    }
    show(response);
    if (!list)
    {
      if (response.detail[0])
      {
        printf("%s\n", response.detail);
      }
      return 0;
    }
  }
}
