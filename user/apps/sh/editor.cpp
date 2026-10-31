#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/format/layout.h"
#include "user/runtime/user.h"
#include "editor.h"
#include "user/runtime/directory.h"

namespace shell
{
namespace
{
char history[32][512];
int history_count = 0;
char matches[128][257];
unsigned match_count;

void add_match(const char *name, const char *prefix)
{
  unsigned size = strlen(prefix);
  if (strlen(name) >= sizeof(matches[0]) || strlen(name) < size || memcmp(name, prefix, size))
  {
    return;
  }
  for (unsigned i = 0; i < match_count; ++i)
  {
    if (!strcmp(matches[i], name))
    {
      return;
    }
  }
  if (match_count < 128)
  {
    strcpy(matches[match_count++], name);
  }
}

void scan(const char *directory, const char *prefix, bool command)
{
  int fd = open(*directory ? directory : ".", O_RDONLY);
  if (fd < 0)
  {
    return;
  }
  DirectoryReader reader(fd);const linux_dirent64 *entry;
  while (reader.next(entry)==1)
  {
    char name[257];strcpy(name,entry->d_name);
    if (!strcmp(name, ".") || !strcmp(name, "..") ||
        (name[0] == '.' && prefix[0] != '.'))
    {
      continue;
    }
    bool plain = true;
    for (const char *p = name; *p; ++p)
    {
      if (strchr(" \t\r\n\\\"'|&;()<>$", *p))
      {
        plain = false;
      }
    }
    if (!plain)
    {
      continue;
    }
    static char path[MAXPATH];
    unsigned size = strlen(directory);
    if (size + strlen(name) + 2 > sizeof(path))
    {
      continue;
    }
    strcpy(path, directory);
    if (size && path[size - 1] != '/')
    {
      path[size++] = '/';
    }
    strcpy(path + size, name);
    struct stat st;
    if (stat(path, &st) < 0)
    {
      continue;
    }
    if (command && st.type != T_DIR && (st.type != T_FILE || access(path, 1) < 0))
    {
      continue;
    }
    if (st.type == T_DIR)
    {
      strcpy(name + strlen(name), "/");
    }
    add_match(name, prefix);
  }
  close(fd);
}
} // namespace

Editor::Editor(char *line, int capacity, bool interactive)
    : line_(line), capacity_(capacity), history_(history_count)
{
  if (!interactive || tcgetattr(0, &saved_) < 0 || !(saved_.c_lflag & ICANON) ||
      !(saved_.c_lflag & ECHO))
  {
    return;
  }
  termios settings = saved_;
  settings.c_lflag &= ~(ICANON | ECHO | ECHONL | ISIG | IEXTEN);
  settings.c_cc[VMIN] = 1;
  settings.c_cc[VTIME] = 0;
  enabled_ = tcsetattr(0, TCSANOW, &settings) == 0;
}

Editor::~Editor()
{
  if (enabled_)
  {
    tcsetattr(0, TCSANOW, &saved_);
  }
}

void Editor::redraw(int used)
{
  write(2, "\r\033[2K", 5);
  prompt();
  if (used)
  {
    write(2, line_, used);
  }
  for (int i = cursor_; i < used; ++i)
  {
    write(2, "\b", 1);
  }
}

void Editor::remember(int used)
{
  if (!used || (history_count &&
                strlen(history[history_count - 1]) == static_cast<unsigned>(used) &&
                !memcmp(history[history_count - 1], line_, used)))
  {
    return;
  }
  if (history_count == 32)
  {
    memmove(history[0], history[1], 31 * sizeof(history[0]));
    --history_count;
  }
  memmove(history[history_count], line_, used);
  history[history_count++][used] = 0;
}

void Editor::complete(int &used)
{
  int start = cursor_;
  while (start && !strchr(" \t;|&", line_[start - 1]))
  {
    --start;
  }
  for (int i = start; i < cursor_; ++i)
  {
    if (strchr("\\\"'$()<>`,", line_[i]))
    {
      return;
    }
  }
  static char token[MAXPATH];
  int size = cursor_ - start;
  if (size >= MAXPATH)
  {
    return;
  }
  memmove(token, line_ + start, size);
  token[size] = 0;
  bool command = true;
  for (int i = start - 1; i >= 0 && !strchr(";|&", line_[i]); --i)
  {
    if (line_[i] != ' ' && line_[i] != '\t')
    {
      command = false;
    }
  }
  char *base = token;
  for (char *p = token; *p; ++p)
  {
    if (*p == '/')
    {
      base = p + 1;
    }
  }
  match_count = 0;
  if (base != token)
  {
    static char directory[MAXPATH];
    unsigned length = base - token;
    memmove(directory, token, length);
    directory[length] = 0;
    scan(directory, base, command);
  }
  else if (!command)
  {
    scan(".", base, false);
  }
  else
  {
    const char *builtins[] = {"cd", "export", "unset", "umask", "jobs", "fg", "bg"};
    for (auto *name : builtins)
    {
      add_match(name, base);
    }
    const char *path = getenv("PATH");
    if (!path)
    {
      path = "/bin:/sbin:/usr/tests";
    }
    for (;;)
    {
      static char directory[MAXPATH];
      unsigned length = 0;
      while (path[length] && path[length] != ':')
      {
        ++length;
      }
      if (length < sizeof(directory))
      {
        memmove(directory, path, length);
        directory[length] = 0;
        scan(directory, base, true);
      }
      path += length;
      if (!*path)
      {
        break;
      }
      ++path;
    }
  }
  if (!match_count)
  {
    write(2, "\a", 1);
    return;
  }
  unsigned common = strlen(matches[0]);
  for (unsigned i = 1; i < match_count; ++i)
  {
    unsigned j = 0;
    while (j < common && matches[0][j] == matches[i][j])
    {
      ++j;
    }
    common = j;
  }
  // At the match cap list candidates without inserting an uncertain common prefix.
  if (match_count == 128)
  {
    common = strlen(base);
  }
  unsigned prefix = strlen(base), extra = common - prefix;
  bool space =
      match_count == 1 && common && matches[0][common - 1] != '/' && cursor_ == used;
  if (used + extra + space < static_cast<unsigned>(capacity_ - 1))
  {
    memmove(line_ + cursor_ + extra + space, line_ + cursor_, used - cursor_ + 1);
    memmove(line_ + cursor_, matches[0] + prefix, extra);
    cursor_ += extra;
    if (space)
    {
      line_[cursor_++] = ' ';
    }
    used += extra + space;
  }
  if (match_count > 1 && !extra)
  {
    write(2, "\n", 1);
    for (unsigned i = 0; i < match_count; ++i)
    {
      write(2, matches[i], strlen(matches[i]));
      write(2, "  ", 2);
    }
    write(2, "\n", 1);
  }
  redraw(used);
}

int Editor::input(char value, int &used)
{
  if (escape_)
  {
    if (escape_ == 1)
    {
      escape_ = (value == '[' || value == 'O') ? 2 : 0;
      parameter_ = 0;
      return 0;
    }
    if (value >= '0' && value <= '9')
    {
      if (parameter_ < 100)
      {
        parameter_ = parameter_ * 10 + value - '0';
      }
      return 0;
    }
    escape_ = 0;
    if (value == 'D' && cursor_)
    {
      --cursor_;
    }
    if (value == 'C' && cursor_ < used)
    {
      ++cursor_;
    }
    if (value == 'H' || (value == '~' && (parameter_ == 1 || parameter_ == 7)))
    {
      cursor_ = 0;
    }
    if (value == 'F' || (value == '~' && (parameter_ == 4 || parameter_ == 8)))
    {
      cursor_ = used;
    }
    if (value == '~' && parameter_ == 3 && cursor_ < used)
    {
      memmove(line_ + cursor_, line_ + cursor_ + 1, used - cursor_);
      --used;
    }
    if (value == 'A' || value == 'B')
    {
      if (history_ == history_count)
      {
        strcpy(draft_, line_);
      }
      if (value == 'A' && history_ > 0)
      {
        --history_;
      }
      if (value == 'B' && history_ < history_count)
      {
        ++history_;
      }
      strcpy(line_, history_ == history_count ? draft_ : history[history_]);
      used = cursor_ = strlen(line_);
    }
    redraw(used);
    return 0;
  }
  if (value == 27)
  {
    escape_ = 1;
    return 0;
  }
  if (value == '\n' || value == '\r')
  {
    remember(used);
    line_[used++] = '\n';
    line_[used] = 0;
    if (tcsetattr(0, TCSANOW, &saved_) == 0)
    {
      enabled_ = false;
    }
    write(2, "\n", 1);
    return 1;
  }
  if (value == 3)
  {
    used = cursor_ = 0;
    line_[0] = 0;
    history_ = history_count;
    write(2, "^C\n", 3);
    prompt();
    return 0;
  }
  if (value == 4 && !used)
  {
    return -1;
  }
  if ((value == 4 && cursor_ < used) || value == 127 || value == 8)
  {
    if (value != 4)
    {
      if (!cursor_)
      {
        return 0;
      }
      --cursor_;
    }
    if (cursor_ < used)
    {
      memmove(line_ + cursor_, line_ + cursor_ + 1, used - cursor_);
      --used;
      redraw(used);
    }
    return 0;
  }
  if (value == 1)
  {
    cursor_ = 0;
    redraw(used);
    return 0;
  }
  if (value == 5)
  {
    cursor_ = used;
    redraw(used);
    return 0;
  }
  if (value == 21)
  {
    used = cursor_ = 0;
    line_[0] = 0;
    redraw(used);
    return 0;
  }
  if (value == '\t')
  {
    complete(used);
    return 0;
  }
  if (static_cast<unsigned char>(value) < 32 || used >= capacity_ - 2)
  {
    return 0;
  }
  bool append = cursor_ == used;
  memmove(line_ + cursor_ + 1, line_ + cursor_, used - cursor_ + 1);
  line_[cursor_++] = value;
  ++used;
  if (append)
  {
    write(2, &value, 1);
  }
  else
  {
    redraw(used);
  }
  return 0;
}
} // namespace shell
