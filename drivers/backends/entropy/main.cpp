module;
#include "drivers/sdk/host.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/poll.h"
module xv.drivers;
import :entropy;

#include "drivers/sdk/adapters.inc"

namespace
{
void
interrupt(void *context) {
  static_cast<xv::drivers::VirtioEntropy *>(context)->handle_interrupt();
}

int
attach(const xv_kmod_host_v2 *host, const xv_kmod_device_v2 *device) {
  xv_driver_set_host(host);
  auto *page = host->page(host->owner);
  if (!page)
  {
    return -1;
  }
  static_assert(sizeof(xv::drivers::VirtioEntropy) <= 4096);
  auto *state =
      new (page) xv::drivers::VirtioEntropy(device->base.mmio, device->base.irq);
  if (host->interrupt(device->binding, state, interrupt) < 0)
  {
    host->free_page(host->owner, page, false);
    return -1;
  }
  state->initialize();
  if (state->poll(nullptr) & POLLERR)
  {
    host->interrupt(device->binding, nullptr, nullptr);
    host->free_page(host->owner, page, false);
    return -1;
  }
  auto operations = xv::drivers::plugin::character(*state);
  if (host->character(device->binding, "random", 7, 0, 0444, &operations) < 0 ||
      host->character(device->binding, "urandom", 7, 1, 0444, &operations) < 0)
  {
    return -1;
  }
  return 0;
}

const xv_kmod_driver_v2 driver{
    {sizeof(driver), XV_KMOD_ABI_V2, XV_KMOD_BUS_VIRTIO_MMIO, 4, "virtio_rng", nullptr},
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
