#include "terminal.h"
#include "kernel/fs/poll.h"

namespace vim
{
namespace
{
char output[4096];
char row_text[240], previous[100][240];
unsigned lengths[100]{};
bool valid[100]{}, reversed[100]{};
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
  for (auto &entry : valid)
  {
    entry = false;
  }
  resize();
  text("\033[?1049h\033[0m\033[2J\033[H");
  flush();
  return good;
}

void
Terminal::resize() {
  unsigned old_columns = columns, old_rows = rows;
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
  if (columns != old_columns || rows != old_rows)
  {
    for (auto &entry : valid)
    {
      entry = false;
    }
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
Terminal::begin_row(unsigned row, bool reverse) {
  row_ = row - 1;
  row_used_ = 0;
  reverse_ = reverse;
  capturing_ = true;
}

void
Terminal::end_row() {
  capturing_ = false;
  unsigned first = 0, end = row_used_;
  bool same_style = valid[row_] && reversed[row_] == reverse_;
  if (same_style)
  {
    while (first < row_used_ && first < lengths[row_] &&
           row_text[first] == previous[row_][first])
    {
      ++first;
    }
    if (row_used_ == lengths[row_])
    {
      while (end > first && row_text[end - 1] == previous[row_][end - 1])
      {
        --end;
      }
    }
  }
  bool clear = row_used_ < columns && (!valid[row_] || row_used_ < lengths[row_]);
  if (first < end || clear || !same_style)
  {
    move(row_ + 1, first + 1);
    if (reverse_)
    {
      text("\033[7m");
    }
    for (unsigned i = first; i < end; ++i)
    {
      character(row_text[i]);
    }
    if (reverse_)
    {
      text("\033[0m");
    }
    if (clear)
    {
      text("\033[K");
    }
  }
  for (unsigned i = 0; i < row_used_; ++i)
  {
    previous[row_][i] = row_text[i];
  }
  lengths[row_] = row_used_;
  reversed[row_] = reverse_;
  valid[row_] = true;
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
  if (capturing_)
  {
    if (row_used_ < columns)
    {
      row_text[row_used_++] = value;
    }
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
