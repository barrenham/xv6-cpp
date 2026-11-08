#include "editor.h"
#include "user/runtime/terminal_style.h"
#include "user/runtime/ui.h"

namespace vim
{
unsigned
Editor::column(unsigned start, unsigned end) const {
  unsigned result = 0;
  for (; start < end; ++start)
  {
    result += buffer_.data()[start] == '\t' ? 8 - result % 8 : 1;
  }
  return result;
}

void
Editor::normalize() {
  if (cursor_ > buffer_.size())
  {
    cursor_ = buffer_.size();
  }
  unsigned start = buffer_.line_start(cursor_), end = buffer_.line_end(cursor_);
  if (mode_ != Mode::Insert && end > start && cursor_ == end)
  {
    --cursor_;
  }
}

bool
Editor::snapshot() {
  if (buffer_.snapshot(cursor_))
  {
    return true;
  }
  message_ = "Not enough memory for undo; edit cancelled";
  return false;
}

void
Editor::draw() {
  terminal_.resize();
  unsigned height = terminal_.rows - 2, start = buffer_.line_start(cursor_);
  if (top_ > buffer_.size() || start < top_)
  {
    top_ = start;
  }
  top_ = buffer_.line_start(top_);
  unsigned row = 0;
  for (unsigned at = top_; at < start; at = buffer_.line_end(at) + 1)
  {
    ++row;
  }
  while (row >= height)
  {
    top_ = buffer_.line_end(top_) + 1;
    --row;
  }
  unsigned visual = column(start, cursor_);
  if (visual < left_)
  {
    left_ = visual;
  }
  if (visual >= left_ + terminal_.columns)
  {
    left_ = visual - terminal_.columns + 1;
  }
  terminal_.text("\033[?25l\033[H");
  unsigned at = top_;
  for (unsigned r = 0; r < height; ++r)
  {
    if (at > buffer_.size())
    {
      terminal_.text("~");
    }
    else
    {
      unsigned end = buffer_.line_end(at), x = 0;
      for (unsigned i = at; i < end && x < left_ + terminal_.columns; ++i)
      {
        unsigned char value = buffer_.data()[i];
        unsigned width = value == '\t' ? 8 - x % 8 : 1;
        for (unsigned n = 0; n < width; ++n, ++x)
        {
          if (x >= left_ && x < left_ + terminal_.columns)
          {
            terminal_.character(value == '\t'                ? ' '
                                : value >= 32 && value < 127 ? value
                                                             : '?');
          }
        }
      }
      at = end < buffer_.size() ? end + 1 : buffer_.size() + 1;
    }
    terminal_.text("\033[K\r\n");
  }
  bool style = terminal_style::enabled(1);
  if (style)
  {
    terminal_.text("\033[7m");
  }
  terminal_.text(mode_ == Mode::Insert ? " INSERT | " : " NORMAL | ");
  if (terminal_.columns >= 40)
  {
    terminal_.safe(*file_.path ? file_.path : "[No Name]", terminal_.columns - 36);
  }
  terminal_.text(buffer_.modified() ? " [+]  " : "      ");
  unsigned line = 1;
  for (unsigned i = 0; i < start; ++i)
  {
    if (buffer_.data()[i] == '\n')
    {
      ++line;
    }
  }
  if (terminal_.columns >= 40)
  {
    terminal_.number(line);
    terminal_.character(':');
    terminal_.number(visual + 1);
  }
  if (style)
  {
    terminal_.text("\033[0m");
  }
  terminal_.text("\033[K");
  terminal_.move(terminal_.rows, 1);
  if (mode_ == Mode::Command || mode_ == Mode::Search)
  {
    terminal_.character(mode_ == Mode::Command ? ':' : '/');
    unsigned first =
        input_size_ >= terminal_.columns - 1 ? input_size_ - terminal_.columns + 2 : 0;
    terminal_.safe(input_ + first, terminal_.columns - 1);
  }
  else
  {
    terminal_.safe(message_, terminal_.columns - 1);
  }
  terminal_.text("\033[K");
  if (mode_ == Mode::Command || mode_ == Mode::Search)
  {
    terminal_.move(terminal_.rows, input_size_ + 2 < terminal_.columns
                                       ? input_size_ + 2
                                       : terminal_.columns);
  }
  else
  {
    terminal_.move(row + 1, visual - left_ + 1);
  }
  terminal_.text("\033[?25h");
  terminal_.flush();
}

void
Editor::normal(int key) {
  if (key == ':')
  {
    mode_ = Mode::Command;
    input_size_ = 0;
    input_[0] = 0;
    pending_ = 0;
    return;
  }
  if (key == '/')
  {
    mode_ = Mode::Search;
    input_size_ = 0;
    input_[0] = 0;
    pending_ = 0;
    return;
  }
  if (pending_ == 'd' && key == 'd')
  {
    unsigned start = buffer_.line_start(cursor_), end = buffer_.line_end(cursor_);
    if (end < buffer_.size())
    {
      ++end;
    }
    if (end > start && snapshot())
    {
      buffer_.erase(start, end - start);
      cursor_ = start;
    }
    pending_ = 0;
    normalize();
    return;
  }
  if (pending_ == 'g' && key == 'g')
  {
    cursor_ = 0;
    pending_ = 0;
    return;
  }
  pending_ = 0;
  if (key == 'd' || key == 'g')
  {
    pending_ = key;
    return;
  }
  if (key == 'i' || key == 'a' || key == 'I' || key == 'A' || key == 'o' || key == 'O')
  {
    if (!snapshot())
    {
      return;
    }
    if (key == 'I' || key == 'O')
    {
      cursor_ = buffer_.line_start(cursor_);
    }
    if (key == 'A' || key == 'o')
    {
      cursor_ = buffer_.line_end(cursor_);
    }
    if (key == 'a' && cursor_ < buffer_.line_end(cursor_))
    {
      ++cursor_;
    }
    if (key == 'o' || key == 'O')
    {
      if (!buffer_.insert(cursor_, "\n", 1))
      {
        message_ = "File limit or memory exhausted";
        return;
      }
      if (key == 'o')
      {
        ++cursor_;
      }
    }
    mode_ = Mode::Insert;
    message_ = "-- INSERT --";
    return;
  }
  if (key == 'h' || key == Left)
  {
    if (cursor_ > buffer_.line_start(cursor_))
    {
      --cursor_;
    }
  }
  else if (key == 'l' || key == Right)
  {
    if (cursor_ + 1 < buffer_.line_end(cursor_))
    {
      ++cursor_;
    }
  }
  else if (key == 'j' || key == Down)
  {
    cursor_ = buffer_.vertical(cursor_, 1);
  }
  else if (key == 'k' || key == Up)
  {
    cursor_ = buffer_.vertical(cursor_, -1);
  }
  else if (key == '0' || key == Home)
  {
    cursor_ = buffer_.line_start(cursor_);
  }
  else if (key == '$' || key == End)
  {
    cursor_ = buffer_.line_end(cursor_);
  }
  else if (key == 'G')
  {
    cursor_ =
        buffer_.line_start(buffer_.size() && buffer_.data()[buffer_.size() - 1] == '\n'
                               ? buffer_.size() - 1
                               : buffer_.size());
  }
  else if (key == PageDown || key == PageUp || key == 4 || key == 21)
  {
    unsigned lines =
        key == 4 || key == 21 ? (terminal_.rows - 2) / 2 : terminal_.rows - 2;
    for (unsigned i = 0; i < lines; ++i)
    {
      cursor_ = buffer_.vertical(cursor_, key == PageUp || key == 21 ? -1 : 1);
    }
  }
  else if (key == 'x' || key == Delete)
  {
    if (cursor_ < buffer_.line_end(cursor_) && snapshot())
    {
      buffer_.erase(cursor_, 1);
    }
  }
  else if (key == 'u' || key == 18)
  {
    if (!buffer_.undo(cursor_))
    {
      message_ = "Nothing to undo";
    }
  }
  else if (key == 'n')
  {
    find();
  }
  normalize();
}

void
Editor::insert(int key) {
  if (key == Escape || key == 3)
  {
    mode_ = Mode::Normal;
    if (cursor_ > buffer_.line_start(cursor_))
    {
      --cursor_;
    }
    message_ = "";
    normalize();
    return;
  }
  if (key == 127 || key == 8)
  {
    if (cursor_)
    {
      buffer_.erase(--cursor_, 1);
    }
  }
  else if (key == Delete)
  {
    buffer_.erase(cursor_, 1);
  }
  else if (key == Left)
  {
    if (cursor_)
    {
      --cursor_;
    }
  }
  else if (key == Right)
  {
    if (cursor_ < buffer_.size())
    {
      ++cursor_;
    }
  }
  else if (key == Up || key == Down)
  {
    cursor_ = buffer_.vertical(cursor_, key == Up ? -1 : 1);
  }
  else if (key == Home)
  {
    cursor_ = buffer_.line_start(cursor_);
  }
  else if (key == End)
  {
    cursor_ = buffer_.line_end(cursor_);
  }
  else if (key == '\r' || key == '\n' || key == '\t' || (key >= 32 && key < 127))
  {
    char value = key == '\r' ? '\n' : key;
    if (buffer_.insert(cursor_, &value, 1))
    {
      ++cursor_;
    }
    else
    {
      message_ = "File limit or memory exhausted";
    }
  }
}

void
Editor::find() {
  unsigned length = strlen(search_);
  if (!length)
  {
    message_ = "No search pattern";
    return;
  }
  for (unsigned i = 0; i < buffer_.size(); ++i)
  {
    unsigned at = (cursor_ + 1 + i) % buffer_.size();
    if (length <= buffer_.size() - at && !memcmp(buffer_.data() + at, search_, length))
    {
      cursor_ = at;
      normalize();
      message_ = "";
      return;
    }
  }
  message_ = "Pattern not found";
}

void
Editor::execute() {
  char *name = strchr(input_, ' ');
  if (name)
  {
    *name++ = 0;
    while (*name == ' ')
    {
      ++name;
    }
  }
  if (!strcmp(input_, "q!"))
  {
    done_ = true;
    return;
  }
  if (!strcmp(input_, "q"))
  {
    if (buffer_.modified())
    {
      message_ = "No write since last change (use :q!)";
    }
    else
    {
      done_ = true;
    }
    return;
  }
  bool quit = !strcmp(input_, "wq") || !strcmp(input_, "x");
  if (!strcmp(input_, "w") || quit)
  {
    if (quit && !buffer_.modified() && (!name || !*name) && !strcmp(input_, "x"))
    {
      done_ = true;
      return;
    }
    if (file_.save(buffer_, name))
    {
      message_ = "Written";
      if (quit)
      {
        done_ = true;
      }
    }
    else
    {
      message_ = file_.error;
    }
  }
  else if (!strcmp(input_, "help"))
  {
    message_ = "i/a/I/A/o/O  hjkl/arrows  0/$/gg/G  x/dd  u  /find n  :w :wq :q!";
  }
  else
  {
    message_ = "Unknown command (use :help)";
  }
}

void
Editor::command(int key) {
  if (key == Escape || key == 3)
  {
    mode_ = Mode::Normal;
    message_ = "";
    return;
  }
  if (key == '\n' || key == '\r')
  {
    bool search = mode_ == Mode::Search;
    mode_ = Mode::Normal;
    if (search)
    {
      if (input_size_ >= sizeof(search_))
      {
        message_ = "Search pattern too long";
      }
      else
      {
        strcpy(search_, input_);
        find();
      }
    }
    else
    {
      execute();
    }
    return;
  }
  if (key == 8 || key == 127)
  {
    if (input_size_)
    {
      input_[--input_size_] = 0;
    }
  }
  else if (key >= 32 && key < 127 && input_size_ + 1 < sizeof(input_))
  {
    input_[input_size_++] = key;
    input_[input_size_] = 0;
  }
}

int
Editor::run(const char *path) {
  if (path && !file_.load(buffer_, path))
  {
    ui::error("vim: %s\n", file_.error);
    return 1;
  }
  if (!terminal_.enter())
  {
    ui::error("vim: requires a foreground interactive terminal on stdin/stdout\n");
    return 1;
  }
  while (!done_ && terminal_.good)
  {
    draw();
    int key = terminal_.key();
    if (key == EndOfInput)
    {
      return terminal_.interrupted() ? 128 + terminal_.interrupted() : 1;
    }
    if (mode_ == Mode::Insert)
    {
      insert(key);
    }
    else if (mode_ == Mode::Normal)
    {
      normal(key);
    }
    else
    {
      command(key);
    }
  }
  return terminal_.good ? 0 : 1;
}
} // namespace vim
