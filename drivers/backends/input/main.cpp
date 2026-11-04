module;
#include "drivers/sdk/host.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/io/keyboard.h"
#include "kernel/fs/poll.h"
module xv.drivers;
import :keyboard;

#include "drivers/sdk/adapters.inc"

namespace
{
void
interrupt(void *context) {
  static_cast<xv::drivers::VirtioKeyboard *>(context)->handle_interrupt();
}

int
attach(const xv_kmod_host_v2 *host, const xv_kmod_device_v2 *device) {
  xv_driver_set_host(host);
  auto *config = reinterpret_cast<volatile unsigned char *>(device->base.mmio + 0x100);
  config[0] = 0x11;
  config[1] = XV_INPUT_REL;
  asm volatile("fence iorw, iorw" ::: "memory");
  bool wheel = config[2] > XV_REL_WHEEL / 8 &&
               (config[8 + XV_REL_WHEEL / 8] & (1U << (XV_REL_WHEEL % 8)));
  auto *page = host->page(host->owner);
  if (!page)
  {
    return -1;
  }
  static_assert(sizeof(xv::drivers::VirtioKeyboard) <= 4096);
  auto *state =
      new (page) xv::drivers::VirtioKeyboard(device->base.mmio, device->base.irq);
  if (host->interrupt(device->binding, state, interrupt) < 0)
  {
    host->free_page(host->owner, page, false);
    return -1;
  }
  if (wheel)
  {
    state->initialize_wheel();
  }
  else
  {
    state->initialize();
  }
  if (!state->open(true))
  {
    host->interrupt(device->binding, nullptr, nullptr);
    host->free_page(host->owner, page, false);
    return -1;
  }
  state->close();
  auto operations = xv::drivers::plugin::character(*state);
  return host->character(device->binding, wheel ? "mouse0" : "kbd0", 30, wheel ? 1 : 0,
                         0600, &operations);
}

const xv_kmod_driver_v2 driver{{sizeof(driver), XV_KMOD_ABI_V2, XV_KMOD_BUS_VIRTIO_MMIO,
                                18, "virtio_input", nullptr},
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
