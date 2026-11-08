#include "user/runtime/ui.h"
#include "application.h"
#include "user/services/supervisor.h"
#include "user/runtime/user.h"

extern "C" int
app_main(int argc, char **argv) {
  const char *config = "/etc/services.conf", *socket = services::DefaultSocket;
  int ready = -1;
  for (int i = 1; i < argc; ++i)
  {
    if (!strcmp(argv[i], "--config") && i + 1 < argc)
    {
      config = argv[++i];
    }
    else if (!strcmp(argv[i], "--socket") && i + 1 < argc)
    {
      socket = argv[++i];
    }
    else if (!strcmp(argv[i], "--ready-fd") && i + 1 < argc)
    {
      ready = atoi(argv[++i]);
    }
    else
    {
      ui::usage("usage: serviced [--config FILE] [--socket PATH]\n");
      return 1;
    }
  }
  return services::run(config, socket, ready);
}
