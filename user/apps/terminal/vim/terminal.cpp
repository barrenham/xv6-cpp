#include "terminal.h"
#include "kernel/fs/poll.h"

namespace vim
{
namespace
{
char output[4096];
volatile sig_atomic_t stopping = 0;
constexpr int signals[] = {SIGTERM, SIGHUP, SIGINT, SIGPIPE, SIGTSTP};

void
stop(int signal) {
  stopping = signal;
}
} // namespace

Terminal::~Terminal() {
  if (active_)
  {
    text("\033[0m\033[?25h\033[?1049l\r");
    flush();
    tcsetattr(0, TCSANOW, &saved_);
  }
  while (installed_)
  {
    --installed_;
    sigaction(signals[installed_], &previous_[installed_], nullptr);
  }
}

bool
Terminal::enter() {
  termios destination{};
  if (tcgetattr(0, &saved_) < 0 || tcgetattr(1, &destination) < 0 ||
      tcgetpgrp(0) != getpgrp())
  {
    return false;
  }
  for (int signal_number : signals)
  {
    struct sigaction action{signal_number == SIGTSTP ? SIG_IGN : stop, 0, 0}, old{};
    if (sigaction(signal_number, &action, &old) < 0)
    {
      return false;
    }
    previous_[installed_++] = old;
  }
  termios raw = saved_;
  cfmakeraw(&raw);
  if (tcsetattr(0, TCSANOW, &raw) < 0)
  {
    return false;
  }
  active_ = true;
  resize();
  text("\033[?1049h\033[0m\033[2J\033[H");
  flush();
  return good;
}

void
Terminal::resize() {
  winsize size{};
  if (ioctl(0, TIOCGWINSZ, reinterpret_cast<unsigned long>(&size)) == 0)
  {
    if (size.ws_col)
    {
      columns = size.ws_col;
    }
    if (size.ws_row)
    {
      rows = size.ws_row;
    }
  }
  if (columns < 20)
  {
    columns = 20;
  }
  if (columns > 240)
  {
    columns = 240;
  }
  if (rows < 4)
  {
    rows = 4;
  }
  if (rows > 100)
  {
    rows = 100;
  }
}

int
Terminal::byte(int timeout) {
  for (;;)
  {
    if (stopping || !good)
    {
      return EndOfInput;
    }
    pollfd event{0, POLLIN, 0};
    int ready = poll(&event, 1, timeout);
    if (ready < 0 && errno == EINTR)
    {
      continue;
    }
    if (ready <= 0)
    {
      return ready ? EndOfInput : -2;
    }
    if (!(event.revents & POLLIN))
    {
      return EndOfInput;
    }
    unsigned char value;
    int n = read(0, &value, 1);
    if (n < 0 && errno == EINTR)
    {
      continue;
    }
    return n == 1 ? value : EndOfInput;
  }
}

int
Terminal::key() {
  int value = pending_ >= 0 ? pending_ : byte(-1);
  pending_ = -1;
  if (value != Escape)
  {
    return value;
  }
  int next = byte(40);
  if (next < 0)
  {
    return Escape;
  }
  if (next != '[' && next != 'O')
  {
    pending_ = next;
    return Escape;
  }
  unsigned parameter = 0;
  for (unsigned i = 0; i < 16; ++i)
  {
    int c = byte(40);
    if (c < 0)
    {
      return Escape;
    }
    if (c >= '0' && c <= '9')
    {
      parameter = parameter * 10 + c - '0';
      if (parameter > 999)
      {
        return Escape;
      }
      continue;
    }
    if (c == 'A')
    {
      return Up;
    }
    if (c == 'B')
    {
      return Down;
    }
    if (c == 'C')
    {
      return Right;
    }
    if (c == 'D')
    {
      return Left;
    }
    if (c == 'H')
    {
      return Home;
    }
    if (c == 'F')
    {
      return End;
    }
    if (c == '~')
    {
      if (parameter == 1 || parameter == 7)
      {
        return Home;
      }
      if (parameter == 4 || parameter == 8)
      {
        return End;
      }
      if (parameter == 3)
      {
        return Delete;
      }
      if (parameter == 5)
      {
        return PageUp;
      }
      if (parameter == 6)
      {
        return PageDown;
      }
    }
    return Escape;
  }
  return Escape;
}

void
Terminal::flush() {
  unsigned sent = 0;
  while (sent < used_ && good)
  {
    int n = write(1, output + sent, used_ - sent);
    if (n < 0 && errno == EINTR)
    {
      continue;
    }
    if (n <= 0)
    {
      good = false;
      break;
    }
    sent += n;
  }
  used_ = 0;
}

void
Terminal::character(char value) {
  if (!good)
  {
    return;
  }
  output[used_++] = value;
  if (used_ == sizeof(output))
  {
    flush();
  }
}

void
Terminal::text(const char *text) {
  while (*text)
  {
    character(*text++);
  }
}

void
Terminal::number(unsigned value) {
  char digits[12];
  unsigned used = 0;
  do
  {
    digits[used++] = '0' + value % 10;
    value /= 10;
  } while (value);
  while (used)
  {
    character(digits[--used]);
  }
}

void
Terminal::move(unsigned row, unsigned column) {
  text("\033[");
  number(row);
  character(';');
  number(column);
  character('H');
}

void
Terminal::safe(const char *text, unsigned maximum) {
  for (unsigned i = 0; text[i] && i < maximum; ++i)
  {
    character(text[i] >= 32 && text[i] < 127 ? text[i] : '?');
  }
}

int
Terminal::interrupted() const {
  return stopping;
}
} // namespace vim
