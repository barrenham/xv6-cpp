module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/io_status.h"

module xv.fs;
import :platform;
import :pipe;

namespace xv::fs
{
static_assert(sizeof(Pipe) <= 4096);

unsigned Pipe::poll(bool writable, xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(lock_);
  if (table) table->watch(writable ? static_cast<void *>(&written_) : &read_);
  if (writable)
  {
    if (!reader_open_) return POLLERR;
    return written_ - read_ < Capacity ? POLLOUT : 0;
  }
  unsigned events = written_ != read_ || !writer_open_ ? POLLIN : 0;
  return events | (writer_open_ ? 0 : POLLHUP);
}

void Pipe::initialize()
{
  lock_.init("pipe");
}

void Pipe::close(bool writable)
{
  lock_.acquire();
  if (writable)
  {
    writer_open_ = 0;
    detail::wakeup(&read_);
  }
  else
  {
    reader_open_ = 0;
    detail::wakeup(&written_);
  }
  if (reader_open_ == 0 && writer_open_ == 0)
  {
    lock_.release();
    this->~Pipe();
    detail::free_page(this);
  }
  else
  {
    lock_.release();
  }
}

int Pipe::write(uint64 addr, int n, bool nonblocking)
{
  if (n <= 0) return n == 0 ? 0 : -1;
  int i = 0;

  lock_.acquire();
  // Writes no larger than the ring are all-or-nothing with respect to space.
  while (static_cast<unsigned>(n) <= Capacity && Capacity - (written_ - read_) < static_cast<unsigned>(n))
  {
    if (!reader_open_ || detail::killed()) { lock_.release(); return -1; }
    if (nonblocking) { lock_.release(); return -EAGAIN; }
    detail::prepare_interruptible_sleep(&written_);
    lock_.release();
    detail::sleep();
    lock_.acquire();
  }
  while (i < n)
  {
    if (detail::killed())
    {
      lock_.release();
      return i ? i : -1;
    }
    if (reader_open_ == 0)
    {
      lock_.release();
      return i ? i : -1;
    }
    if (written_ == read_ + Capacity)
    { //DOC: pipewrite-full
      detail::wakeup(&read_);
      if (nonblocking) { lock_.release(); return i ? i : -EAGAIN; }
      detail::prepare_interruptible_sleep(&written_);
      lock_.release();
      detail::sleep();
      lock_.acquire();
    }
    else
    {
      char ch;
      if (detail::copy_in(&ch, true, addr + i, 1) == -1)
      {
        if (i == 0)
        {
          i = -1;
        }
        break;
      }
      data_[written_++ % Capacity] = ch;
      i++;
    }
  }
  detail::wakeup(&read_);
  lock_.release();

  return i;
}

int Pipe::read(uint64 addr, int n, bool nonblocking)
{
  if (n <= 0) return n == 0 ? 0 : -1;
  int i;
  char ch;

  lock_.acquire();
  while (read_ == written_ && writer_open_)
  { //DOC: pipe-empty
    if (detail::killed())
    {
      lock_.release();
      return -1;
    }
    if (nonblocking) { lock_.release(); return -EAGAIN; }
    detail::prepare_interruptible_sleep(&read_); //DOC: piperead-sleep
    lock_.release();
    detail::sleep();
    lock_.acquire();
  }
  for (i = 0; i < n; i++)
  { //DOC: piperead-copy
    if (read_ == written_)
    {
      break;
    }
    ch = data_[read_ % Capacity];
    if (detail::copy_out(true, addr + i, &ch, 1) == -1)
    {
      if (i == 0)
      {
        i = -1;
      }
      break;
    }
    read_++;
  }
  detail::wakeup(&written_); //DOC: piperead-wakeup
  lock_.release();
  return i;
}

} // namespace xv::fs
