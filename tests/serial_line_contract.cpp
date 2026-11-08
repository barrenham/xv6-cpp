#include "user/apps/shell/sh/line_display.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

struct Screen
{
  explicit Screen(unsigned width)
      : columns(width), cells(128, std::string(width, ' ')) {
  }

  unsigned columns, row = 0, column = 0;
  bool wrap = false;
  std::vector<std::string> cells;

  void
  feed(const std::string &bytes) {
    for (unsigned i = 0; i < bytes.size(); ++i)
    {
      char c = bytes[i];
      if (c == '\033')
      {
        assert(bytes[++i] == '[');
        unsigned n = 0;
        while (bytes[++i] >= '0' && bytes[i] <= '9')
        {
          n = n * 10 + bytes[i] - '0';
        }
        if (!n)
        {
          n = 1;
        }
        if (bytes[i] == 'A')
        {
          assert(row >= n);
          row -= n;
        }
        else if (bytes[i] == 'B')
        {
          row += n;
        }
        else if (bytes[i] == 'C')
        {
          column += n;
        }
        else
        {
          assert(false && "unexpected or destructive terminal command");
        }
        wrap = false;
      }
      else if (c == '\r')
      {
        column = 0;
        wrap = false;
      }
      else
      {
        assert(c >= 32);
        if (wrap)
        {
          ++row;
          column = 0;
          wrap = false;
        }
        assert(row < cells.size() && column < columns);
        cells[row][column] = c;
        if (++column == columns)
        {
          column = columns - 1;
          wrap = true;
        }
      }
    }
  }
};

int
main() {
  unsigned total_bytes = 0;
  for (unsigned columns : {8U, 12U, 80U})
  {
    std::string prompt = "root:/$ ", text;
    shell::LineDisplay display;
    Screen screen(columns);
    screen.feed(prompt);
    display.reset(prompt.size(), columns);
    unsigned cursor = 0, maximum = 0, random = 37;
    for (unsigned step = 0; step < 4000; ++step)
    {
      random = random * 1664525 + 1013904223;
      switch (random % 5)
      {
      case 0:
        if (text.size() < 180)
        {
          text.insert(cursor, 1, 'a' + (step % 26));
          ++cursor;
        }
        break;
      case 1:
        if (cursor)
        {
          text.erase(--cursor, 1);
        }
        break;
      case 2:
        cursor = text.empty() ? 0 : random % (text.size() + 1);
        break;
      case 3:
        cursor = text.size();
        break;
      case 4:
        if (cursor < text.size())
        {
          text[cursor] = 'A' + step % 26;
        }
        break;
      }
      std::string bytes;
      display.render(text.data(), text.size(), cursor, [&](const char *p, unsigned n) {
        bytes.append(p, n);
      });
      total_bytes += bytes.size();
      assert(bytes.find("\033[J") == std::string::npos);
      assert(bytes.find("\033[2J") == std::string::npos);
      screen.feed(bytes);
      if (text.size() > maximum)
      {
        maximum = text.size();
      }
      std::string expected = prompt + text + std::string(maximum - text.size(), ' ');
      for (unsigned i = 0; i < expected.size(); ++i)
      {
        assert(screen.cells[i / columns][i % columns] == expected[i]);
      }
      unsigned position = prompt.size() + cursor;
      bool wrap = cursor == text.size() && position % columns == 0;
      assert(screen.wrap == wrap);
      assert(screen.row == (wrap ? position - 1 : position) / columns);
      assert(screen.column == (wrap ? columns - 1 : position % columns));
    }
  }
  shell::LineDisplay display;
  display.reset(8, 80);
  std::string text(100, 'x'), bytes;
  auto emit = [&](const char *p, unsigned n) {
    bytes.append(p, n);
  };
  display.render(text.data(), text.size(), text.size(), emit);
  bytes.clear();
  display.render(text.data(), text.size(), 50, emit);
  assert(bytes.size() < 20 && bytes.find('x') == std::string::npos);
  Screen notice(80);
  notice.feed("root:/$ ");
  shell::LineDisplay notifying;
  notifying.reset(8, 80);
  std::string transaction;
  auto capture = [&](const char *p, unsigned n) {
    transaction.append(p, n);
  };
  notifying.render(text.data(), text.size(), 50, capture);
  notice.feed(transaction);
  notice.cells[3][0] = '!';
  transaction.clear();
  notifying.erase(capture);
  notice.feed(transaction);
  for (unsigned i = 0; i < 108; ++i)
  {
    assert(notice.cells[i / 80][i % 80] == ' ');
  }
  assert(notice.cells[3][0] == '!' && notice.row == 0 && notice.column == 0);
  std::printf(
      "serial line: 12000 edits, wrap boundaries, cursor state and owned-cell clearing PASS; cursor move=%zu bytes; edit stream=%u bytes\n",
      bytes.size(), total_bytes);
}
