module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/io_status.h"

module xv.fs;
import :pipe_flow;
import :platform;
import :pipe;

namespace xv::fs
{
static_assert(sizeof(Pipe) <= 4096);

unsigned Pipe::poll(bool writable, xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(lock_);
  if (table)
  {
    table->watch(writable ? static_cast<void *>(&written_) : &read_);
  }
  if (writable)
  {
    if (!reader_open_)
    {
      return POLLERR;
    }
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
  if (n <= 0)
  {
    return n == 0 ? 0 : -1;
  }
  lock_.acquire();
  WriteFlow::Context context{*this, addr, n, nonblocking};
  (void)WriteFlow::Graph::run<0>(context);
  return context.result;
}

int Pipe::read(uint64 addr, int n, bool nonblocking)
{
  if (n <= 0)
  {
    return n == 0 ? 0 : -1;
  }
  lock_.acquire();
  ReadFlow::Context context{*this, addr, n, nonblocking};
  (void)ReadFlow::Graph::run<0>(context);
  return context.result;
}

} // namespace xv::fs
