module;
#include "drivers/sdk/host.h"
#include "kernel/support/placement_new.hpp"
module xv.drivers;
import :gpu;

#include "drivers/sdk/adapters.inc"

namespace
{
alignas(xv::drivers::VirtioGpu) unsigned char storage[sizeof(xv::drivers::VirtioGpu)]{};
bool attached = false;

void
interrupt(void *context) {
  static_cast<xv::drivers::VirtioGpu *>(context)->handle_interrupt();
}

int
attach(const xv_kmod_host_v2 *host, const xv_kmod_device_v2 *device) {
  if (attached)
  {
    return -1;
  }
  xv_driver_set_host(host);
  auto *state =
      new (storage) xv::drivers::VirtioGpu(device->base.mmio, device->base.irq);
  if (host->interrupt(device->binding, state, interrupt) < 0)
  {
    return -1;
  }
  state->initialize();
  if (!state->open(false))
  {
    host->interrupt(device->binding, nullptr, nullptr);
    return -1;
  }
  auto operations = xv::drivers::plugin::character(*state);
  if (host->character(device->binding, "fb0", 29, 0, 0600, &operations) < 0)
  {
    return -1;
  }
  attached = true;
  return 0;
}

const xv_kmod_driver_v2 driver{{sizeof(driver), XV_KMOD_ABI_V2, XV_KMOD_BUS_VIRTIO_MMIO,
                                16, "virtio_gpu", nullptr},
                               attach};
} // namespace

extern "C"
{
  [[gnu::visibility("default")]]
  const xv_kmod_driver *
  xv_driver_entry(unsigned abi) {
    return abi == XV_KMOD_ABI_V2 ? &driver.base : nullptr;
  }
}
