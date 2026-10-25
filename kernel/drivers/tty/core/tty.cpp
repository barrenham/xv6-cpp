module;
#include "kernel/tty/termios.h"
#include "kernel/fs/poll.h"

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

int Tty::inject(unsigned char character)
{
  lock_.acquire();
  for (;;)
  {
    if (hungup_ || detail::process_killed())
    {
      lock_.release();
      return -1;
    }
    auto action = discipline_.receive(character, settings_, output_);
    if (action != InputAction::Retry)
    {
      if (action == InputAction::Interrupt || action == InputAction::Suspend)
        detail::post_terminal_interrupt(this, action == InputAction::Suspend);
      detail::wake_processes(&discipline_);
      lock_.release();
      return 0;
    }
    detail::prepare_interruptible_sleep(&discipline_);
    lock_.release();
    detail::sleep_process();
    lock_.acquire();
  }
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
  if (table) table->watch(&discipline_);
  return !hungup_ && discipline_.input_room(settings_.c_lflag & ICANON);
}

unsigned Tty::poll(xv::sync::PollTable *table)
{
  lock_.acquire();
  if (table) table->watch(&discipline_);
  unsigned minimum = 1;
  if (!(settings_.c_lflag & ICANON) && !settings_.c_cc[VTIME] && settings_.c_cc[VMIN])
    minimum = settings_.c_cc[VMIN];
  unsigned events = discipline_.available() >= minimum ? POLLIN : 0;
  bool disconnected = hungup_;
  lock_.release();
  if (disconnected) return events | POLLIN | POLLHUP;
  if ((!table || (table->events() & POLLOUT)) && output_.writable(table)) events |= POLLOUT;
  return events;
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
  if (!read_lock_.acquire_interruptible())
  {
    return -1;
  }
  xv::sync::ScopedLock reader(read_lock_, xv::sync::adopt_lock);
  lock_.acquire();
  unsigned generation = generation_;
  unsigned long start = detail::terminal_milliseconds();
  int done = 0;
  bool finished = false;
  bool fault = false;
  while (!finished)
  {
    if (!detail::may_read_terminal(this))
    {
      fault = true;
      break;
    }
    if (generation != generation_)
    {
      if (done)
      {
        break;
      }
      generation = generation_;
      start = detail::terminal_milliseconds();
    }
    const bool canonical = settings_.c_lflag & ICANON;
    const unsigned timeout = canonical ? 0 : settings_.c_cc[VTIME] * 100U;
    int minimum = canonical ? 1 : settings_.c_cc[VMIN];
    if (minimum > count)
    {
      minimum = count;
    }
    while (discipline_.available() && done < count)
    {
      auto byte = discipline_.front();
      if (byte.eof)
      {
        discipline_.consume();
        if (canonical)
        {
          finished = true;
          break;
        }
        continue;
      }
      char value = byte.value;
      if (detail::copy_out(destination.space == AddressSpace::User,
                           destination.address + done, &value, 1) < 0)
      {
        fault = finished = true;
        break;
      }
      discipline_.consume();
      ++done;
      start = detail::terminal_milliseconds();
      if (canonical && byte.end)
      {
        finished = true;
        break;
      }
    }
    // EOF is a record boundary, never an extra empty read after its payload.
    if (canonical && !finished && done && discipline_.available() &&
        discipline_.front().eof)
    {
      discipline_.consume();
      finished = true;
    }
    if (finished || done == count || hungup_)
    {
      break;
    }
    if (!canonical)
    {
      if (done >= minimum && (minimum || !timeout || done))
      {
        break;
      }
      if (timeout && (!minimum || done) &&
          detail::terminal_milliseconds() - start >= timeout)
      {
        break;
      }
    }
    if (detail::process_killed())
    {
      fault = true;
      break;
    }
    detail::wake_processes(&discipline_);
    timed_wait_ = timeout && (!minimum || done);
    detail::prepare_interruptible_sleep(&discipline_);
    lock_.release();
    detail::sleep_process();
    lock_.acquire();
    timed_wait_ = false;
  }
  detail::wake_processes(&discipline_);
  lock_.release();
  return done ? done : (fault ? -1 : 0);
}

int Tty::write(IoBuffer source, int count)
{
  if (count < 0)
  {
    return -1;
  }
  if (!write_lock_.acquire_interruptible())
  {
    return -1;
  }
  xv::sync::ScopedLock writer(write_lock_, xv::sync::adopt_lock);
  lock_.acquire();
  unsigned flags = settings_.c_oflag;
  bool disconnected = hungup_;
  lock_.release();
  if (disconnected) return count == 0 ? 0 : -1;
  int done = 0;
  while (done < count)
  {
    if (detail::process_killed())
    {
      return done ? done : -1;
    }
    char input[32], output[64];
    int part = count - done;
    if (part > 32)
    {
      part = 32;
    }
    if (detail::copy_in(input, source.space == AddressSpace::User,
                        source.address + done, part) < 0)
    {
      return done ? done : -1;
    }
    int used = 0;
    for (int i = 0; i < part; ++i)
    {
      if (input[i] == '\n' && (flags & (OPOST | ONLCR)) == (OPOST | ONLCR))
      {
        output[used++] = '\r';
      }
      output[used++] = input[i];
    }
    if (output_.transmit(output, used) != used)
      return done ? done : -1;
    done += part;
  }
  return done;
}

int Tty::ioctl(unsigned request, IoBuffer argument)
{
  bool user = argument.space == AddressSpace::User;
  if (request == TIOCSCTTY)
  {
    if (argument.address != 0)
    {
      return -1;
    }
    xv::sync::ScopedLock guard(lock_);
    return hungup_ ? -1 : detail::claim_terminal(this);
  }
  if (request == TIOCGPGRP)
  {
    int group = detail::terminal_foreground(this);
    if (group < 0)
    {
      return -1;
    }
    return detail::copy_out(user, argument.address, &group, sizeof(group));
  }
  if (request == TIOCSPGRP)
  {
    int group;
    if (detail::copy_in(&group, user, argument.address, sizeof(group)) < 0)
    {
      return -1;
    }
    xv::sync::ScopedLock guard(lock_);
    int result = detail::set_terminal_foreground(this, group);
    if (result == 0)
    {
      detail::wake_processes(&discipline_);
    }
    return result;
  }
  if (request == TCGETS)
  {
    lock_.acquire();
    auto value = settings_;
    lock_.release();
    return detail::copy_out(user, argument.address, &value, sizeof(value));
  }
  if (request == TCSETS || request == TCSETSW || request == TCSETSF)
  {
    termios value{};
    if (detail::copy_in(&value, user, argument.address, sizeof(value)) < 0 ||
        !valid(value))
    {
      return -1;
    }
    // Serialize with writers before draining; never wait with the state lock.
    if (request != TCSETS)
    {
      if (!write_lock_.acquire_interruptible())
      {
        return -1;
      }
      output_.drain();
    }
    lock_.acquire();
    if (request == TCSETSF)
    {
      discipline_.flush();
    }
    if ((value.c_lflag ^ settings_.c_lflag) & ICANON)
    {
      discipline_.change_mode(value.c_lflag & ICANON);
    }
    settings_ = value;
    ++generation_;
    detail::wake_processes(&discipline_);
    lock_.release();
    if (request != TCSETS)
    {
      write_lock_.release();
    }
    return 0;
  }
  if (request == TIOCGWINSZ)
  {
    lock_.acquire();
    auto value = size_;
    lock_.release();
    return detail::copy_out(user, argument.address, &value, sizeof(value));
  }
  if (request == TIOCSWINSZ)
  {
    winsize value{};
    if (detail::copy_in(&value, user, argument.address, sizeof(value)) < 0)
    {
      return -1;
    }
    xv::sync::ScopedLock guard(lock_);
    size_ = value;
    return 0;
  }
  if (request == FIONREAD)
  {
    lock_.acquire();
    int count = discipline_.readable_bytes();
    lock_.release();
    return detail::copy_out(user, argument.address, &count, sizeof(count));
  }
  if (request == TCFLSH && argument.address == TCIFLUSH)
  {
    xv::sync::ScopedLock guard(lock_);
    discipline_.flush();
    ++generation_;
    detail::wake_processes(&discipline_);
    return 0;
  }
  return -1;
}
} // namespace xv::drivers
