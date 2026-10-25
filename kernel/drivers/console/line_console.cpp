module xv.drivers;

import :console;
import :platform;

namespace xv::drivers
{
constexpr int Backspace = 0x100;

constexpr int control(int character)
{
  return character - '@';
}

void LineConsole::initialize()
{
  lock_.init("console");
}

void LineConsole::put_sync(int c)
{
  if (c == Backspace)
  {
    // if the user typed backspace, overwrite with a space.
    serial_.put_sync('\b');
    serial_.put_sync(' ');
    serial_.put_sync('\b');
  }
  else
  {
    serial_.put_sync(c);
  }
}

//
// user write() system calls to the console go here.
// uses sleep() and UART interrupts.
//
int LineConsole::write(IoBuffer source, int n)
{
  char buf[32]; // move batches from user space to uart.
  int i = 0;

  while (i < n)
  {
    int nn = sizeof(buf);
    if (nn > n - i)
    {
      nn = n - i;
    }
    if (detail::copy_in(buf, source.space == AddressSpace::User, source.address + i,
                        nn) == -1)
    {
      break;
    }
    serial_.write(buf, nn);
    i += nn;
  }

  return i;
}

//
// user read()s from the console go here.
// copy (up to) a whole input line to dst.
// user_dst indicates whether dst is a user
// or kernel address.
//
int LineConsole::read(IoBuffer destination, int n)
{
  int target;
  int c;
  char cbuf;

  if (n <= 0)
  {
    return 0;
  }
  target = n;
  lock_.acquire();
  while (n > 0)
  {
    // wait until interrupt handler has put some
    // input into input buffer.
    while (read_ == written_)
    {
      if (detail::process_killed())
      {
        lock_.release();
        return -1;
      }
      detail::prepare_sleep(&read_);
      lock_.release();
      detail::sleep_process();
      lock_.acquire();
    }

    c = buffer_[read_++ % InputSize];

    if (c == control('D'))
    { // end-of-file
      if (n < target)
      {
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        read_--;
      }
      break;
    }

    // copy the input byte to the user-space buffer.
    cbuf = c;
    if (detail::copy_out(destination.space == AddressSpace::User, destination.address,
                         &cbuf, 1) == -1)
    {
      break;
    }

    destination.address++;
    --n;

    if (c == '\n')
    {
      // a whole line has arrived, return to
      // the user-level read().
      break;
    }
  }
  lock_.release();

  return target - n;
}

//
// the console input interrupt handler.
// The attached serial port delivers each received character here.
// do erase/kill processing, append to buffer_,
// wake up read() if a whole line has arrived.
//
void LineConsole::receive(int c)
{
  xv::sync::ScopedLock guard(lock_);

  switch (c)
  {
  case control('P'): // Print process list.
    detail::dump_processes();
    break;
  case control('U'): // Kill line.
    while (edited_ != written_ && buffer_[(edited_ - 1) % InputSize] != '\n')
    {
      edited_--;
      put_sync(Backspace);
    }
    break;
  case control('H'): // Backspace
  case '\x7f':       // Delete key
    if (edited_ != written_)
    {
      edited_--;
      put_sync(Backspace);
    }
    break;
  default:
    if (c != 0 && edited_ - read_ < InputSize)
    {
      c = (c == '\r') ? '\n' : c;

      // echo back to the user.
      put_sync(c);

      // store for consumption by read().
      buffer_[edited_++ % InputSize] = c;

      if (c == '\n' || c == control('D') || edited_ - read_ == InputSize)
      {
        // wake up read() if a whole line (or end-of-file)
        // has arrived.
        written_ = edited_;
        detail::wake_processes(&read_);
      }
    }
    break;
  }
}

} // namespace xv::drivers
