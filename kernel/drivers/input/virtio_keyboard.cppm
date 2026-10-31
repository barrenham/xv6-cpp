module;
#include "kernel/io/keyboard.h"

module xv.drivers:keyboard;

import :character;
import :interrupt;
import :protocol;
import :mmio;
import xv.sync;

namespace xv::drivers
{
class VirtioKeyboard final : public CharacterDevice, public InterruptDevice
{
public:

  constexpr VirtioKeyboard(unsigned long base, unsigned irq)
      : registers_(base), irq_(irq)
  {
  }

  const char *name() const override
  {
    return "VirtIO keyboard";
  }

  unsigned irq() const override
  {
    return irq_;
  }

  void initialize() override;
  CharacterDevice *open(bool nonblocking) override;
  void close() override;
  int read(IoBuffer destination, int count) override;
  unsigned poll(xv::sync::PollTable *table = nullptr) override;
  void handle_interrupt() override;

  int write(IoBuffer, int count) override
  {
    return count == 0 ? 0 : -1;
  }

  void put_sync(int) override
  {
  }

private:

  struct InitializeFlow;
  struct ReadFlow;
  struct InterruptFlow;

  static constexpr unsigned QueueEntries = 64;
  static constexpr unsigned BufferedEvents = 256;
  static constexpr unsigned DmaPages = 7;

  struct AvailableRing
  {
    unsigned short flags;
    unsigned short index;
    unsigned short entries[QueueEntries];
    unsigned short unused;
  };

  struct UsedRing
  {
    unsigned short flags;
    unsigned short index;
    virtq_used_elem entries[QueueEntries];
  };

  detail::MmioRegion registers_;
  unsigned irq_;
  xv::sync::SpinLock lock_;
  // Event queue, idle status queue, then device-writable event slots.
  void *pages_[DmaPages]{};
  KeyboardEvent buffered_[BufferedEvents]{};
  unsigned head_ = 0;
  unsigned size_ = 0;
  unsigned short used_index_ = 0;
  bool ready_ = false;
  bool opened_ = false;

  bool identify() const;
  bool negotiate();
  bool allocate();
  bool publish();
  void rollback();
  void disable();
  void enqueue(const KeyboardEvent &event);
  bool drain();
};
} // namespace xv::drivers
