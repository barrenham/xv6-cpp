module xv.drivers;

import :platform;

namespace xv::drivers
{
void InterruptRouter::initialize()
{
  for (unsigned i = 0; i < count_; ++i)
  {
    if (!devices_[i] || devices_[i]->irq() == 0)
    {
      detail::fail("interrupt router: invalid source");
    }
    for (unsigned j = 0; j < i; ++j)
    {
      if (devices_[i]->irq() == devices_[j]->irq())
      {
        detail::fail("interrupt router: duplicate IRQ");
      }
    }
  }
  controller_.initialize();
  for (unsigned i = 0; i < count_; ++i)
  {
    controller_.set_priority(devices_[i]->irq(), 1);
  }
}

void InterruptRouter::initialize_hart(int hart)
{
  controller_.initialize_hart(hart);
  for (unsigned i = 0; i < count_; ++i)
  {
    controller_.enable(hart, devices_[i]->irq());
  }
}

unsigned InterruptRouter::dispatch(int hart)
{
  const auto irq = controller_.claim(hart);
  if (irq == 0)
  {
    return 0;
  }
  unsigned unknown = irq;
  for (unsigned i = 0; i < count_; ++i)
  {
    if (devices_[i]->irq() == irq)
    {
      devices_[i]->handle_interrupt();
      unknown = 0;
      break;
    }
  }
  controller_.complete(hart, irq);
  return unknown;
}
} // namespace xv::drivers
