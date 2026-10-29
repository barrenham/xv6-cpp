module;
#include "kernel/tty/termios.h"
#include "kernel/support/flow.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/io_status.h"

module xv.drivers;
import :tty;
import :platform;

namespace xv::drivers
{
namespace
{
constexpr unsigned FixedControl = B38400 | CS8 | CREAD | CLOCAL;
static_assert(sizeof(termios) == 36 && sizeof(winsize) == 8);

bool valid(const termios &value)
{
  return !(value.c_iflag & ~(IGNCR | ICRNL | INLCR | ISTRIP)) &&
         !(value.c_oflag & ~(OPOST | ONLCR)) &&
         !(value.c_lflag & ~(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOCTL |
                             ECHOKE | IEXTEN)) &&
         value.c_cflag == FixedControl && value.c_line == 0;
}
} // namespace

namespace flow = xv::flow;

struct Tty::InjectFlow
{
  struct Context
  {
    Tty &owner;
    unsigned char character;
    bool nonblocking;
    InputAction action = InputAction::None;
    int result = -1;
  };

  static bool stopped(Context &c) noexcept
  {
    return c.owner.hungup_ || detail::process_killed();
  }

  static void receive(Context &c) noexcept
  {
    c.action =
        c.owner.discipline_.receive(c.character, c.owner.settings_, c.owner.output_);
  }

  static bool retry(Context &c) noexcept
  {
    return c.action == InputAction::Retry;
  }

  static void accepted(Context &c) noexcept
  {
    if (c.action == InputAction::Interrupt || c.action == InputAction::Suspend)
    {
      detail::post_terminal_interrupt(&c.owner, c.action == InputAction::Suspend);
    }
    detail::wake_processes(&c.owner.discipline_);
    c.result = 0;
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
    detail::prepare_interruptible_sleep(&c.owner.discipline_);
    c.owner.lock_.release();
    detail::sleep_process();
    c.owner.lock_.acquire();
  }
  struct Check;
  struct Receive;
  struct Accepted;
  struct Nonblocking;
  struct Again;
  struct Wait;
  using Graph = flow::graph<
      Check,
      flow::types<flow::node<Check, flow::noop,
                             flow::branch<flow::condition<&stopped>, flow::stop,
                                          flow::go<Receive>>>,
                  flow::node<Receive, flow::action<&receive>,
                             flow::branch<flow::condition<&retry>,
                                          flow::go<Nonblocking>, flow::go<Accepted>>>,
                  flow::node<Accepted, flow::action<&accepted>, flow::stop>,
                  flow::node<Nonblocking, flow::noop,
                             flow::branch<flow::condition<&nonblocking>,
                                          flow::go<Again>, flow::go<Wait>>>,
                  flow::node<Again, flow::action<&again>, flow::stop>,
                  flow::node<Wait, flow::action<&wait>, flow::go<Check>>>>;
};

struct Tty::PollFlow
{
  struct Context
  {
    Tty &owner;
    xv::sync::PollTable *table;
    unsigned events = 0;
    bool disconnected = false;
  };

  static void snapshot(Context &c) noexcept
  {
    c.owner.lock_.acquire();
    if (c.table)
    {
      c.table->watch(&c.owner.discipline_);
    }
    unsigned minimum = 1;
    if (!(c.owner.settings_.c_lflag & ICANON) && !c.owner.settings_.c_cc[VTIME] &&
        c.owner.settings_.c_cc[VMIN])
    {
      minimum = c.owner.settings_.c_cc[VMIN];
    }
    c.events = c.owner.discipline_.available() >= minimum ? POLLIN : 0;
    c.disconnected = c.owner.hungup_;
    c.owner.lock_.release();
  }

  static bool disconnected(Context &c) noexcept
  {
    return c.disconnected;
  }

  static void hangup(Context &c) noexcept
  {
    c.events |= POLLIN | POLLHUP;
  }

  static void reader(Context &c) noexcept
  {
    if ((!c.table || (c.table->events() & POLLIN)) &&
        !c.owner.read_lock_.available(c.table))
    {
      c.events &= ~POLLIN;
    }
  }

  static void writer(Context &c) noexcept
  {
    if (!c.table || (c.table->events() & POLLOUT))
    {
      bool unlocked = c.owner.write_lock_.available(c.table);
      bool space = c.owner.output_.writable(c.table);
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
                             flow::branch<flow::condition<&disconnected>,
                                          flow::go<Hangup>, flow::go<Reader>>>,
                  flow::node<Hangup, flow::action<&hangup>, flow::stop>,
                  flow::node<Reader, flow::action<&reader>, flow::go<Writer>>,
                  flow::node<Writer, flow::action<&writer>, flow::stop>>>;
};

struct Tty::ReadFlow
{
  struct Context
  {
    Tty &owner;
    IoBuffer destination;
    int count;
    unsigned generation;
    unsigned long start;
    int done = 0, minimum = 0;
    unsigned timeout = 0;
    bool finished = false, fault = false, again = false, canonical = false;
  };

  static bool permitted(Context &c) noexcept
  {
    return detail::may_read_terminal(&c.owner);
  }

  static void fault(Context &c) noexcept
  {
    c.fault = true;
  }

  static void generation(Context &c) noexcept
  {
    if (c.generation != c.owner.generation_)
    {
      if (c.done)
      {
        c.finished = true;
      }
      else
      {
        c.generation = c.owner.generation_;
        c.start = detail::terminal_milliseconds();
      }
    }
  }

  static bool finished(Context &c) noexcept
  {
    return c.finished;
  }

  static void mode(Context &c) noexcept
  {
    c.canonical = c.owner.settings_.c_lflag & ICANON;
    c.timeout = c.canonical ? 0 : c.owner.settings_.c_cc[VTIME] * 100U;
    c.minimum = c.canonical ? 1 : c.owner.settings_.c_cc[VMIN];
    if (c.minimum > c.count)
    {
      c.minimum = c.count;
    }
  }

  static void drain(Context &c) noexcept
  {
    while (c.owner.discipline_.available() && c.done < c.count)
    {
      auto byte = c.owner.discipline_.front();
      if (byte.eof)
      {
        c.owner.discipline_.consume();
        if (c.canonical)
        {
          c.finished = true;
          break;
        }
        continue;
      }
      char value = byte.value;
      if (detail::copy_out(c.destination.space == AddressSpace::User,
                           c.destination.address + c.done, &value, 1) < 0)
      {
        c.fault = c.finished = true;
        break;
      }
      c.owner.discipline_.consume();
      ++c.done;
      c.start = detail::terminal_milliseconds();
      if (c.canonical && byte.end)
      {
        c.finished = true;
        break;
      }
    }
    if (c.canonical && !c.finished && c.done && c.owner.discipline_.available() &&
        c.owner.discipline_.front().eof)
    {
      c.owner.discipline_.consume();
      c.finished = true;
    }
  }

  static bool complete(Context &c) noexcept
  {
    return c.finished || c.done == c.count || c.owner.hungup_;
  }

  static bool nonblocking(Context &c) noexcept
  {
    return c.destination.nonblocking;
  }

  static void again(Context &c) noexcept
  {
    c.again = c.done == 0;
  }

  static bool raw_complete(Context &c) noexcept
  {
    if (c.canonical)
    {
      return false;
    }
    if (c.done >= c.minimum && (c.minimum || !c.timeout || c.done))
    {
      return true;
    }
    return c.timeout && (!c.minimum || c.done) &&
           detail::terminal_milliseconds() - c.start >= c.timeout;
  }

  static bool killed(Context &) noexcept
  {
    return detail::process_killed();
  }

  static void wait(Context &c) noexcept
  {
    detail::wake_processes(&c.owner.discipline_);
    c.owner.timed_wait_ = c.timeout && (!c.minimum || c.done);
    detail::prepare_interruptible_sleep(&c.owner.discipline_);
    c.owner.lock_.release();
    detail::sleep_process();
    c.owner.lock_.acquire();
    c.owner.timed_wait_ = false;
  }
  struct Access;
  struct Fault;
  struct Generation;
  struct Mode;
  struct Drain;
  struct Nonblocking;
  struct Again;
  struct Raw;
  struct Killed;
  struct Wait;
  using Graph = flow::graph<
      Access,
      flow::types<flow::node<Access, flow::noop,
                             flow::branch<flow::condition<&permitted>,
                                          flow::go<Generation>, flow::go<Fault>>>,
                  flow::node<Fault, flow::action<&fault>, flow::stop>,
                  flow::node<Generation, flow::action<&generation>,
                             flow::branch<flow::condition<&finished>, flow::stop,
                                          flow::go<Mode>>>,
                  flow::node<Mode, flow::action<&mode>, flow::go<Drain>>,
                  flow::node<Drain, flow::action<&drain>,
                             flow::branch<flow::condition<&complete>, flow::stop,
                                          flow::go<Nonblocking>>>,
                  flow::node<Nonblocking, flow::noop,
                             flow::branch<flow::condition<&nonblocking>,
                                          flow::go<Again>, flow::go<Raw>>>,
                  flow::node<Again, flow::action<&again>, flow::stop>,
                  flow::node<Raw, flow::noop,
                             flow::branch<flow::condition<&raw_complete>, flow::stop,
                                          flow::go<Killed>>>,
                  flow::node<Killed, flow::noop,
                             flow::branch<flow::condition<&killed>, flow::go<Fault>,
                                          flow::go<Wait>>>,
                  flow::node<Wait, flow::action<&wait>, flow::go<Access>>>>;
};

struct Tty::WriteFlow
{
  struct Context
  {
    Tty &owner;
    IoBuffer source;
    int count;
    unsigned flags;
    char (&input)[32], (&output)[64];
    int done = 0, part = 0, used = 0, sent = 0, result = -1;
    bool ok = false;
  };

  static bool remaining(Context &c) noexcept
  {
    return c.done < c.count;
  }

  static bool killed(Context &) noexcept
  {
    return detail::process_killed();
  }

  static void failure(Context &c) noexcept
  {
    c.result = c.done ? c.done : -1;
  }

  static void read(Context &c) noexcept
  {
    c.part = c.count - c.done;
    if (c.part > 32)
    {
      c.part = 32;
    }
    if (c.source.nonblocking)
    {
      c.part = 1;
    }
    c.ok = detail::copy_in(c.input, c.source.space == AddressSpace::User,
                           c.source.address + c.done, c.part) >= 0;
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void transform(Context &c) noexcept
  {
    c.used = 0;
    for (int i = 0; i < c.part; ++i)
    {
      if (c.input[i] == '\n' && (c.flags & (OPOST | ONLCR)) == (OPOST | ONLCR))
      {
        c.output[c.used++] = '\r';
      }
      c.output[c.used++] = c.input[i];
    }
  }

  static void transmit(Context &c) noexcept
  {
    c.sent = c.owner.output_.transmit(c.output, c.used, c.source.nonblocking);
  }

  static bool sent(Context &c) noexcept
  {
    return c.sent == c.used;
  }

  static void short_write(Context &c) noexcept
  {
    c.result = c.done ? c.done : c.sent;
  }

  static void advance(Context &c) noexcept
  {
    c.done += c.part;
  }

  static void success(Context &c) noexcept
  {
    c.result = c.done;
  }
  struct Remaining;
  struct Killed;
  struct Failure;
  struct Read;
  struct Transform;
  struct Transmit;
  struct ShortWrite;
  struct Advance;
  struct Success;
  using Graph = flow::graph<
      Remaining,
      flow::types<flow::node<Remaining, flow::noop,
                             flow::branch<flow::condition<&remaining>, flow::go<Killed>,
                                          flow::go<Success>>>,
                  flow::node<Killed, flow::noop,
                             flow::branch<flow::condition<&killed>, flow::go<Failure>,
                                          flow::go<Read>>>,
                  flow::node<Failure, flow::action<&failure>, flow::stop>,
                  flow::node<Read, flow::action<&read>,
                             flow::branch<flow::condition<&ok>, flow::go<Transform>,
                                          flow::go<Failure>>>,
                  flow::node<Transform, flow::action<&transform>, flow::go<Transmit>>,
                  flow::node<Transmit, flow::action<&transmit>,
                             flow::branch<flow::condition<&sent>, flow::go<Advance>,
                                          flow::go<ShortWrite>>>,
                  flow::node<ShortWrite, flow::action<&short_write>, flow::stop>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Remaining>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct Tty::IoctlFlow
{
  struct Context
  {
    Tty &owner;
    unsigned request;
    IoBuffer argument;
    bool user;
    int result = -1;
    termios value{};
    bool ok = false;
  };

  static bool claim_request(Context &c) noexcept
  {
    return c.request == TIOCSCTTY;
  }

  static bool group_request(Context &c) noexcept
  {
    return c.request == TIOCGPGRP;
  }

  static bool set_group_request(Context &c) noexcept
  {
    return c.request == TIOCSPGRP;
  }

  static bool settings_request(Context &c) noexcept
  {
    return c.request == TCGETS;
  }

  static bool set_settings_request(Context &c) noexcept
  {
    return c.request == TCSETS || c.request == TCSETSW || c.request == TCSETSF;
  }

  static bool window_request(Context &c) noexcept
  {
    return c.request == TIOCGWINSZ;
  }

  static bool set_window_request(Context &c) noexcept
  {
    return c.request == TIOCSWINSZ;
  }

  static bool readable_request(Context &c) noexcept
  {
    return c.request == FIONREAD;
  }

  static bool flush_request(Context &c) noexcept
  {
    return c.request == TCFLSH && c.argument.address == TCIFLUSH;
  }

  static void claim(Context &c) noexcept
  {
    if (c.argument.address != 0)
    {
      return;
    }
    xv::sync::ScopedLock guard(c.owner.lock_);
    c.result = c.owner.hungup_ ? -1 : detail::claim_terminal(&c.owner);
  }

  static void group(Context &c) noexcept
  {
    int group = detail::terminal_foreground(&c.owner);
    if (group >= 0)
    {
      c.result = detail::copy_out(c.user, c.argument.address, &group, sizeof(group));
    }
  }

  static void set_group(Context &c) noexcept
  {
    int group;
    if (detail::copy_in(&group, c.user, c.argument.address, sizeof(group)) < 0)
    {
      return;
    }
    xv::sync::ScopedLock guard(c.owner.lock_);
    c.result = detail::set_terminal_foreground(&c.owner, group);
    if (c.result == 0)
    {
      detail::wake_processes(&c.owner.discipline_);
    }
  }

  static void settings(Context &c) noexcept
  {
    c.owner.lock_.acquire();
    auto value = c.owner.settings_;
    c.owner.lock_.release();
    c.result = detail::copy_out(c.user, c.argument.address, &value, sizeof(value));
  }

  static void read_settings(Context &c) noexcept
  {
    c.ok =
        detail::copy_in(&c.value, c.user, c.argument.address, sizeof(c.value)) >= 0 &&
        valid(c.value);
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static bool draining(Context &c) noexcept
  {
    return c.request != TCSETS;
  }

  static void writer(Context &c) noexcept
  {
    c.ok = c.owner.write_lock_.acquire_interruptible();
  }

  static void drain(Context &c) noexcept
  {
    c.owner.output_.drain();
  }

  static void apply(Context &c) noexcept
  {
    c.owner.lock_.acquire();
    if (c.request == TCSETSF)
    {
      c.owner.discipline_.flush();
    }
    if ((c.value.c_lflag ^ c.owner.settings_.c_lflag) & ICANON)
    {
      c.owner.discipline_.change_mode(c.value.c_lflag & ICANON);
    }
    c.owner.settings_ = c.value;
    ++c.owner.generation_;
    detail::wake_processes(&c.owner.discipline_);
    c.owner.lock_.release();
  }

  static void release_writer(Context &c) noexcept
  {
    c.owner.write_lock_.release();
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }

  static void window(Context &c) noexcept
  {
    c.owner.lock_.acquire();
    auto value = c.owner.size_;
    c.owner.lock_.release();
    c.result = detail::copy_out(c.user, c.argument.address, &value, sizeof(value));
  }

  static void set_window(Context &c) noexcept
  {
    winsize value{};
    if (detail::copy_in(&value, c.user, c.argument.address, sizeof(value)) < 0)
    {
      return;
    }
    xv::sync::ScopedLock guard(c.owner.lock_);
    c.owner.size_ = value;
    c.result = 0;
  }

  static void readable(Context &c) noexcept
  {
    c.owner.lock_.acquire();
    int count = c.owner.discipline_.readable_bytes();
    c.owner.lock_.release();
    c.result = detail::copy_out(c.user, c.argument.address, &count, sizeof(count));
  }

  static void flush(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock_);
    c.owner.discipline_.flush();
    ++c.owner.generation_;
    detail::wake_processes(&c.owner.discipline_);
    c.result = 0;
  }
  struct Entry;
  struct Claim;
  struct Group;
  struct SetGroup;
  struct Settings;
  struct ReadSettings;
  struct DrainCheck;
  struct Writer;
  struct Drain;
  struct Apply;
  struct ReleaseWriter;
  struct Success;
  struct Window;
  struct SetWindow;
  struct Readable;
  struct Flush;
  using Graph = flow::graph<
      Entry,
      flow::types<
          flow::node<
              Entry, flow::noop,
              flow::branch<
                  flow::condition<&claim_request>, flow::go<Claim>,
                  flow::branch<
                      flow::condition<&group_request>,
                      flow::go<Group>,
                      flow::branch<
                          flow::condition<&set_group_request>,
                          flow::go<SetGroup>,
                          flow::branch<
                              flow::condition<&settings_request>, flow::go<Settings>,
                              flow::branch<
                                  flow::condition<&set_settings_request>,
                                  flow::go<ReadSettings>,
                                  flow::branch<
                                      flow::condition<&window_request>,
                                      flow::go<Window>,
                                      flow::branch<
                                          flow::condition<&set_window_request>,
                                          flow::go<SetWindow>,
                                          flow::branch<
                                              flow::condition<&readable_request>,
                                              flow::go<Readable>,
                                              flow::branch<
                                                  flow::condition<&flush_request>,
                                                  flow::go<Flush>, flow::stop>>>>>>>>>>,
          flow::node<Claim, flow::action<&claim>, flow::stop>,
          flow::node<Group, flow::action<&group>, flow::stop>,
          flow::node<SetGroup, flow::action<&set_group>, flow::stop>,
          flow::node<Settings, flow::action<&settings>, flow::stop>,
          flow::node<
              ReadSettings, flow::action<&read_settings>,
              flow::branch<flow::condition<&ok>, flow::go<DrainCheck>, flow::stop>>,
          flow::node<DrainCheck, flow::noop,
                     flow::branch<flow::condition<&draining>, flow::go<Writer>,
                                  flow::go<Apply>>>,
          flow::node<Writer, flow::action<&writer>,
                     flow::branch<flow::condition<&ok>, flow::go<Drain>, flow::stop>>,
          flow::node<Drain, flow::action<&drain>, flow::go<Apply>>,
          flow::node<Apply, flow::action<&apply>,
                     flow::branch<flow::condition<&draining>, flow::go<ReleaseWriter>,
                                  flow::go<Success>>>,
          flow::node<ReleaseWriter, flow::action<&release_writer>, flow::go<Success>>,
          flow::node<Success, flow::action<&success>, flow::stop>,
          flow::node<Window, flow::action<&window>, flow::stop>,
          flow::node<SetWindow, flow::action<&set_window>, flow::stop>,
          flow::node<Readable, flow::action<&readable>, flow::stop>,
          flow::node<Flush, flow::action<&flush>, flow::stop>>>;
};

void Tty::initialize()
{
  lock_.init("tty state");
  read_lock_.init("tty reader");
  write_lock_.init("tty writer");
  settings_.c_iflag = ICRNL;
  // Keep xv6's LF console default. ONLCR is selectable through termios.
  settings_.c_oflag = OPOST;
  settings_.c_cflag = FixedControl;
  settings_.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;
  settings_.c_cc[VINTR] = 3;
  settings_.c_cc[VSUSP] = 26;
  settings_.c_cc[VERASE] = 127;
  settings_.c_cc[VKILL] = 21;
  settings_.c_cc[VEOF] = 4;
  settings_.c_cc[VWERASE] = 23;
  settings_.c_cc[VLNEXT] = 22;
  settings_.c_cc[VMIN] = 1;
  discipline_.flush();
}

void Tty::put_sync(int character)
{
  // Kernel diagnostics bypass termios, including during panic.
  output_.put_sync(character);
}

void Tty::receive(int character)
{
  xv::sync::ScopedLock guard(lock_);
  auto action =
      discipline_.receive(static_cast<unsigned char>(character), settings_, output_);
  if (action == InputAction::Interrupt || action == InputAction::Suspend)
  {
    detail::post_terminal_interrupt(this, action == InputAction::Suspend);
  }
  if (discipline_.available())
  {
    detail::wake_processes(&discipline_);
  }
}

int Tty::inject(unsigned char character, bool nonblocking)
{
  lock_.acquire();
  InjectFlow::Context context{*this, character, nonblocking};
  (void)InjectFlow::Graph::run<0>(context);
  lock_.release();
  return context.result;
}

void Tty::hangup()
{
  xv::sync::ScopedLock guard(lock_);
  hungup_ = true;
  // Publish an unfinished canonical line, then let readers drain it before EOF.
  discipline_.change_mode(false);
  detail::detach_terminal(this);
  detail::wake_processes(&discipline_);
}

bool Tty::input_writable(xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(lock_);
  if (table)
  {
    table->watch(&discipline_);
  }
  return !hungup_ && discipline_.input_room(settings_.c_lflag & ICANON);
}

unsigned Tty::poll(xv::sync::PollTable *table)
{
  PollFlow::Context context{*this, table};
  (void)PollFlow::Graph::run<0>(context);
  return context.events;
}

void Tty::timer_tick()
{
  xv::sync::ScopedLock guard(lock_);
  if (timed_wait_)
  {
    detail::wake_processes(&discipline_);
  }
}

int Tty::read(IoBuffer destination, int count)
{
  if (count <= 0)
  {
    return count == 0 ? 0 : -1;
  }
  if (!detail::may_read_terminal(this))
  {
    return -1;
  }
  if (detail::process_killed())
  {
    return -1;
  }
  if (destination.nonblocking)
  {
    if (!read_lock_.try_acquire())
    {
      return -EAGAIN;
    }
  }
  else if (!read_lock_.acquire_interruptible())
  {
    return -1;
  }
  xv::sync::ScopedLock reader(read_lock_, xv::sync::adopt_lock);
  lock_.acquire();
  ReadFlow::Context context{*this, destination, count, generation_,
                            detail::terminal_milliseconds()};
  (void)ReadFlow::Graph::run<0>(context);
  detail::wake_processes(&discipline_);
  lock_.release();
  return context.done ? context.done
                      : (context.fault ? -1 : (context.again ? -EAGAIN : 0));
}

int Tty::write(IoBuffer source, int count)
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
    if (!write_lock_.try_acquire())
    {
      return -EAGAIN;
    }
  }
  else if (!write_lock_.acquire_interruptible())
  {
    return -1;
  }
  xv::sync::ScopedLock writer(write_lock_, xv::sync::adopt_lock);
  lock_.acquire();
  unsigned flags = settings_.c_oflag;
  bool disconnected = hungup_;
  lock_.release();
  if (disconnected)
  {
    return count == 0 ? 0 : -1;
  }
  char input[32], output[64];
  WriteFlow::Context context{*this, source, count, flags, input, output};
  (void)WriteFlow::Graph::run<0>(context);
  return context.result;
}

int Tty::ioctl(unsigned request, IoBuffer argument)
{
  IoctlFlow::Context context{*this, request, argument,
                             argument.space == AddressSpace::User};
  (void)IoctlFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::drivers
