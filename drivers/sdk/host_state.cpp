#include "drivers/sdk/host.h"

namespace
{
const xv_kmod_host_v2 *services;
}

extern "C"
{
  const xv_kmod_host_v2 *
  xv_driver_host() {
    return services;
  }

  void
  xv_driver_set_host(const xv_kmod_host_v2 *host) {
    if (!services)
    {
      services = host;
    }
    else if (services != host)
    {
      host->panic("driver: changed resident host table");
    }
  }
}
