module;
#include "kernel/support/placement_new.hpp"
#include "kernel/support/flow.h"
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

namespace flow = xv::flow;

struct CreatePtyFlow
{
  struct Context
  {
    CharacterDevice **master, **slave;
    void *page = nullptr;
    Pty *pair = nullptr;
    bool result = false;
  };

  static void allocate(Context &c) noexcept
  {
    c.page = detail::allocate_dma_page();
  }

  static bool allocated(Context &c) noexcept
  {
    return c.page != nullptr;
  }

  static void construct(Context &c) noexcept
  {
    c.pair = new (c.page) Pty;
    c.pair->initialize();
  }

  static void register_pair(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(ptys_lock);
    c.pair->next = ptys;
    ptys = c.pair;
  }

  static void publish(Context &c) noexcept
  {
    *c.master = &c.pair->master();
    *c.slave = &c.pair->slave();
    c.result = true;
  }
  struct Allocate;
  struct Construct;
  struct Register;
  struct Publish;
  using Graph = flow::graph<
      Allocate,
      flow::types<flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&allocated>,
                                          flow::go<Construct>, flow::stop>>,
                  flow::node<Construct, flow::action<&construct>, flow::go<Register>>,
                  flow::node<Register, flow::action<&register_pair>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

} // namespace

void initialize_ptys()
{
  ptys_lock.init("pty list");
}

void pty_timer_tick()
{
  xv::sync::ScopedLock guard(ptys_lock);
  for (auto *pair = ptys; pair; pair = pair->next)
  {
    pair->timer_tick();
  }
}

bool create_pty(CharacterDevice **master, CharacterDevice **slave)
{
  static_assert(sizeof(Pty) <= 4096);
  *master = *slave = nullptr;
  CreatePtyFlow::Context context{master, slave};
  (void)CreatePtyFlow::Graph::run<0>(context);
  return context.result;
}

namespace flow = xv::flow;

struct Pty::ReleaseFlow
{
  struct Context
  {
    Pty *owner;
  };

  static void unlink(Context &c) noexcept
  {
    auto **link = &ptys;
    while (*link && *link != c.owner)
    {
      link = &(*link)->next;
    }
    if (!*link)
    {
      detail::fail("pty: unregistered pair");
    }
    *link = c.owner->next;
  }

  static void unlock(Context &) noexcept
  {
    ptys_lock.release();
  }

  static void destroy(Context &c) noexcept
  {
    c.owner->~Pty();
  }

  static void free_page(Context &c) noexcept
  {
    detail::free_terminal_page(c.owner);
  }
  struct Unlink;
  struct Unlock;
  struct Destroy;
  struct Free;
  using Graph = flow::graph<
      Unlink, flow::types<flow::node<Unlink, flow::action<&unlink>, flow::go<Unlock>>,
                          flow::node<Unlock, flow::action<&unlock>, flow::go<Destroy>>,
                          flow::node<Destroy, flow::action<&destroy>, flow::go<Free>>,
                          flow::node<Free, flow::action<&free_page>, flow::stop>>>;
};

struct Pty::CloseFlow
{
  struct Context
  {
    Pty &owner;
    bool master;
    bool last_slave = false;
  };

  static void disconnect(Context &c) noexcept
  {
    ptys_lock.acquire();
    c.last_slave = !c.master && --c.owner.slave_files == 0;
    if (c.master)
    {
      c.owner.published = false;
    }
    c.owner.lock_.acquire();
    if (c.master)
    {
      c.owner.master_open_ = false;
    }
    else if (c.last_slave)
    {
      c.owner.slave_open_ = false;
    }
    detail::wake_processes(&c.owner.read_);
    detail::wake_processes(&c.owner.written_);
    c.owner.lock_.release();
    ptys_lock.release();
  }

  static bool master(Context &c) noexcept
  {
    return c.master;
  }

  static void session(Context &c) noexcept
  {
    detail::hangup_terminal(&c.owner.terminal_);
  }

  static bool hangup(Context &c) noexcept
  {
    return c.master || (!c.owner.named && c.last_slave);
  }

  static void terminal(Context &c) noexcept
  {
    c.owner.terminal_.hangup();
  }

  static void release(Context &c) noexcept
  {
    c.owner.release();
  }
  struct Disconnect;
  struct Session;
  struct HangupCheck;
  struct Terminal;
  struct Release;
  using Graph = flow::graph<
      Disconnect,
      flow::types<flow::node<Disconnect, flow::action<&disconnect>,
                             flow::branch<flow::condition<&master>, flow::go<Session>,
                                          flow::go<HangupCheck>>>,
                  flow::node<Session, flow::action<&session>, flow::go<HangupCheck>>,
                  flow::node<HangupCheck, flow::noop,
                             flow::branch<flow::condition<&hangup>, flow::go<Terminal>,
                                          flow::go<Release>>>,
                  flow::node<Terminal, flow::action<&terminal>, flow::go<Release>>,
                  flow::node<Release, flow::action<&release>, flow::stop>>>;
};

struct Pty::TransmitFlow
{
  struct Context
  {
    Pty &owner;
    const char *data;
    int count;
    bool nonblocking;
    int result = -1;
  };

  static bool space(Context &c) noexcept
  {
    return Capacity - (c.owner.written_ - c.owner.read_) >=
           static_cast<unsigned>(c.count);
  }

  static bool stopped(Context &c) noexcept
  {
    return !c.owner.master_open_ || detail::process_killed();
  }

  static bool nonblocking(Context &c) noexcept
  {
    return c.nonblocking;
  }

  static void again(Context &c) noexcept
  {
    c.result = -EAGAIN;
  }

  static void wait(Context &c) noexcept
  {
    detail::prepare_interruptible_sleep(&c.owner.read_);
    c.owner.lock_.release();
    detail::sleep_process();
    c.owner.lock_.acquire();
  }

  static void publish(Context &c) noexcept
  {
    for (int i = 0; i < c.count; ++i)
    {
      c.owner.data_[c.owner.written_++ % Capacity] = c.data[i];
    }
    detail::wake_processes(&c.owner.written_);
    c.result = c.count;
  }
  struct Space;
  struct Waiting;
  struct Nonblocking;
  struct Again;
  struct Wait;
  struct Connected;
  struct Publish;
  using Graph = flow::graph<
      Space,
      flow::types<flow::node<Space, flow::noop,
                             flow::branch<flow::condition<&space>, flow::go<Connected>,
                                          flow::go<Waiting>>>,
                  flow::node<Waiting, flow::noop,
                             flow::branch<flow::condition<&stopped>, flow::stop,
                                          flow::go<Nonblocking>>>,
                  flow::node<Nonblocking, flow::noop,
                             flow::branch<flow::condition<&nonblocking>,
                                          flow::go<Again>, flow::go<Wait>>>,
                  flow::node<Again, flow::action<&again>, flow::stop>,
                  flow::node<Wait, flow::action<&wait>, flow::go<Space>>,
                  flow::node<Connected, flow::noop,
                             flow::branch<flow::condition<&stopped>, flow::stop,
                                          flow::go<Publish>>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct Pty::DrainFlow
{
  struct Context
  {
    Pty &owner;
  };

  static bool pending(Context &c) noexcept
  {
    return c.owner.read_ != c.owner.written_ && c.owner.master_open_ &&
           !detail::process_killed();
  }

  static void wait(Context &c) noexcept
  {
    detail::prepare_interruptible_sleep(&c.owner.read_);
    c.owner.lock_.release();
    detail::sleep_process();
    c.owner.lock_.acquire();
  }
  struct Check;
  struct Wait;
  using Graph =
      flow::graph<Check,
                  flow::types<flow::node<Check, flow::noop,
                                         flow::branch<flow::condition<&pending>,
                                                      flow::go<Wait>, flow::stop>>,
                              flow::node<Wait, flow::action<&wait>,
                                         flow::branch<flow::condition<&pending>,
                                                      flow::go<Wait>, flow::stop>>>>;
};

struct Pty::ReadMasterFlow
{
  struct Context
  {
    Pty &owner;
    IoBuffer destination;
    int count;
    int result = -1;
  };

  static bool waiting(Context &c) noexcept
  {
    return c.owner.read_ == c.owner.written_ && c.owner.slave_open_;
  }

  static bool killed(Context &) noexcept
  {
    return detail::process_killed();
  }

  static bool nonblocking(Context &c) noexcept
  {
    return c.destination.nonblocking;
  }

  static void again(Context &c) noexcept
  {
    c.result = -EAGAIN;
  }

  static void wait(Context &c) noexcept
  {
    detail::prepare_interruptible_sleep(&c.owner.written_);
    c.owner.lock_.release();
    detail::sleep_process();
    c.owner.lock_.acquire();
  }

  static void drain(Context &c) noexcept
  {
    int done = 0;
    bool fault = false;
    while (done < c.count && c.owner.read_ != c.owner.written_)
    {
      char value = c.owner.data_[c.owner.read_ % Capacity];
      if (detail::copy_out(c.destination.space == AddressSpace::User,
                           c.destination.address + done, &value, 1) < 0)
      {
        fault = true;
        break;
      }
      ++c.owner.read_;
      ++done;
    }
    detail::wake_processes(&c.owner.read_);
    c.result = done ? done : (fault ? -1 : 0);
  }
  struct Check;
  struct Killed;
  struct Nonblocking;
  struct Again;
  struct Wait;
  struct Drain;
  using Graph = flow::graph<
      Check, flow::types<flow::node<Check, flow::noop,
                                    flow::branch<flow::condition<&waiting>,
                                                 flow::go<Killed>, flow::go<Drain>>>,
                         flow::node<Killed, flow::noop,
                                    flow::branch<flow::condition<&killed>, flow::stop,
                                                 flow::go<Nonblocking>>>,
                         flow::node<Nonblocking, flow::noop,
                                    flow::branch<flow::condition<&nonblocking>,
                                                 flow::go<Again>, flow::go<Wait>>>,
                         flow::node<Again, flow::action<&again>, flow::stop>,
                         flow::node<Wait, flow::action<&wait>, flow::go<Check>>,
                         flow::node<Drain, flow::action<&drain>, flow::stop>>>;
};

struct Pty::WriteMasterFlow
{
  struct Context
  {
    Pty &owner;
    IoBuffer source;
    int count;
    int done = 0, sent = 0, result = -1;
    bool connected = false, ok = false;
    unsigned char value = 0;
  };

  static bool remaining(Context &c) noexcept
  {
    return c.done < c.count;
  }

  static void connection(Context &c) noexcept
  {
    c.owner.lock_.acquire();
    c.connected = c.owner.slave_open_;
    c.owner.lock_.release();
  }

  static bool stopped(Context &c) noexcept
  {
    return !c.connected || detail::process_killed();
  }

  static void failure(Context &c) noexcept
  {
    c.result = c.done ? c.done : -1;
  }

  static void copy(Context &c) noexcept
  {
    c.ok = detail::copy_in(&c.value, c.source.space == AddressSpace::User,
                           c.source.address + c.done, 1) >= 0;
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void inject(Context &c) noexcept
  {
    c.sent = c.owner.terminal_.inject(c.value, c.source.nonblocking);
  }

  static bool accepted(Context &c) noexcept
  {
    return c.sent >= 0;
  }

  static void short_write(Context &c) noexcept
  {
    c.result = c.done ? c.done : c.sent;
  }

  static void complete(Context &c) noexcept
  {
    ++c.done;
    detail::service_terminal_signals();
  }

  static void success(Context &c) noexcept
  {
    c.result = c.done;
  }
  struct Remaining;
  struct Connection;
  struct Failure;
  struct Copy;
  struct Inject;
  struct ShortWrite;
  struct Complete;
  struct Success;
  using Graph = flow::graph<
      Remaining,
      flow::types<flow::node<Remaining, flow::noop,
                             flow::branch<flow::condition<&remaining>,
                                          flow::go<Connection>, flow::go<Success>>>,
                  flow::node<Connection, flow::action<&connection>,
                             flow::branch<flow::condition<&stopped>, flow::go<Failure>,
                                          flow::go<Copy>>>,
                  flow::node<Failure, flow::action<&failure>, flow::stop>,
                  flow::node<Copy, flow::action<&copy>,
                             flow::branch<flow::condition<&ok>, flow::go<Inject>,
                                          flow::go<Failure>>>,
                  flow::node<Inject, flow::action<&inject>,
                             flow::branch<flow::condition<&accepted>,
                                          flow::go<Complete>, flow::go<ShortWrite>>>,
                  flow::node<ShortWrite, flow::action<&short_write>, flow::stop>,
                  flow::node<Complete, flow::action<&complete>, flow::go<Remaining>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

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
  ReleaseFlow::Context context{this};
  (void)ReleaseFlow::Graph::run<0>(context);
}

void Pty::attach_slave()
{
  xv::sync::ScopedLock guard(lock_);
  slave_open_ = true;
  detail::wake_processes(&written_);
}

void Pty::close(bool master)
{
  CloseFlow::Context context{*this, master};
  (void)CloseFlow::Graph::run<0>(context);
}

int Pty::Output::transmit(const char *data, int count, bool nonblocking)
{
  if (count < 0 || static_cast<unsigned>(count) > Capacity)
  {
    return -1;
  }
  auto &p = pair_;
  p.lock_.acquire();
  TransmitFlow::Context context{p, data, count, nonblocking};
  (void)TransmitFlow::Graph::run<0>(context);
  p.lock_.release();
  return context.result;
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
  if (table)
  {
    table->watch(&pair_.read_);
  }
  // One input byte may expand to CRLF. Larger writes can still block.
  return pair_.master_open_ && pair_.written_ - pair_.read_ <= Capacity - 2;
}

void Pty::Output::drain()
{
  auto &p = pair_;
  p.lock_.acquire();
  DrainFlow::Context context{p};
  (void)DrainFlow::Graph::run<0>(context);
  p.lock_.release();
}

int Pty::read_master(IoBuffer destination, int count)
{
  if (count <= 0)
  {
    return count == 0 ? 0 : -1;
  }
  if (detail::process_killed())
  {
    return -1;
  }
  if (destination.nonblocking)
  {
    if (!reader_lock_.try_acquire())
    {
      return -EAGAIN;
    }
  }
  else if (!reader_lock_.acquire_interruptible())
  {
    return -1;
  }
  xv::sync::ScopedLock reader(reader_lock_, xv::sync::adopt_lock);
  lock_.acquire();
  ReadMasterFlow::Context context{*this, destination, count};
  (void)ReadMasterFlow::Graph::run<0>(context);
  lock_.release();
  return context.result;
}

int Pty::write_master(IoBuffer source, int count)
{
  if (count <= 0)
  {
    return count == 0 ? 0 : -1;
  }
  if (detail::process_killed())
  {
    return -1;
  }
  if (source.nonblocking)
  {
    if (!input_lock_.try_acquire())
    {
      return -EAGAIN;
    }
  }
  else if (!input_lock_.acquire_interruptible())
  {
    return -1;
  }
  xv::sync::ScopedLock input(input_lock_, xv::sync::adopt_lock);
  WriteMasterFlow::Context context{*this, source, count};
  (void)WriteMasterFlow::Graph::run<0>(context);
  return context.result;
}

struct Pty::Endpoint::IoctlFlow
{
  struct Context
  {
    Endpoint &owner;
    unsigned request;
    IoBuffer argument;
    int result = -1;
  };

  static bool number_request(Context &c) noexcept
  {
    return c.request == TIOCGPTN;
  }

  static bool lock_request(Context &c) noexcept
  {
    return c.request == TIOCSPTLCK;
  }

  static bool master_claim(Context &c) noexcept
  {
    return c.owner.master_ && c.request == TIOCSCTTY;
  }

  static bool master_readable(Context &c) noexcept
  {
    return c.owner.master_ && c.request == FIONREAD;
  }

  static void number(Context &c) noexcept
  {
    if (!c.owner.master_ || !c.owner.pair_.named)
    {
      return;
    }
    unsigned number = c.owner.pair_.number;
    c.result = detail::copy_out(c.argument.space == AddressSpace::User,
                                c.argument.address, &number, sizeof(number));
  }

  static void lock(Context &c) noexcept
  {
    if (!c.owner.master_ || !c.owner.pair_.named)
    {
      return;
    }
    int locked;
    if (detail::copy_in(&locked, c.argument.space == AddressSpace::User,
                        c.argument.address, sizeof(locked)) < 0)
    {
      return;
    }
    xv::sync::ScopedLock guard(ptys_lock);
    c.owner.pair_.slave_locked = locked != 0;
    c.result = 0;
  }

  static void readable(Context &c) noexcept
  {
    c.owner.pair_.lock_.acquire();
    int count = c.owner.pair_.written_ - c.owner.pair_.read_;
    c.owner.pair_.lock_.release();
    c.result = detail::copy_out(c.argument.space == AddressSpace::User,
                                c.argument.address, &count, sizeof(count));
  }

  static void delegate(Context &c) noexcept
  {
    c.result = c.owner.pair_.terminal_.ioctl(c.request, c.argument);
  }
  struct Entry;
  struct Number;
  struct Lock;
  struct Readable;
  struct Delegate;
  using Dispatch = flow::branch<
      flow::condition<&number_request>, flow::go<Number>,
      flow::branch<flow::condition<&lock_request>, flow::go<Lock>,
                   flow::branch<flow::condition<&master_claim>, flow::stop,
                                flow::branch<flow::condition<&master_readable>,
                                             flow::go<Readable>, flow::go<Delegate>>>>>;
  using Graph = flow::graph<
      Entry, flow::types<flow::node<Entry, flow::noop, Dispatch>,
                         flow::node<Number, flow::action<&number>, flow::stop>,
                         flow::node<Lock, flow::action<&lock>, flow::stop>,
                         flow::node<Readable, flow::action<&readable>, flow::stop>,
                         flow::node<Delegate, flow::action<&delegate>, flow::stop>>>;
};

struct Pty::Endpoint::PollFlow
{
  struct Context
  {
    Endpoint &owner;
    xv::sync::PollTable *table;
    unsigned events = 0;
    bool connected = false;
  };

  static void snapshot(Context &c) noexcept
  {
    auto &pair = c.owner.pair_;
    pair.lock_.acquire();
    if (c.table)
    {
      c.table->watch(&pair.written_);
    }
    c.connected = pair.slave_open_;
    c.events = pair.read_ != pair.written_ ? POLLIN : 0;
    pair.lock_.release();
  }

  static bool connected(Context &c) noexcept
  {
    return c.connected;
  }

  static void hangup(Context &c) noexcept
  {
    c.events |= POLLIN | POLLHUP;
  }

  static void reader(Context &c) noexcept
  {
    if ((!c.table || (c.table->events() & POLLIN)) &&
        !c.owner.pair_.reader_lock_.available(c.table))
    {
      c.events &= ~POLLIN;
    }
  }

  static void writer(Context &c) noexcept
  {
    if (!c.table || (c.table->events() & POLLOUT))
    {
      bool unlocked = c.owner.pair_.input_lock_.available(c.table);
      bool space = c.owner.pair_.terminal_.input_writable(c.table);
      if (unlocked && space)
      {
        c.events |= POLLOUT;
      }
    }
  }
  struct Snapshot;
  struct Hangup;
  struct Reader;
  struct Writer;
  using Graph = flow::graph<
      Snapshot,
      flow::types<flow::node<Snapshot, flow::action<&snapshot>,
                             flow::branch<flow::condition<&connected>, flow::go<Reader>,
                                          flow::go<Hangup>>>,
                  flow::node<Hangup, flow::action<&hangup>, flow::stop>,
                  flow::node<Reader, flow::action<&reader>, flow::go<Writer>>,
                  flow::node<Writer, flow::action<&writer>, flow::stop>>>;
};

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
  IoctlFlow::Context context{*this, request, argument};
  (void)IoctlFlow::Graph::run<0>(context);
  return context.result;
}

unsigned Pty::Endpoint::poll(xv::sync::PollTable *table)
{
  if (!master_)
  {
    return pair_.terminal_.poll(table);
  }
  PollFlow::Context context{*this, table};
  (void)PollFlow::Graph::run<0>(context);
  return context.events;
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

  const char *name() const override
  {
    return "ptmx";
  }

  void initialize() override
  {
  }

  void put_sync(int) override
  {
  }

  int read(IoBuffer, int) override
  {
    return -1;
  }

  int write(IoBuffer, int) override
  {
    return -1;
  }

  CharacterDevice *open(bool) override;

private:

  struct OpenFlow;
};

struct PtyMultiplexer::OpenFlow
{
  struct Context
  {
    void *page = nullptr;
    Pty *pair = nullptr;
    unsigned number = 0;
    CharacterDevice *result = nullptr;
  };

  static void allocate(Context &c) noexcept
  {
    c.page = detail::allocate_dma_page();
  }

  static bool allocated(Context &c) noexcept
  {
    return c.page != nullptr;
  }

  static void construct(Context &c) noexcept
  {
    c.pair = new (c.page) Pty;
    c.pair->initialize();
    c.pair->uid = detail::effective_uid();
    c.pair->gid = detail::effective_gid();
  }

  static void find(Context &c) noexcept
  {
    ptys_lock.acquire();
    for (; c.number < MaxNamedPtys; ++c.number)
    {
      auto *p = ptys;
      while (p && (!p->named || p->number != c.number))
      {
        p = p->next;
      }
      if (!p)
      {
        break;
      }
    }
  }

  static bool available(Context &c) noexcept
  {
    return c.number != MaxNamedPtys;
  }

  static void rollback(Context &c) noexcept
  {
    ptys_lock.release();
    c.pair->~Pty();
    detail::free_terminal_page(c.page);
  }

  static void publish(Context &c) noexcept
  {
    c.pair->named = true;
    c.pair->number = c.number;
    c.pair->published = true;
    c.pair->references = 1;
    c.pair->slave_files = 0;
    c.pair->next = ptys;
    ptys = c.pair;
    ptys_lock.release();
    c.result = &c.pair->master();
  }
  struct Allocate;
  struct Construct;
  struct Find;
  struct Rollback;
  struct Publish;
  using Graph = flow::graph<
      Allocate,
      flow::types<flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&allocated>,
                                          flow::go<Construct>, flow::stop>>,
                  flow::node<Construct, flow::action<&construct>, flow::go<Find>>,
                  flow::node<Find, flow::action<&find>,
                             flow::branch<flow::condition<&available>,
                                          flow::go<Publish>, flow::go<Rollback>>>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

CharacterDevice *PtyMultiplexer::open(bool)
{
  OpenFlow::Context context{};
  (void)OpenFlow::Graph::run<0>(context);
  return context.result;
}

constinit PtyMultiplexer multiplexer;
} // namespace

CharacterDevice &ptmx()
{
  return multiplexer;
}

bool retain_named_pty(unsigned number)
{
  xv::sync::ScopedLock guard(ptys_lock);
  for (auto *p = ptys; p; p = p->next)
  {
    if (p->named && p->number == number && p->published)
    {
      ++p->references;
      return true;
    }
  }
  return false;
}

bool named_pty_owner(unsigned number, unsigned &uid, unsigned &gid)
{
  xv::sync::ScopedLock guard(ptys_lock);
  for (auto *pair = ptys; pair; pair = pair->next)
  {
    if (pair->named && pair->number == number)
    {
      uid = pair->uid;
      gid = pair->gid;
      return true;
    }
  }
  return false;
}

void release_named_pty(unsigned number)
{
  ptys_lock.acquire();
  auto *pair = ptys;
  while (pair && (!pair->named || pair->number != number))
  {
    pair = pair->next;
  }
  if (!pair)
  {
    detail::fail("pty: stale node pin");
  }
  ptys_lock.release();
  pair->release();
}

namespace
{
struct OpenNamedPtyFlow
{
  struct Context
  {
    unsigned number;
    Pty *pair = nullptr;
    CharacterDevice *result = nullptr;
  };

  static void find(Context &c) noexcept
  {
    for (auto *p = ptys; p; p = p->next)
    {
      if (p->named && p->number == c.number && p->published && !p->slave_locked)
      {
        c.pair = p;
        return;
      }
    }
  }

  static bool found(Context &c) noexcept
  {
    return c.pair != nullptr;
  }

  static void retain(Context &c) noexcept
  {
    ++c.pair->references;
    ++c.pair->slave_files;
  }

  static void attach(Context &c) noexcept
  {
    c.pair->attach_slave();
  }

  static void publish(Context &c) noexcept
  {
    c.result = &c.pair->slave();
  }
  struct Find;
  struct Retain;
  struct Attach;
  struct Publish;
  using Graph = flow::graph<
      Find, flow::types<flow::node<Find, flow::action<&find>,
                                   flow::branch<flow::condition<&found>,
                                                flow::go<Retain>, flow::stop>>,
                        flow::node<Retain, flow::action<&retain>, flow::go<Attach>>,
                        flow::node<Attach, flow::action<&attach>, flow::go<Publish>>,
                        flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};
} // namespace

CharacterDevice *open_named_pty(unsigned number)
{
  xv::sync::ScopedLock guard(ptys_lock);
  OpenNamedPtyFlow::Context context{number};
  (void)OpenNamedPtyFlow::Graph::run<0>(context);
  return context.result;
}

unsigned named_pty_ids(unsigned *numbers, unsigned capacity)
{
  xv::sync::ScopedLock guard(ptys_lock);
  unsigned count = 0;
  for (unsigned i = 0; i < MaxNamedPtys && count < capacity; ++i)
  {
    for (auto *p = ptys; p; p = p->next)
    {
      if (p->named && p->number == i && p->published)
      {
        numbers[count++] = i;
        break;
      }
    }
  }
  return count;
}

} // namespace xv::drivers
