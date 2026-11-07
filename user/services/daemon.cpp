#include "supervisor.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/process/wait/wait.h"
#include "kernel/support/placement_new.hpp"

namespace services
{
namespace
{
volatile sig_atomic_t stopping = 0;

void
stop_signal(int) {
  stopping = 1;
}
} // namespace

int
run(const char *config, const char *socket, int ready_fd) {
  if (geteuid() != 0)
  {
    fprintf(2, "serviced: root required\n");
    return 1;
  }
  umask(0077);
  auto *memory = malloc(sizeof(Supervisor));
  if (!memory)
  {
    return 1;
  }
  auto *supervisor = new (memory) Supervisor{};

  struct Cleanup
  {
    Supervisor *owner;

    ~Cleanup() {
      owner->shutdown();
      owner->~Supervisor();
      free(owner);
    }
  } cleanup{supervisor};

  char error[96];
  if (!supervisor->load(config, error))
  {
    fprintf(2, "serviced: %s\n", error);
    return 1;
  }
  int listener = services::listen(socket);
  if (listener < 0)
  {
    fprintf(2, "serviced: cannot bind control socket\n");
    return 1;
  }
  struct sigaction action{stop_signal, 0, 0};
  if (sigaction(SIGTERM, &action, nullptr) < 0 ||
      sigaction(SIGINT, &action, nullptr) < 0)
  {
    close(listener);
    unlink(socket);
    return 1;
  }
  if (ready_fd >= 0)
  {
    bool sent = write(ready_fd, "R", 1) == 1;
    close(ready_fd);
    if (!sent)
    {
      close(listener);
      unlink(socket);
      return 1;
    }
  }
  printf("serviced: ready pid=%d config=%s socket=%s\n", getpid(), config, socket);
  while (!stopping)
  {
    supervisor->tick();
    pollfd pending{listener, POLLIN, 0};
    if (poll(&pending, 1, 50) < 0 && errno != EINTR)
    {
      break;
    }
    if (!(pending.revents & POLLIN))
    {
      continue;
    }
    for (unsigned budget = 0; budget < 8; ++budget)
    {
      Request request;
      int client = accept_request(listener, request);
      if (client < 0)
      {
        break;
      }
      Reply response;
      supervisor->request(request, response);
      services::reply(client, response);
    }
  }
  close(listener);
  unlink(socket);
  return 0;
}

} // namespace services
