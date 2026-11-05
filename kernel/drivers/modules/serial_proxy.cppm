module;
#include "kernel/io/kmodule_v2.h"
#include "kernel/memory/layout/memlayout.h"
module xv.drivers:serial_proxy;
import :module_runtime;
import :serial;

namespace xv::drivers::modules
{
class SerialProxy final : public SerialPort
{
public:

  constexpr SerialProxy() = default;

  const char *
  name() const override {
    return "serial transport";
  }

  void configure(unsigned long base, unsigned irq) { base_ = base; irq_ = irq; }

  unsigned
  irq() const override {
    return irq_;
  }

  void
  initialize() override {
    auto *registers = reinterpret_cast<volatile unsigned char *>(base_);
    registers[1] = 0;
    registers[3] = 3;
  }

  void
  attach(InputReceiver &receiver) override {
    receiver_ = &receiver;
  }

  void
  receive(int character) {
    if (receiver_)
    {
      receiver_->receive(character);
    }
  }

  void
  publish(Binding &binding) {
    __atomic_store_n(&binding_, &binding, __ATOMIC_RELEASE);
  }

  int
  transmit(const char *data, int count, bool nonblocking) override {
    auto *binding = __atomic_load_n(&binding_, __ATOMIC_ACQUIRE);
    if (binding)
    {
      xv_kmod_synchronize();
      return binding->serial.transmit(binding->serial.context, data, count,
                                      nonblocking);
    }
    for (int i = 0; i < count; ++i)
    {
      put_sync(data[i]);
    }
    return count;
  }

  void
  write(const char *data, int count) override {
    transmit(data, count, false);
  }

  void
  put_sync(int character) override {
    auto *binding = __atomic_load_n(&binding_, __ATOMIC_ACQUIRE);
    if (binding)
    {
      xv_kmod_synchronize();
      binding->serial.put(binding->serial.context, character);
      return;
    }
    xv::sync::push_interrupts_off();
    auto *registers = reinterpret_cast<volatile unsigned char *>(base_);
    while (!(registers[5] & 0x20))
    {
    }
    registers[0] = character;
    xv::sync::pop_interrupts_off();
  }

  void
  drain() override {
    auto *binding = __atomic_load_n(&binding_, __ATOMIC_ACQUIRE);
    if (binding)
    {
      xv_kmod_synchronize();
      binding->serial.drain(binding->serial.context);
    }
  }

  bool
  writable(xv::sync::PollTable *table) override {
    auto *binding = __atomic_load_n(&binding_, __ATOMIC_ACQUIRE);
    if (!binding)
    {
      return true;
    }
    xv_kmod_synchronize();
    return binding->serial.writable(binding->serial.context, table);
  }

  void
  handle_interrupt() override {
  }

private:

  unsigned long base_ = UART0; // Emergency output before validated DT discovery.
  unsigned irq_ = UART0_IRQ;

  Binding *binding_ = nullptr;
  InputReceiver *receiver_ = nullptr;
};

extern SerialProxy serial;
} // namespace xv::drivers::modules
