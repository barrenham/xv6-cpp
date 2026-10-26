module xv.drivers;
import :e1000;
import :pci;
import :platform;
namespace xv::drivers
{
void E1000::initialize()
{
  lock_.init("e1000");
  if (!receiver_ || !detail::configure_e1000(0x40000000UL)) return;
  registers_.write32(0xd8, ~0U);
  registers_.write32(0, registers_.read32(0) | (1U << 26));
  unsigned tries = 100000;
  while ((registers_.read32(0) & (1U << 26)) && --tries) {}
  if (!tries) return;
  registers_.write32(0xd8, ~0U);
  registers_.read32(0xc0);
  for (unsigned i = 0; i < Count; ++i)
  {
    received_[i] = detail::allocate_dma_page();
    if (!received_[i])
    {
      while (i) detail::free_dma_page(received_[--i]);
      return;
    }
    rx_[i].address = reinterpret_cast<unsigned long>(received_[i]);
    tx_[i].status = 1;
  }
  auto tx = reinterpret_cast<unsigned long>(tx_);
  auto rx = reinterpret_cast<unsigned long>(rx_);
  registers_.write32(0x3800, tx);
  registers_.write32(0x3804, tx >> 32);
  registers_.write32(0x3808, sizeof(tx_));
  registers_.write32(0x3810, 0);
  registers_.write32(0x3818, 0);
  registers_.write32(0x2800, rx);
  registers_.write32(0x2804, rx >> 32);
  registers_.write32(0x2808, sizeof(rx_));
  registers_.write32(0x2810, 0);
  registers_.write32(0x2818, Count - 1);
  registers_.write32(0x5400, 0x12005452);
  registers_.write32(0x5404, 0x80005634);
  for (unsigned i = 0; i < 128; ++i) registers_.write32(0x5200 + i * 4, 0);
  registers_.write32(0x410, 10 | (8 << 10) | (6 << 20));
  registers_.write32(0x400, 2 | 8 | (0x10 << 4) | (0x40 << 12));
  registers_.write32(0x2820, 0);
  registers_.write32(0x282c, 0);
  detail::io_barrier();
  ready_ = true;
  registers_.write32(0x100, 2 | (1 << 15) | (1 << 26));
  registers_.write32(0xd0, (1 << 7) | 1);
}

void E1000::reclaim()
{
  for (unsigned i = 0; i < Count; ++i)
    if (transmitted_[i] && (__atomic_load_n(&tx_[i].status, __ATOMIC_ACQUIRE) & 1))
    {
      detail::free_dma_page(transmitted_[i]);
      transmitted_[i] = nullptr;
    }
}

bool E1000::transmit(void *page, unsigned length)
{
  if (!ready_ || !page || length < 14 || length > 1514) return false;
  xv::sync::ScopedLock guard(lock_);
  reclaim();
  auto &descriptor = tx_[tx_tail_];
  // Keep one descriptor between tail and head: TDT == TDH means empty.
  if (transmitted_[tx_tail_] || (tx_tail_ + 1) % Count == registers_.read32(0x3810))
    return false;
  descriptor.address = reinterpret_cast<unsigned long>(page);
  descriptor.length = length;
  descriptor.command = 1 | 2 | 8;
  descriptor.status = 0;
  transmitted_[tx_tail_] = page;
  detail::io_barrier();
  tx_tail_ = (tx_tail_ + 1) % Count;
  registers_.write32(0x3818, tx_tail_);
  return true;
}

void E1000::handle_interrupt()
{
  if (!ready_) return;
  registers_.read32(0xc0);
  for (;;)
  {
    lock_.acquire();
    reclaim();
    auto &descriptor = rx_[rx_next_];
    auto status = __atomic_load_n(&descriptor.status, __ATOMIC_ACQUIRE);
    if (!(status & 1)) { lock_.release(); return; }
    detail::io_barrier();
    auto length = descriptor.length;
    bool valid = (status & 2) && !descriptor.errors && length >= 14 && length <= 1514;
    void *page = nullptr;
    if (valid)
    {
      auto *replacement = detail::allocate_dma_page();
      if (replacement)
      {
        page = received_[rx_next_];
        received_[rx_next_] = replacement;
        descriptor.address = reinterpret_cast<unsigned long>(replacement);
      }
    }
    descriptor.status = 0;
    descriptor.errors = 0;
    detail::io_barrier();
    registers_.write32(0x2818, rx_next_);
    rx_next_ = (rx_next_ + 1) % Count;
    lock_.release();
    if (page) receiver_->receive_frame(page, length);
  }
}
}
