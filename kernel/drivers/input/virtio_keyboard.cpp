module;
#include "kernel/fs/io_status.h"
#include "kernel/fs/poll.h"
#include "kernel/io/keyboard.h"
#include "kernel/support/flow.h"

module xv.drivers;

import :keyboard;
import :platform;

namespace xv::drivers
{
namespace
{
constexpr unsigned VirtioMagic = 0x74726976;
constexpr unsigned InputDevice = 18;
constexpr unsigned DriverPresent = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
constexpr unsigned FeaturesAccepted = DriverPresent | VIRTIO_CONFIG_S_FEATURES_OK;
constexpr unsigned DriverReady = FeaturesAccepted | VIRTIO_CONFIG_S_DRIVER_OK;
constexpr unsigned ConfigEventBits = 0x11;
constexpr unsigned KeyEnter = 28;
constexpr unsigned KeyA = 30;
constexpr unsigned PageSize = 4096;
} // namespace

namespace flow = xv::flow;

bool VirtioKeyboard::identify() const
{
  return registers_.read32(VIRTIO_MMIO_MAGIC_VALUE) == VirtioMagic &&
         registers_.read32(VIRTIO_MMIO_VERSION) == 2 &&
         registers_.read32(VIRTIO_MMIO_DEVICE_ID) == InputDevice;
}

bool VirtioKeyboard::negotiate()
{
  registers_.write32(VIRTIO_MMIO_STATUS, 0);
  if (registers_.read32(VIRTIO_MMIO_STATUS) != 0)
  {
    return false;
  }
  registers_.write32(VIRTIO_MMIO_STATUS, DriverPresent);
  registers_.write32(VIRTIO_MMIO_DEVICE_FEATURES_SEL, 1);
  if (!(registers_.read32(VIRTIO_MMIO_DEVICE_FEATURES) & 1))
  {
    return false;
  }
  registers_.write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL, 0);
  registers_.write32(VIRTIO_MMIO_DRIVER_FEATURES, 0);
  registers_.write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL, 1);
  registers_.write32(VIRTIO_MMIO_DRIVER_FEATURES, 1);
  registers_.write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL, 0);
  registers_.write32(VIRTIO_MMIO_STATUS, FeaturesAccepted);
  if (!(registers_.read32(VIRTIO_MMIO_STATUS) & VIRTIO_CONFIG_S_FEATURES_OK))
  {
    return false;
  }

  // Distinguish a keyboard from other VirtIO input devices in this board slot.
  registers_.write8(VIRTIO_MMIO_CONFIG, ConfigEventBits);
  registers_.write8(VIRTIO_MMIO_CONFIG + 1, XV_KEY_EVENT);
  detail::io_barrier();
  unsigned bytes = registers_.read8(VIRTIO_MMIO_CONFIG + 2);
  if (bytes <= KeyA / 8 || bytes > 128)
  {
    return false;
  }
  auto keys = registers_.read8(VIRTIO_MMIO_CONFIG + 8 + KeyA / 8);
  return (keys & (1U << (KeyA % 8))) && (keys & (1U << (KeyEnter % 8)));
}

bool VirtioKeyboard::allocate()
{
  for (auto &page : pages_)
  {
    page = detail::allocate_dma_page();
    if (!page)
    {
      return false;
    }
    detail::zero_page(page);
  }
  return true;
}

bool VirtioKeyboard::publish()
{
  static_assert(sizeof(virtq_desc) * QueueEntries <= PageSize);
  static_assert(sizeof(AvailableRing) <= PageSize && sizeof(UsedRing) <= PageSize);
  static_assert(sizeof(KeyboardEvent) * QueueEntries <= PageSize);
  // Both queues are initialized; LED/status feedback is intentionally unused.
  for (unsigned queue = 0; queue < 2; ++queue)
  {
    registers_.write32(VIRTIO_MMIO_QUEUE_SEL, queue);
    detail::io_barrier();
    if (registers_.read32(VIRTIO_MMIO_QUEUE_READY) ||
        registers_.read32(VIRTIO_MMIO_QUEUE_NUM_MAX) < QueueEntries)
    {
      return false;
    }
    registers_.write32(VIRTIO_MMIO_QUEUE_NUM, QueueEntries);
    auto address = [&](unsigned low, unsigned high, void *page)
    {
      auto value = reinterpret_cast<unsigned long>(page);
      registers_.write32(low, static_cast<unsigned>(value));
      registers_.write32(high, static_cast<unsigned>(value >> 32));
    };
    address(VIRTIO_MMIO_QUEUE_DESC_LOW, VIRTIO_MMIO_QUEUE_DESC_HIGH, pages_[queue * 3]);
    address(VIRTIO_MMIO_DRIVER_DESC_LOW, VIRTIO_MMIO_DRIVER_DESC_HIGH,
            pages_[queue * 3 + 1]);
    address(VIRTIO_MMIO_DEVICE_DESC_LOW, VIRTIO_MMIO_DEVICE_DESC_HIGH,
            pages_[queue * 3 + 2]);
    detail::io_barrier();
    registers_.write32(VIRTIO_MMIO_QUEUE_READY, 1);
  }

  auto *descriptors = static_cast<virtq_desc *>(pages_[0]);
  auto *available = static_cast<AvailableRing *>(pages_[1]);
  auto *events = static_cast<KeyboardEvent *>(pages_[6]);
  for (unsigned i = 0; i < QueueEntries; ++i)
  {
    descriptors[i] = {reinterpret_cast<unsigned long>(&events[i]),
                      sizeof(KeyboardEvent), VRING_DESC_F_WRITE, 0};
    available->entries[i] = i;
  }
  detail::io_barrier();
  available->index = QueueEntries;
  detail::io_barrier();
  ready_ = true;
  registers_.write32(VIRTIO_MMIO_STATUS, DriverReady);
  registers_.write32(VIRTIO_MMIO_QUEUE_NOTIFY, 0);
  return true;
}

void VirtioKeyboard::rollback()
{
  ready_ = false;
  registers_.write32(VIRTIO_MMIO_STATUS, 0);
  detail::io_barrier();
  auto begin = detail::terminal_milliseconds();
  while (registers_.read32(VIRTIO_MMIO_STATUS))
  {
    if (detail::terminal_milliseconds() - begin >= 100)
    {
      return; // Retain DMA pages until the device can no longer access them.
    }
  }
  for (auto &page : pages_)
  {
    if (page)
    {
      detail::free_dma_page(page);
      page = nullptr;
    }
  }
}

struct VirtioKeyboard::InitializeFlow
{
  struct Context
  {
    VirtioKeyboard &owner;
    bool okay = false;
  };

  static bool found(Context &context) noexcept
  {
    return context.owner.identify();
  }

  static void negotiate(Context &context) noexcept
  {
    context.okay = context.owner.negotiate();
  }

  static void allocate(Context &context) noexcept
  {
    context.okay = context.owner.allocate();
  }

  static void publish(Context &context) noexcept
  {
    context.okay = context.owner.publish();
  }

  static bool okay(Context &context) noexcept
  {
    return context.okay;
  }

  static void fail(Context &context) noexcept
  {
    context.owner.rollback();
  }

  struct Found;
  struct Negotiate;
  struct Allocate;
  struct Publish;
  struct Fail;
  using Graph = flow::graph<
      Found,
      flow::types<
          flow::node<
              Found, flow::noop,
              flow::branch<flow::condition<&found>, flow::go<Negotiate>, flow::stop>>,
          flow::node<
              Negotiate, flow::action<&negotiate>,
              flow::branch<flow::condition<&okay>, flow::go<Allocate>, flow::go<Fail>>>,
          flow::node<
              Allocate, flow::action<&allocate>,
              flow::branch<flow::condition<&okay>, flow::go<Publish>, flow::go<Fail>>>,
          flow::node<Publish, flow::action<&publish>,
                     flow::branch<flow::condition<&okay>, flow::stop, flow::go<Fail>>>,
          flow::node<Fail, flow::action<&fail>, flow::stop>>>;
};

void VirtioKeyboard::initialize()
{
  lock_.init("virtio keyboard");
  InitializeFlow::Context context{*this};
  (void)InitializeFlow::Graph::run<0>(context);
}

CharacterDevice *VirtioKeyboard::open(bool)
{
  xv::sync::ScopedLock guard(lock_);
  if (!ready_ || opened_ || detail::effective_uid() != 0)
  {
    return nullptr;
  }
  head_ = size_ = 0;
  opened_ = true;
  return this;
}

void VirtioKeyboard::close()
{
  xv::sync::ScopedLock guard(lock_);
  opened_ = false;
  head_ = size_ = 0;
}

void VirtioKeyboard::enqueue(const KeyboardEvent &event)
{
  if (!opened_)
  {
    return;
  }
  if (size_ == BufferedEvents)
  {
    head_ = 0;
    size_ = 1;
    buffered_[0] = {XV_KEY_SYN, XV_KEY_SYN_DROPPED, 0};
  }
  buffered_[(head_ + size_) % BufferedEvents] = event;
  ++size_;
}

bool VirtioKeyboard::drain()
{
  auto *used = static_cast<volatile UsedRing *>(pages_[2]);
  unsigned short end = used->index;
  unsigned count = static_cast<unsigned short>(end - used_index_);
  if (count > QueueEntries)
  {
    return false;
  }
  detail::io_barrier();
  bool completed[QueueEntries]{};
  unsigned ids[QueueEntries];
  for (unsigned i = 0; i < count; ++i)
  {
    const auto &entry = used->entries[(used_index_ + i) % QueueEntries];
    unsigned id = entry.id;
    if (id >= QueueEntries || entry.len != sizeof(KeyboardEvent) || completed[id])
    {
      return false;
    }
    completed[id] = true;
    ids[i] = id;
  }

  auto *available = static_cast<AvailableRing *>(pages_[1]);
  auto *events = static_cast<volatile KeyboardEvent *>(pages_[6]);
  unsigned short next = available->index;
  for (unsigned i = 0; i < count; ++i)
  {
    unsigned id = ids[i];
    KeyboardEvent event{events[id].type, events[id].code, events[id].value};
    if (event.type == XV_KEY_SYN || (event.type == XV_KEY_EVENT && event.value <= 2))
    {
      enqueue(event);
    }
    available->entries[next % QueueEntries] = id;
    ++next;
  }
  used_index_ = end;
  detail::io_barrier();
  available->index = next;
  detail::io_barrier();
  if (count)
  {
    registers_.write32(VIRTIO_MMIO_QUEUE_NOTIFY, 0);
  }
  return true;
}

void VirtioKeyboard::disable()
{
  ready_ = false;
  registers_.write32(VIRTIO_MMIO_STATUS, 0);
  detail::io_barrier();
  // IRQ context must not wait for reset. Retain DMA ownership after failure.
  detail::wake_processes(&size_);
}

struct VirtioKeyboard::InterruptFlow
{
  struct Context
  {
    VirtioKeyboard &owner;
    bool okay = false;
  };

  static void acknowledge(Context &context) noexcept
  {
    auto &registers = context.owner.registers_;
    registers.write32(VIRTIO_MMIO_INTERRUPT_ACK,
                      registers.read32(VIRTIO_MMIO_INTERRUPT_STATUS) & 3);
    detail::io_barrier();
  }

  static bool ready(Context &context) noexcept
  {
    return context.owner.ready_;
  }

  static void drain(Context &context) noexcept
  {
    context.okay = context.owner.drain();
  }

  static bool okay(Context &context) noexcept
  {
    return context.okay;
  }

  static void fail(Context &context) noexcept
  {
    context.owner.disable();
  }

  static void wake(Context &context) noexcept
  {
    if (context.owner.size_)
    {
      detail::wake_processes(&context.owner.size_);
    }
  }

  struct Acknowledge;
  struct Drain;
  struct Wake;
  struct Fail;
  using Graph =
      flow::graph<Acknowledge,
                  flow::types<flow::node<Acknowledge, flow::action<&acknowledge>,
                                         flow::branch<flow::condition<&ready>,
                                                      flow::go<Drain>, flow::stop>>,
                              flow::node<Drain, flow::action<&drain>,
                                         flow::branch<flow::condition<&okay>,
                                                      flow::go<Wake>, flow::go<Fail>>>,
                              flow::node<Wake, flow::action<&wake>, flow::stop>,
                              flow::node<Fail, flow::action<&fail>, flow::stop>>>;
};

void VirtioKeyboard::handle_interrupt()
{
  xv::sync::ScopedLock guard(lock_);
  InterruptFlow::Context context{*this};
  (void)InterruptFlow::Graph::run<0>(context);
}

struct VirtioKeyboard::ReadFlow
{
  struct Context
  {
    VirtioKeyboard &owner;
    IoBuffer destination;
    int count;
    int result = -1;
  };

  static bool stopped(Context &context) noexcept
  {
    return !context.owner.ready_ || !context.owner.opened_ || detail::process_killed();
  }

  static bool available(Context &context) noexcept
  {
    return context.owner.size_ != 0;
  }

  static bool nonblocking(Context &context) noexcept
  {
    return context.destination.nonblocking;
  }

  static void again(Context &context) noexcept
  {
    context.result = -EAGAIN;
  }

  static void sleep(Context &context) noexcept
  {
    detail::prepare_interruptible_sleep(&context.owner.size_);
    context.owner.lock_.release();
    detail::sleep_process();
    context.owner.lock_.acquire();
  }

  static void copy(Context &context) noexcept
  {
    auto &owner = context.owner;
    unsigned count = static_cast<unsigned>(context.count) / sizeof(KeyboardEvent);
    if (count > owner.size_)
    {
      count = owner.size_;
    }
    if (count > 32)
    {
      count = 32;
    }
    KeyboardEvent events[32];
    for (unsigned i = 0; i < count; ++i)
    {
      events[i] = owner.buffered_[(owner.head_ + i) % BufferedEvents];
    }
    unsigned bytes = count * sizeof(KeyboardEvent);
    if (detail::copy_out(context.destination.space == AddressSpace::User,
                         context.destination.address, events, bytes) < 0)
    {
      return;
    }
    owner.head_ = (owner.head_ + count) % BufferedEvents;
    owner.size_ -= count;
    context.result = bytes;
  }

  struct Check;
  struct Available;
  struct Nonblocking;
  struct Again;
  struct Sleep;
  struct Copy;
  using Graph = flow::graph<
      Check,
      flow::types<flow::node<Check, flow::noop,
                             flow::branch<flow::condition<&stopped>, flow::stop,
                                          flow::go<Available>>>,
                  flow::node<Available, flow::noop,
                             flow::branch<flow::condition<&available>, flow::go<Copy>,
                                          flow::go<Nonblocking>>>,
                  flow::node<Nonblocking, flow::noop,
                             flow::branch<flow::condition<&nonblocking>,
                                          flow::go<Again>, flow::go<Sleep>>>,
                  flow::node<Again, flow::action<&again>, flow::stop>,
                  flow::node<Sleep, flow::action<&sleep>, flow::go<Check>>,
                  flow::node<Copy, flow::action<&copy>, flow::stop>>>;
};

int VirtioKeyboard::read(IoBuffer destination, int count)
{
  if (count == 0)
  {
    return 0;
  }
  if (count < 0 || count % sizeof(KeyboardEvent) || detail::effective_uid() != 0)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(lock_);
  ReadFlow::Context context{*this, destination, count};
  (void)ReadFlow::Graph::run<0>(context);
  return context.result;
}

unsigned VirtioKeyboard::poll(xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(lock_);
  if (table)
  {
    table->watch(&size_);
  }
  if (!ready_ || !opened_ || detail::effective_uid() != 0)
  {
    return POLLERR;
  }
  return size_ ? POLLIN : 0;
}
} // namespace xv::drivers
