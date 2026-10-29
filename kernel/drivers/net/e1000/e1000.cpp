module;
#include "kernel/support/flow.h"

module xv.drivers;
import :e1000;
import :pci;
import :platform;

namespace xv::drivers
{

namespace flow = xv::flow;

struct E1000::InitializeFlow
{
  struct Context
  {
    E1000 &owner;
    unsigned index = 0;
    bool ok = false;
  };

  static void lock(Context &c) noexcept
  {
    c.owner.lock_.init("e1000");
  }

  static bool configured(Context &c) noexcept
  {
    return c.owner.receiver_ && detail::configure_e1000(c.owner.base_, c.owner.slot_);
  }

  static void reset(Context &c) noexcept
  {
    c.owner.registers_.write32(0xd8, ~0U);
    c.owner.registers_.write32(0, c.owner.registers_.read32(0) | (1U << 26));
    unsigned tries = 100000;
    while ((c.owner.registers_.read32(0) & (1U << 26)) && --tries)
    {
    }
    c.ok = tries != 0;
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void mac(Context &c) noexcept
  {
    c.owner.registers_.write32(0xd8, ~0U);
    c.owner.registers_.read32(0xc0);
    for (unsigned word = 0; word < 3; ++word)
    {
      c.owner.registers_.write32(0x14, 1 | (word << 8));
      unsigned budget = 100000;
      while (!(c.owner.registers_.read32(0x14) & 0x10) && --budget)
      {
      }
      if (!budget)
      {
        c.ok = false;
        return;
      }
      unsigned value = c.owner.registers_.read32(0x14) >> 16;
      c.owner.mac_[word * 2] = value;
      c.owner.mac_[word * 2 + 1] = value >> 8;
    }
    c.ok = !(c.owner.mac_[0] & 1);
  }

  static void allocate(Context &c) noexcept
  {
    for (; c.index < Count; ++c.index)
    {
      c.owner.received_[c.index] = detail::allocate_dma_page();
      if (!c.owner.received_[c.index])
      {
        c.ok = false;
        return;
      }
      c.owner.rx_[c.index].address =
          reinterpret_cast<unsigned long>(c.owner.received_[c.index]);
      c.owner.tx_[c.index].status = 1;
    }
    c.ok = true;
  }

  static void rollback(Context &c) noexcept
  {
    while (c.index)
    {
      detail::free_dma_page(c.owner.received_[--c.index]);
    }
  }

  static void program(Context &c) noexcept
  {
    auto tx = reinterpret_cast<unsigned long>(c.owner.tx_);
    auto rx = reinterpret_cast<unsigned long>(c.owner.rx_);
    c.owner.registers_.write32(0x3800, tx);
    c.owner.registers_.write32(0x3804, tx >> 32);
    c.owner.registers_.write32(0x3808, sizeof(c.owner.tx_));
    c.owner.registers_.write32(0x3810, 0);
    c.owner.registers_.write32(0x3818, 0);
    c.owner.registers_.write32(0x2800, rx);
    c.owner.registers_.write32(0x2804, rx >> 32);
    c.owner.registers_.write32(0x2808, sizeof(c.owner.rx_));
    c.owner.registers_.write32(0x2810, 0);
    c.owner.registers_.write32(0x2818, Count - 1);
    c.owner.registers_.write32(0x5400, unsigned(c.owner.mac_[0]) |
                                           (unsigned(c.owner.mac_[1]) << 8) |
                                           (unsigned(c.owner.mac_[2]) << 16) |
                                           (unsigned(c.owner.mac_[3]) << 24));
    c.owner.registers_.write32(0x5404, 0x80000000U | c.owner.mac_[4] |
                                           (unsigned(c.owner.mac_[5]) << 8));
    for (unsigned i = 0; i < 128; ++i)
    {
      c.owner.registers_.write32(0x5200 + i * 4, 0);
    }
    c.owner.registers_.write32(0x410, 10 | (8 << 10) | (6 << 20));
    c.owner.registers_.write32(0x400, 2 | 8 | (0x10 << 4) | (0x40 << 12));
    c.owner.registers_.write32(0x2820, 0);
    c.owner.registers_.write32(0x282c, 0);
  }

  static void start(Context &c) noexcept
  {
    detail::io_barrier();
    c.owner.ready_ = true;
    c.owner.registers_.write32(0x100, 2 | (1 << 15) | (1 << 26));
    c.owner.registers_.write32(0xd0, (1 << 7) | 1);
  }
  struct Lock;
  struct Reset;
  struct Mac;
  struct Allocate;
  struct Rollback;
  struct Program;
  struct Start;
  using Graph = flow::graph<
      Lock,
      flow::types<
          flow::node<
              Lock, flow::action<&lock>,
              flow::branch<flow::condition<&configured>, flow::go<Reset>, flow::stop>>,
          flow::node<Reset, flow::action<&reset>,
                     flow::branch<flow::condition<&ok>, flow::go<Mac>, flow::stop>>,
          flow::node<
              Mac, flow::action<&mac>,
              flow::branch<flow::condition<&ok>, flow::go<Allocate>, flow::stop>>,
          flow::node<Allocate, flow::action<&allocate>,
                     flow::branch<flow::condition<&ok>, flow::go<Program>,
                                  flow::go<Rollback>>>,
          flow::node<Rollback, flow::action<&rollback>, flow::stop>,
          flow::node<Program, flow::action<&program>, flow::go<Start>>,
          flow::node<Start, flow::action<&start>, flow::stop>>>;
};

struct E1000::TransmitFlow
{
  struct Context
  {
    E1000 &owner;
    void *page;
    unsigned length;
    TxDescriptor *descriptor = nullptr;
    bool result = false;
  };

  static void reclaim(Context &c) noexcept
  {
    c.owner.reclaim();
    c.descriptor = &c.owner.tx_[c.owner.tx_tail_];
  }

  static bool space(Context &c) noexcept
  {
    return !c.owner.transmitted_[c.owner.tx_tail_] &&
           (c.owner.tx_tail_ + 1) % Count != c.owner.registers_.read32(0x3810);
  }

  static void prepare(Context &c) noexcept
  {
    c.descriptor->address = reinterpret_cast<unsigned long>(c.page);
    c.descriptor->length = c.length;
    c.descriptor->command = 1 | 2 | 8;
    c.descriptor->status = 0;
    c.owner.transmitted_[c.owner.tx_tail_] = c.page;
  }

  static void publish(Context &c) noexcept
  {
    detail::io_barrier();
    c.owner.tx_tail_ = (c.owner.tx_tail_ + 1) % Count;
    c.owner.registers_.write32(0x3818, c.owner.tx_tail_);
    c.result = true;
  }
  struct Reclaim;
  struct Prepare;
  struct Publish;
  using Graph = flow::graph<
      Reclaim,
      flow::types<flow::node<Reclaim, flow::action<&reclaim>,
                             flow::branch<flow::condition<&space>, flow::go<Prepare>,
                                          flow::stop>>,
                  flow::node<Prepare, flow::action<&prepare>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct E1000::InterruptFlow
{
  struct Context
  {
    E1000 &owner;
    RxDescriptor *descriptor = nullptr;
    unsigned status = 0, length = 0;
    void *page = nullptr;
  };

  static void acknowledge(Context &c) noexcept
  {
    c.owner.registers_.read32(0xc0);
  }

  static void inspect(Context &c) noexcept
  {
    c.owner.lock_.acquire();
    c.owner.reclaim();
    c.descriptor = &c.owner.rx_[c.owner.rx_next_];
    c.status = __atomic_load_n(&c.descriptor->status, __ATOMIC_ACQUIRE);
  }

  static bool done(Context &c) noexcept
  {
    return c.status & 1;
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.lock_.release();
  }

  static void replace(Context &c) noexcept
  {
    detail::io_barrier();
    c.length = c.descriptor->length;
    bool valid =
        (c.status & 2) && !c.descriptor->errors && c.length >= 14 && c.length <= 1514;
    c.page = nullptr;
    if (valid)
    {
      auto *replacement = detail::allocate_dma_page();
      if (replacement)
      {
        c.page = c.owner.received_[c.owner.rx_next_];
        c.owner.received_[c.owner.rx_next_] = replacement;
        c.descriptor->address = reinterpret_cast<unsigned long>(replacement);
      }
    }
  }

  static void publish(Context &c) noexcept
  {
    c.descriptor->status = 0;
    c.descriptor->errors = 0;
    detail::io_barrier();
    c.owner.registers_.write32(0x2818, c.owner.rx_next_);
    c.owner.rx_next_ = (c.owner.rx_next_ + 1) % Count;
    c.owner.lock_.release();
  }

  static bool deliverable(Context &c) noexcept
  {
    return c.page != nullptr;
  }

  static void deliver(Context &c) noexcept
  {
    c.owner.receiver_->receive_frame(c.page, c.length);
  }
  struct Acknowledge;
  struct Inspect;
  struct Unlock;
  struct Replace;
  struct Publish;
  struct Deliver;
  using Graph = flow::graph<
      Acknowledge,
      flow::types<
          flow::node<Acknowledge, flow::action<&acknowledge>, flow::go<Inspect>>,
          flow::node<Inspect, flow::action<&inspect>,
                     flow::branch<flow::condition<&done>, flow::go<Replace>,
                                  flow::go<Unlock>>>,
          flow::node<Unlock, flow::action<&unlock>, flow::stop>,
          flow::node<Replace, flow::action<&replace>, flow::go<Publish>>,
          flow::node<Publish, flow::action<&publish>,
                     flow::branch<flow::condition<&deliverable>, flow::go<Deliver>,
                                  flow::go<Inspect>>>,
          flow::node<Deliver, flow::action<&deliver>, flow::go<Inspect>>>>;
};

void E1000::initialize()
{
  InitializeFlow::Context context{*this};
  (void)InitializeFlow::Graph::run<0>(context);
}

void E1000::reclaim()
{
  for (unsigned i = 0; i < Count; ++i)
  {
    if (transmitted_[i] && (__atomic_load_n(&tx_[i].status, __ATOMIC_ACQUIRE) & 1))
    {
      detail::free_dma_page(transmitted_[i]);
      transmitted_[i] = nullptr;
    }
  }
}

bool E1000::transmit(void *page, unsigned length)
{
  if (!ready_ || !page || length < 14 || length > 1514)
  {
    return false;
  }
  xv::sync::ScopedLock guard(lock_);
  TransmitFlow::Context context{*this, page, length};
  (void)TransmitFlow::Graph::run<0>(context);
  return context.result;
}

void E1000::handle_interrupt()
{
  if (!ready_)
  {
    return;
  }
  InterruptFlow::Context context{*this};
  (void)InterruptFlow::Graph::run<0>(context);
}
} // namespace xv::drivers

namespace xv::drivers
{
bool E1000::promiscuous(bool enable)
{
  xv::sync::ScopedLock guard(lock_);
  if (!ready_)
  {
    return false;
  }
  unsigned control = registers_.read32(0x100);
  registers_.write32(0x100, enable ? control | 0x18U : control & ~0x18U);
  detail::io_barrier();
  return true;
}
} // namespace xv::drivers
