module;
#include "drivers/sdk/host.h"
#include "kernel/support/placement_new.hpp"
module xv.drivers;
import :uart;

#include "drivers/sdk/adapters.inc"

namespace
{
class Receiver final : public xv::drivers::InputReceiver
{
public:

  explicit Receiver(void *binding) : binding_(binding) {
  }

  void
  receive(int character) override {
    xv_driver_host()->serial_input(binding_, character);
  }

private:

  void *binding_;
};

struct State
{
  xv::drivers::Uart16550 port;
  Receiver receiver;

  explicit State(const xv_kmod_device_v2 &device)
      : port(device.base.mmio, device.base.irq), receiver(device.binding) {
    port.attach(receiver);
  }
};

void
interrupt(void *context) {
  static_cast<State *>(context)->port.handle_interrupt();
}

int
transmit(void *context, const char *bytes, int count, bool nonblocking) {
  return static_cast<State *>(context)->port.transmit(bytes, count, nonblocking);
}

void
put(void *context, int character) {
  static_cast<State *>(context)->port.put_sync(character);
}

void
drain(void *context) {
  static_cast<State *>(context)->port.drain();
}

bool
writable(void *context, void *table) {
  xv::drivers::plugin::PollAdapter adapter(table);
  return static_cast<State *>(context)->port.writable(table ? &adapter : nullptr);
}

int
attach(const xv_kmod_host_v2 *host, const xv_kmod_device_v2 *device) {
  xv_driver_set_host(host);
  static_assert(sizeof(State) <= 4096);
  auto *page = host->page(host->owner);
  if (!page)
  {
    return -1;
  }
  auto *state = new (page) State(*device);
  if (host->interrupt(device->binding, state, interrupt) < 0)
  {
    host->free_page(host->owner, page, false);
    return -1;
  }
  state->port.initialize();
  xv_kmod_serial operations{state, transmit, put, drain, writable};
  return host->serial(device->binding, &operations);
}

const xv_kmod_driver_v2 driver{
    {sizeof(driver), XV_KMOD_ABI_V2, XV_KMOD_BUS_PLATFORM, 1, "uart16550", nullptr},
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
