#pragma once
#include "kernel/tty/termios.h"

namespace shell
{
void prompt();
const char *prompt_text();

class Editor final
{
public:

  Editor(char *line, int capacity, bool interactive = true);
  ~Editor();

  bool active() const
  {
    return enabled_;
  }

  int input(char value, int &used);
  void redraw(int used, bool fresh_line = false);

private:

  void complete(int &used);
  void remember(int used);
  char *line_;
  int capacity_, cursor_ = 0, history_ = 0, escape_ = 0, parameter_ = 0;
  char draft_[512]{};
  termios saved_{};
  int columns_ = 80, prefix_ = 0, displayed_row_ = 0;
  bool enabled_ = false;
};
} // namespace shell
