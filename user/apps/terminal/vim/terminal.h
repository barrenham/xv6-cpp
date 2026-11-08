#pragma once
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"

namespace vim
{
enum Key
{
  EndOfInput = -1,
  Escape = 27,
  Left = 256,
  Right,
  Up,
  Down,
  Home,
  End,
  PageUp,
  PageDown,
  Delete
};

class Terminal final
{
public:

  unsigned columns = 80, rows = 24;
  bool good = true;
  ~Terminal();
  bool enter();
  void resize();
  int key();
  void text(const char *text);
  void character(char value);
  void number(unsigned value);
  void move(unsigned row, unsigned column);
  void safe(const char *text, unsigned maximum);
  void flush();
  int interrupted() const;

private:

  bool active_ = false;
  termios saved_{};
  struct sigaction previous_[5]{};
  unsigned installed_ = 0, used_ = 0;
  int pending_ = -1;
  int byte(int timeout);
};
} // namespace vim
