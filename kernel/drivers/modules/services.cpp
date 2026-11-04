module;
#include "kernel/io/kmodule_v2.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/support/placement_new.hpp"
module xv.drivers;
import :module_runtime;
import :serial_proxy;
import :platform;
import xv.fs;
import xv.memory;
import xv.process;
import xv.runtime;
import xv.print;

namespace xv::drivers::modules
{
bool
callback(const Image &image, unsigned long address) {
  if (address < image.base || address - image.base >= image.bytes || address & 1)
  {
    return false;
  }
  xv::memory::LargestPagePolicy policy;
  xv::memory::Sv39PageTable table(
      const_cast<void *>(xv::memory::kernel_page_table().root()),
      xv::memory::allocator(), policy);
  xv::memory::PageTranslation translation;
  return table.translate(address, translation) && (translation.permissions & PTE_X);
}

constinit SerialProxy serial;

namespace
{
class CharacterProxy final : public CharacterDevice
{
public:

  constexpr CharacterProxy() = default;

  void
  configure(const char *name, const xv_kmod_character &operations, bool session) {
    xv::runtime::copy_fixed(name_, name, sizeof(name_));
    operations_ = operations;
    session_ = session;
  }

  const char *
  name() const override {
    return name_;
  }

  void
  initialize() override {
  }

  CharacterDevice *
  open(bool nonblocking) override {
    xv_kmod_synchronize();
    void *context = operations_.open(operations_.context, nonblocking);
    if (!context)
    {
      return nullptr;
    }
    auto *page = xv::memory::allocator().allocate();
    if (!page)
    {
      operations_.close(context);
      return nullptr;
    }
    auto *session = new (page) CharacterProxy;
    auto operations = operations_;
    operations.context = context;
    session->configure(name_, operations, true);
    return session;
  }

  int
  read(IoBuffer destination, int count) override {
    xv_kmod_synchronize();
    return operations_.read(operations_.context,
                            {destination.address,
                             destination.space == AddressSpace::User,
                             destination.nonblocking},
                            count);
  }

  int
  write(IoBuffer source, int count) override {
    xv_kmod_synchronize();
    return operations_.write(
        operations_.context,
        {source.address, source.space == AddressSpace::User, source.nonblocking},
        count);
  }

  void
  put_sync(int) override {
  }

  int
  ioctl(unsigned request, IoBuffer argument) override {
    xv_kmod_synchronize();
    return operations_.ioctl(
        operations_.context, request,
        {argument.address, argument.space == AddressSpace::User, argument.nonblocking});
  }

  unsigned
  poll(xv::sync::PollTable *table) override {
    xv_kmod_synchronize();
    return operations_.poll(operations_.context, table);
  }

  void
  close() override {
    xv_kmod_synchronize();
    operations_.close(operations_.context);
    if (session_)
    {
      this->~CharacterProxy();
      xv::memory::allocator().deallocate(this);
    }
  }

private:

  xv_kmod_character operations_{};
  char name_[14]{};
  bool session_ = false;
};

constinit CharacterProxy characters[32];
unsigned character_count = 0;

class NetworkProxy final : public EthernetDevice
{
public:

  constexpr NetworkProxy() = default;

  Binding *binding = nullptr;
  EthernetReceiver *receiver = nullptr;

  bool
  online() const override {
    xv_kmod_synchronize();
    return binding && binding->ethernet.online(binding->ethernet.context);
  }

  bool
  carrier() const override {
    xv_kmod_synchronize();
    return binding && binding->ethernet.carrier(binding->ethernet.context);
  }

  void
  mac_address(unsigned char *out) const override {
    xv_kmod_synchronize();
    if (binding)
    {
      binding->ethernet.mac(binding->ethernet.context, out);
    }
    else
    {
      xv::runtime::fill(out, 0, 6);
    }
  }

  bool
  transmit(void *page, unsigned count) override {
    if (!binding || !binding->image->pool.adopt(page))
    {
      return false;
    }
    xv_kmod_synchronize();
    bool accepted = binding->ethernet.transmit(binding->ethernet.context, page, count);
    if (!accepted)
    {
      binding->image->pool.detach(page);
    }
    else
    {
      __atomic_fetch_add(&binding->image->transfers, 1UL, __ATOMIC_RELAXED);
    }
    return accepted;
  }

  bool
  promiscuous(bool enabled) override {
    xv_kmod_synchronize();
    return binding && binding->ethernet.promiscuous(binding->ethernet.context, enabled);
  }
};

constinit NetworkProxy networks[4];
unsigned network_size = 0;

void *
page(void *owner) {
  return static_cast<Image *>(owner)->pool.allocate();
}

void
free_page(void *owner, void *address, bool shared) {
  static_cast<Image *>(owner)->pool.release(address, shared);
}

void *
cpu() {
  return xv::process::current_cpu();
}

int
pid() {
  return xv::process::current() ? xv::process::current()->id() : 0;
}

void
poll_watch(void *table, void *channel) {
  if (table)
  {
    static_cast<xv::sync::PollTable *>(table)->watch(channel);
  }
}

unsigned
poll_events(void *table) {
  return table ? static_cast<xv::sync::PollTable *>(table)->events() : 0;
}

int
character(void *token, const char *name, unsigned major, unsigned minor, unsigned mode,
          const xv_kmod_character *operations) {
  auto &binding = *static_cast<Binding *>(token);
  const auto &image = *binding.image;
  if (character_count == 32 || !operations ||
      !callback(image, reinterpret_cast<unsigned long>(operations->open)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->close)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->read)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->write)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->ioctl)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->poll)))
  {
    return -1;
  }
  auto &proxy = characters[character_count];
  proxy.configure(name, *operations, false);
  if (!xv::fs::devices().attach_dynamic_character(name, major, minor, mode, proxy))
  {
    return -1;
  }
  ++character_count;
  return 0;
}

int
set_serial(void *token, const xv_kmod_serial *operations) {
  auto &binding = *static_cast<Binding *>(token);
  const auto &image = *binding.image;
  if (!operations ||
      !callback(image, reinterpret_cast<unsigned long>(operations->transmit)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->put)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->drain)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->writable)))
  {
    return -1;
  }
  binding.serial = *operations;
  serial.publish(binding);
  return 0;
}

int
set_network(void *token, const xv_kmod_ethernet *operations) {
  auto &binding = *static_cast<Binding *>(token);
  const auto &image = *binding.image;
  if (network_size == 4 ||
      __atomic_load_n(&binding.network_slot, __ATOMIC_ACQUIRE) != ~0U || !operations ||
      !callback(image, reinterpret_cast<unsigned long>(operations->online)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->carrier)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->mac)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->transmit)) ||
      !callback(image, reinterpret_cast<unsigned long>(operations->promiscuous)))
  {
    return -1;
  }
  binding.ethernet = *operations;
  networks[network_size].binding = &binding;
  __atomic_store_n(&binding.network_slot, network_size, __ATOMIC_RELEASE);
  __atomic_store_n(&network_size, network_size + 1, __ATOMIC_RELEASE);
  return 0;
}

int
set_interrupt(void *token, void *context, void (*handler)(void *)) {
  auto &binding = *static_cast<Binding *>(token);
  if (handler && !callback(*binding.image, reinterpret_cast<unsigned long>(handler)))
  {
    return -1;
  }
  xv::sync::ScopedLock guard(binding.interrupt_lock);
  binding.interrupt_context = context;
  binding.interrupt_handler = handler;
  __atomic_store_n(&binding.interrupt_active, handler ? 1U : 0U, __ATOMIC_RELEASE);
  return 0;
}

void
serial_input(void *, int character) {
  serial.receive(character);
}

void
ethernet_input(void *token, void *page, unsigned count) {
  auto &binding = *static_cast<Binding *>(token);
  binding.image->pool.detach(page);
  auto slot = __atomic_load_n(&binding.network_slot, __ATOMIC_ACQUIRE);
  auto *receiver = slot < __atomic_load_n(&network_size, __ATOMIC_ACQUIRE)
                       ? __atomic_load_n(&networks[slot].receiver, __ATOMIC_ACQUIRE)
                       : nullptr;
  if (receiver)
  {
    receiver->receive_frame(page, count);
  }
  else
  {
    xv::memory::allocator().deallocate(page);
  }
}
} // namespace

int
attach(Image &image, const xv_kmod_device_v2 &device) {
  if (!image.services.abi)
  {
    image.pool.initialize();
    image.services = {sizeof(xv_kmod_host_v2),
                      XV_KMOD_ABI_V2,
                      &image,
                      page,
                      free_page,
                      xv::sync::push_interrupts_off,
                      xv::sync::pop_interrupts_off,
                      cpu,
                      pid,
                      detail::prepare_sleep,
                      detail::prepare_interruptible_sleep,
                      detail::sleep_process,
                      detail::wake_processes,
                      xv::process::wakeup_one,
                      detail::process_killed,
                      xv::print::panic,
                      detail::panicking,
                      detail::panicked,
                      detail::effective_uid,
                      detail::effective_gid,
                      detail::terminal_milliseconds,
                      detail::copy_in,
                      detail::copy_out,
                      detail::wait_device_completion,
                      poll_watch,
                      poll_events,
                      detail::map_dma_pages,
                      detail::mapped_dma_pages,
                      detail::unmap_dma_pages,
                      character,
                      set_serial,
                      set_network,
                      set_interrupt,
                      serial_input,
                      ethernet_input};
  }
  auto *driver = reinterpret_cast<const xv_kmod_driver_v2 *>(image.driver);
  xv_kmod_synchronize();
  if (driver->attach(&image.services, &device) < 0)
  {
    set_interrupt(device.binding, nullptr, nullptr);
    return -1;
  }
  ++image.bound;
  auto &binding = *static_cast<Binding *>(device.binding);
  __atomic_store_n(&online[binding.index], 1U, __ATOMIC_RELEASE);
  return 0;
}

SerialPort &
serial_proxy() {
  return serial;
}

unsigned
network_count() {
  return __atomic_load_n(&network_size, __ATOMIC_ACQUIRE);
}

EthernetDevice &
network_proxy(unsigned index) {
  if (index >= 4)
  {
    xv::print::panic("driver: invalid network index");
  }
  return networks[index];
}

void
network_receiver(unsigned index, EthernetReceiver &receiver) {
  if (index >= network_count())
  {
    return;
  }
  __atomic_store_n(&networks[index].receiver, &receiver, __ATOMIC_RELEASE);
}
} // namespace xv::drivers::modules
