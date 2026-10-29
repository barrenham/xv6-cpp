module;
#include "kernel/fs/io_status.h"

module xv.drivers;
import :uart_flow;

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
  tx_state_.init("uart tx state");
}

// Caller holds tx_state_. THR interrupts pump the queue after write returns.
void Uart16550::pump()
{
  bool progressed = false;
  while (tx_read_ != tx_written_ && (registers_.read8(LSR) & LSR_TX_IDLE))
  {
    registers_.write8(THR, tx_data_[tx_read_++ % TxCapacity]);
    progressed = true;
  }
  if (progressed)
  {
    detail::wake_processes(&tx_channel_);
  }
}

int Uart16550::transmit(const char *data, int count, bool nonblocking)
{
  if (count <= 0)
  {
    return count == 0 ? 0 : -1;
  }
  if (static_cast<unsigned>(count) > TxCapacity || detail::process_killed())
  {
    return -1;
  }
  TransmitFlow::Context context{*this, data, count, nonblocking};
  (void)TransmitFlow::Graph::run<0>(context);
  return context.result;
}

void Uart16550::write(const char *data, int count)
{
  int done = 0;
  while (done < count)
  {
    int part = count - done;
    if (part > 64)
    {
      part = 64;
    }
    if (transmit(data + done, part) != part)
    {
      return;
    }
    done += part;
  }
}

void Uart16550::put_sync(int character)
{
  if (detail::panicked())
  {
    for (;;)
    {
    }
  }
  if (detail::panicking())
  {
    // A panic may have interrupted a holder of tx_state_: bypass all TX locks.
    while (!(registers_.read8(LSR) & LSR_TX_IDLE))
    {
    }
    registers_.write8(THR, character);
    return;
  }
  xv::sync::ScopedLock guard(tx_state_);
  // Preserve ordering with queued user output before synchronous echo/logging.
  while (tx_read_ != tx_written_)
  {
    pump();
  }
  while (!(registers_.read8(LSR) & LSR_TX_IDLE))
  {
  }
  registers_.write8(THR, character);
}

bool Uart16550::writable(xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(tx_state_);
  if (table)
  {
    table->watch(&tx_channel_);
  }
  bool room = tx_written_ - tx_read_ <= TxCapacity - 2;
  bool unlocked = tx_lock_.available(table);
  return room && unlocked;
}

void Uart16550::drain()
{
  if (!tx_lock_.acquire_interruptible())
  {
    return;
  }
  xv::sync::ScopedLock writer(tx_lock_, xv::sync::adopt_lock);
  tx_state_.acquire();
  DrainFlow::Context context{*this};
  (void)DrainFlow::Graph::run<0>(context);
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

  {
    xv::sync::ScopedLock guard(tx_state_);
    pump();
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
