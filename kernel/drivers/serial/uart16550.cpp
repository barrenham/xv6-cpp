module xv.drivers;

import :uart;
import :platform;

namespace xv::drivers
{
// the UART control registers.
// some have different meanings for read vs write.
// see http://byterunner.com/16550.html
constexpr unsigned RHR = 0; // receive holding register (for input bytes)
constexpr unsigned THR = 0; // transmit holding register (for output bytes)
constexpr unsigned IER = 1; // interrupt enable register
constexpr unsigned IER_RX_ENABLE = (1 << 0); // receiver interrupts
constexpr unsigned IER_TX_ENABLE = (1 << 1); // transmit interrupts
constexpr unsigned FCR = 2;                  // FIFO control register
constexpr unsigned FCR_FIFO_ENABLE = (1 << 0);
constexpr unsigned FCR_FIFO_CLEAR = (3 << 1); // clear the content of the two FIFOs
constexpr unsigned ISR = 2;                   // interrupt status register
constexpr unsigned LCR = 3;                   // line control register
constexpr unsigned LCR_EIGHT_BITS = (3 << 0);
constexpr unsigned LCR_BAUD_LATCH = (1 << 7); // special mode to set baud rate
constexpr unsigned LSR = 5;                   // line status register
constexpr unsigned LSR_RX_READY = (1 << 0);   // input is waiting to be read from RHR
constexpr unsigned LSR_TX_IDLE = (1 << 5); // THR can accept another character to send

void Uart16550::attach(InputReceiver &receiver)
{
  receiver_ = &receiver;
}

void Uart16550::initialize()
{
  // disable interrupts.
  registers_.write8(IER, 0x00);

  // special mode to set baud rate.
  registers_.write8(LCR, LCR_BAUD_LATCH);

  // LSB for baud rate of 38.4K.
  registers_.write8(0, 0x03);

  // MSB for baud rate of 38.4K.
  registers_.write8(1, 0x00);

  // leave set-baud mode,
  // and set word length to 8 bits, no parity.
  registers_.write8(LCR, LCR_EIGHT_BITS);

  // reset and enable FIFOs.
  registers_.write8(FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);

  // enable transmit and receive interrupts.
  registers_.write8(IER, IER_TX_ENABLE | IER_RX_ENABLE);

  tx_lock_.init("uart");
}

// transmit buf[] to the uart. it blocks if the
// uart is busy, so it cannot be called from
// interrupts, only from write() system calls.
void Uart16550::write(const char *buf, int n)
{
  xv::sync::ScopedLock guard(tx_lock_);

  int i = 0;
  while (i < n)
  {
    detail::prepare_sleep(&tx_channel_);
    if (registers_.read8(LSR) & LSR_TX_IDLE)
    {
      registers_.write8(THR, buf[i]);
      i += 1;
    }
    else
    {
      detail::sleep_process();
    }
  }
}

// write a byte to the uart without using
// interrupts, for use by kernel xv::print::printk() and
// to echo characters. it spins waiting for the uart's
// output register to be empty.
void Uart16550::put_sync(int c)
{
  if (!detail::panicking())
  {
    detail::push_interrupts_off();
  }

  if (detail::panicked())
  {
    for (;;)
    {
    }
  }

  // wait for UART to set Transmit Holding Empty in LSR.
  while ((registers_.read8(LSR) & LSR_TX_IDLE) == 0)
  {
  }
  registers_.write8(THR, c);

  if (!detail::panicking())
  {
    detail::pop_interrupts_off();
  }
}

// try to read one input character from the UART.
// return -1 if none is waiting.
int Uart16550::get()
{
  // is input ready?
  if (registers_.read8(LSR) & LSR_RX_READY)
  {
    return registers_.read8(RHR);
  }
  else
  {
    return -1;
  }
}

// handle a uart interrupt, raised because input has
// arrived, or the uart is ready for more output, or
// both. called by the external interrupt router.
void Uart16550::handle_interrupt()
{
  registers_.read8(ISR); // acknowledge the interrupt

  if (registers_.read8(LSR) & LSR_TX_IDLE)
  {
    // UART finished transmitting; wake up sending thread.
    detail::wake_processes(&tx_channel_);
  }

  // read and process incoming characters, if any.
  while (1)
  {
    int c = get();
    if (c == -1)
    {
      break;
    }
    if (receiver_)
    {
      receiver_->receive(c);
    }
  }
}

} // namespace xv::drivers
