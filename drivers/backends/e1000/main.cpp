module;
#include "drivers/sdk/host.h"
#include "kernel/io/kmodule_v4.h"
#include "kernel/support/placement_new.hpp"
module xv.drivers;
import :e1000;

namespace
{
class Receiver final : public xv::drivers::EthernetReceiver
{
public:

  explicit Receiver(void *binding) : binding_(binding) {
  }

  void
  receive_frame(void *page, unsigned count) override {
    xv_driver_host()->ethernet_input(binding_, page, count);
  }

private:

  void *binding_;
};

struct State
{
  xv::drivers::E1000 device;
  Receiver receiver;
  void *token = nullptr;
  State *next = nullptr;

  explicit State(const xv_kmod_device_v2 &resource)
      : device(resource.base.mmio, resource.base.irq), receiver(resource.binding) {
    device.authorize_resources(resource.flags & XV_KMOD_DEVICE_PCI_READY);
    device.attach(receiver);
  }
};

State *states = nullptr;

void
interrupt(void *context) {
  static_cast<State *>(context)->device.handle_interrupt();
}

bool
online(void *context) {
  return static_cast<State *>(context)->device.online();
}

bool
carrier(void *context) {
  return static_cast<State *>(context)->device.carrier();
}

void
mac(void *context, unsigned char *out) {
  static_cast<State *>(context)->device.mac_address(out);
}

bool
transmit(void *context, void *page, unsigned count) {
  return static_cast<State *>(context)->device.transmit(page, count);
}

bool
promiscuous(void *context, bool enabled) {
  return static_cast<State *>(context)->device.promiscuous(enabled);
}

int
attach(const xv_kmod_host_v2 *host, const xv_kmod_device_v3 *identity) {
  const auto *resource = &identity->base;
  if (resource->base.vendor != 0x8086)
  {
    return -1;
  }
  xv_driver_set_host(host);
  static_assert(sizeof(State) <= 4096);
  auto *page = host->page(host->owner);
  if (!page)
  {
    return -1;
  }
  auto *state = new (page) State(*resource);
  if (host->interrupt(resource->binding, state, interrupt) < 0)
  {
    host->free_page(host->owner, page, false);
    return -1;
  }
  state->device.initialize();
  if (!state->device.online())
  {
    host->interrupt(resource->binding, nullptr, nullptr);
    host->free_page(host->owner, page, false);
    return -1;
  }
  xv_kmod_ethernet operations{state, online, carrier, mac, transmit, promiscuous};
  int result = host->ethernet(resource->binding, &operations);
  if (result < 0)
  {
    host->interrupt(resource->binding, nullptr, nullptr);
    if (!state->device.shutdown(false)) host->panic("e1000: DMA reset failed during rollback");
    host->free_page(host->owner, page, false);
    return -1;
  }
  state->token = resource->binding;
  state->next = states;
  states = state;
  return 0;
}

int
remove(const xv_kmod_host_v2 *host, const xv_kmod_device_v3 *identity,
       unsigned reason) {
  State **link = &states;
  while (*link && (*link)->token != identity->base.binding)
  {
    link = &(*link)->next;
  }
  if (!*link)
  {
    return -1;
  }
  auto *state = *link;
  if (!state->device.shutdown(reason == XV_KMOD_REMOVE_SURPRISE))
  {
    return -1;
  }
  *link = state->next;
  state->~State();
  host->free_page(host->owner, state, false);
  return 0;
}

const xv_kmod_match matches[]{
    {.bus = XV_KMOD_BUS_PCI,
     .flags = XV_KMOD_MATCH_VENDOR | XV_KMOD_MATCH_DEVICE | XV_KMOD_MATCH_CLASS,
     .vendor = 0x8086,
     .device = 0x100e,
     .class_code = 0x020000,
     .class_mask = 0xffff00}};
const xv_kmod_driver_v4 driver{
    {{sizeof(driver), XV_KMOD_ABI_V4, XV_KMOD_BUS_PCI, 0x100e, "e1000", nullptr},
     matches,
     1,
     0,
     nullptr,
     attach},
    remove};
} // namespace

extern "C"
{
  [[gnu::visibility("default")]]
  const xv_kmod_driver *
  xv_driver_entry(unsigned abi) {
    return abi == XV_KMOD_ABI_V4 ? &driver.base.base : nullptr;
  }
}
