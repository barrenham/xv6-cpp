module;
#include "kernel/support/placement_new.hpp"
#include "kernel/tty/termios.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/io_status.h"

module xv.drivers;
import :pty;
import :platform;

namespace xv::drivers
{
namespace
{
constinit xv::sync::SpinLock ptys_lock;
constinit Pty *ptys = nullptr;
}

void initialize_ptys()
{
  ptys_lock.init("pty list");
}

void pty_timer_tick()
{
  xv::sync::ScopedLock guard(ptys_lock);
  for (auto *pair = ptys; pair; pair = pair->next)
    pair->timer_tick();
}

bool create_pty(CharacterDevice **master, CharacterDevice **slave)
{
  static_assert(sizeof(Pty) <= 4096);
  *master = *slave = nullptr;
  auto *page = detail::allocate_dma_page();
  if (!page) return false;
  auto *pair = new (page) Pty;
  pair->initialize();
  {
    xv::sync::ScopedLock guard(ptys_lock);
    pair->next = ptys;
    ptys = pair;
  }
  *master = &pair->master();
  *slave = &pair->slave();
  return true;
}

void Pty::initialize()
{
  lock_.init("pty output");
  input_lock_.init("pty input");
  reader_lock_.init("pty master reader");
  terminal_.initialize();
}

void Pty::release()
{
  ptys_lock.acquire();
  if (--references)
  {
    ptys_lock.release();
    return;
  }
  auto **link = &ptys;
  while (*link && *link != this) link = &(*link)->next;
  if (!*link) detail::fail("pty: unregistered pair");
  *link = next;
  ptys_lock.release();
  this->~Pty();
  detail::free_terminal_page(this);
}

void Pty::attach_slave()
{
  xv::sync::ScopedLock guard(lock_);
  slave_open_ = true;
  detail::wake_processes(&written_);
}

void Pty::close(bool master)
{
  ptys_lock.acquire();
  bool last_slave = !master && --slave_files == 0;
  if (master) published = false;
  lock_.acquire();
  if (master) master_open_ = false;
  else if (last_slave) slave_open_ = false;
  detail::wake_processes(&read_);
  detail::wake_processes(&written_);
  lock_.release();
  ptys_lock.release();
  if (master) detail::hangup_terminal(&terminal_);
  if (master || (!named && last_slave)) terminal_.hangup();
  release();
}

int Pty::Output::transmit(const char *data, int count, bool nonblocking)
{
  // Tty emits at most 64 bytes. Accept each chunk atomically so an interrupted
  // ONLCR write never replays half of a transformed newline.
  if (count < 0 || count > Capacity) return -1;
  auto &p = pair_;
  p.lock_.acquire();
  while (Capacity - (p.written_ - p.read_) < static_cast<unsigned>(count))
  {
    if (!p.master_open_ || detail::process_killed())
    {
      p.lock_.release();
      return -1;
    }
    if (nonblocking) { p.lock_.release(); return -EAGAIN; }
    detail::prepare_interruptible_sleep(&p.read_);
    p.lock_.release();
    detail::sleep_process();
    p.lock_.acquire();
  }
  if (!p.master_open_ || detail::process_killed())
  {
    p.lock_.release();
    return -1;
  }
  for (int i = 0; i < count; ++i) p.data_[p.written_++ % Capacity] = data[i];
  detail::wake_processes(&p.written_);
  p.lock_.release();
  return count;
}

void Pty::Output::put_sync(int character)
{
  auto &p = pair_;
  xv::sync::ScopedLock guard(p.lock_);
  // Echo is called with the TTY spinlock held. It must never wait for master.
  // Like hardware RX echo, echo is best effort when its output queue is full.
  if (p.master_open_ && p.written_ - p.read_ < Capacity)
  {
    p.data_[p.written_++ % Capacity] = static_cast<char>(character);
    detail::wake_processes(&p.written_);
  }
}

bool Pty::Output::writable(xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(pair_.lock_);
  if (table) table->watch(&pair_.read_);
  // One input byte may expand to CRLF. Larger writes can still block.
  return pair_.master_open_ && pair_.written_ - pair_.read_ <= Capacity - 2;
}

void Pty::Output::drain()
{
  auto &p = pair_;
  p.lock_.acquire();
  while (p.read_ != p.written_ && p.master_open_ && !detail::process_killed())
  {
    detail::prepare_interruptible_sleep(&p.read_);
    p.lock_.release();
    detail::sleep_process();
    p.lock_.acquire();
  }
  p.lock_.release();
}

int Pty::read_master(IoBuffer destination, int count)
{
  if (count <= 0) return count == 0 ? 0 : -1;
  if (detail::process_killed()) return -1;
  if (destination.nonblocking)
  {
    if (!reader_lock_.try_acquire()) return -EAGAIN;
  }
  else if (!reader_lock_.acquire_interruptible()) return -1;
  xv::sync::ScopedLock reader(reader_lock_, xv::sync::adopt_lock);
  lock_.acquire();
  while (read_ == written_ && slave_open_)
  {
    if (detail::process_killed())
    {
      lock_.release();
      return -1;
    }
    if (destination.nonblocking) { lock_.release(); return -EAGAIN; }
    detail::prepare_interruptible_sleep(&written_);
    lock_.release();
    detail::sleep_process();
    lock_.acquire();
  }
  int done = 0;
  bool fault = false;
  while (done < count && read_ != written_)
  {
    char value = data_[read_ % Capacity];
    if (detail::copy_out(destination.space == AddressSpace::User,
                         destination.address + done, &value, 1) < 0)
    {
      fault = true;
      break;
    }
    ++read_;
    ++done;
  }
  detail::wake_processes(&read_);
  lock_.release();
  return done ? done : (fault ? -1 : 0);
}

int Pty::write_master(IoBuffer source, int count)
{
  if (count <= 0) return count == 0 ? 0 : -1;
  if (detail::process_killed()) return -1;
  if (source.nonblocking)
  {
    if (!input_lock_.try_acquire()) return -EAGAIN;
  }
  else if (!input_lock_.acquire_interruptible()) return -1;
  xv::sync::ScopedLock input(input_lock_, xv::sync::adopt_lock);
  int done = 0;
  while (done < count)
  {
    lock_.acquire();
    bool connected = slave_open_;
    lock_.release();
    if (!connected || detail::process_killed()) return done ? done : -1;
    unsigned char value;
    if (detail::copy_in(&value, source.space == AddressSpace::User,
                        source.address + done, 1) < 0)
      return done ? done : -1;
    int result = terminal_.inject(value, source.nonblocking);
    if (result < 0) return done ? done : result;
    ++done;
    // Unlike UART RX, software input has no external-IRQ completion hook.
    // Wake signal recipients only after dropping the TTY/group spinlocks.
    detail::service_terminal_signals();
  }
  return done;
}

int Pty::Endpoint::read(IoBuffer destination, int count)
{
  return master_ ? pair_.read_master(destination, count)
                 : pair_.terminal_.read(destination, count);
}

int Pty::Endpoint::write(IoBuffer source, int count)
{
  return master_ ? pair_.write_master(source, count)
                 : pair_.terminal_.write(source, count);
}

int Pty::Endpoint::ioctl(unsigned request, IoBuffer argument)
{
  if (request == TIOCGPTN)
  {
    if (!master_ || !pair_.named) return -1;
    unsigned number = pair_.number;
    return detail::copy_out(argument.space == AddressSpace::User, argument.address, &number, sizeof(number));
  }
  if (request == TIOCSPTLCK)
  {
    if (!master_ || !pair_.named) return -1;
    int locked;
    if (detail::copy_in(&locked, argument.space == AddressSpace::User, argument.address, sizeof(locked)) < 0) return -1;
    xv::sync::ScopedLock guard(ptys_lock);
    pair_.slave_locked = locked != 0;
    return 0;
  }

  if (master_ && request == TIOCSCTTY) return -1;
  if (master_ && request == FIONREAD)
  {
    pair_.lock_.acquire();
    int count = pair_.written_ - pair_.read_;
    pair_.lock_.release();
    return detail::copy_out(argument.space == AddressSpace::User,
                            argument.address, &count, sizeof(count));
  }
  return pair_.terminal_.ioctl(request, argument);
}

unsigned Pty::Endpoint::poll(xv::sync::PollTable *table)
{
  if (!master_) return pair_.terminal_.poll(table);
  pair_.lock_.acquire();
  if (table) table->watch(&pair_.written_);
  bool connected = pair_.slave_open_;
  unsigned events = pair_.read_ != pair_.written_ ? POLLIN : 0;
  pair_.lock_.release();
  if (!connected) return events | POLLIN | POLLHUP;
  if ((!table || (table->events() & POLLIN)) && !pair_.reader_lock_.available(table))
    events &= ~POLLIN;
  // Never acquire the TTY lock while holding the output lock (echo goes back).
  if (!table || (table->events() & POLLOUT))
  {
    bool unlocked = pair_.input_lock_.available(table);
    bool space = pair_.terminal_.input_writable(table);
    if (unlocked && space) events |= POLLOUT;
  }
  return events;
}

void Pty::Endpoint::close()
{
  pair_.close(master_);
}
namespace
{
class PtyMultiplexer final : public CharacterDevice
{
public:
  const char *name() const override { return "ptmx"; }
  void initialize() override {}
  void put_sync(int) override {}
  int read(IoBuffer, int) override { return -1; }
  int write(IoBuffer, int) override { return -1; }
  CharacterDevice *open(bool) override
  {
    auto *page = detail::allocate_dma_page();
    if (!page) return nullptr;
    auto *pair = new (page) Pty;
    pair->initialize();
    ptys_lock.acquire();
    unsigned number;
    for (number = 0; number < MaxNamedPtys; ++number)
    {
      auto *p = ptys;
      while (p && (!p->named || p->number != number)) p = p->next;
      if (!p) break;
    }
    if (number == MaxNamedPtys)
    {
      ptys_lock.release();
      pair->~Pty(); detail::free_terminal_page(page);
      return nullptr;
    }
    pair->named = true;
    pair->number = number;
    pair->published = true;
    pair->references = 1;
    pair->slave_files = 0;
    pair->next = ptys;
    ptys = pair;
    ptys_lock.release();
    return &pair->master();
  }
};
constinit PtyMultiplexer multiplexer;
}
CharacterDevice &ptmx() { return multiplexer; }
bool retain_named_pty(unsigned number)
{
  xv::sync::ScopedLock guard(ptys_lock);
  for (auto *p = ptys; p; p = p->next)
    if (p->named && p->number == number && p->published) { ++p->references; return true; }
  return false;
}
void release_named_pty(unsigned number)
{
  ptys_lock.acquire();
  auto *pair = ptys;
  while (pair && (!pair->named || pair->number != number)) pair = pair->next;
  if (!pair) detail::fail("pty: stale node pin");
  ptys_lock.release();
  pair->release();
}
CharacterDevice *open_named_pty(unsigned number)
{
  xv::sync::ScopedLock guard(ptys_lock);
  for (auto *p = ptys; p; p = p->next)
    if (p->named && p->number == number && p->published && !p->slave_locked)
    {
      ++p->references;
      ++p->slave_files;
      p->attach_slave();
      return &p->slave();
    }
  return nullptr;
}
unsigned named_pty_ids(unsigned *numbers, unsigned capacity)
{
  xv::sync::ScopedLock guard(ptys_lock);
  unsigned count = 0;
  for (unsigned i = 0; i < MaxNamedPtys && count < capacity; ++i)
    for (auto *p = ptys; p; p = p->next)
      if (p->named && p->number == i && p->published) { numbers[count++] = i; break; }
  return count;
}

}
