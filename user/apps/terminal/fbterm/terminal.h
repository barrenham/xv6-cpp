#pragma once
#include "font.h"

namespace fbterm
{
inline constexpr unsigned cell_width = font_width, cell_height = font_height;
inline constexpr unsigned max_width = 1920, max_height = 1080;
inline constexpr unsigned max_columns = max_width / cell_width;
inline constexpr unsigned max_rows = max_height / cell_height;
inline constexpr unsigned palette[] = {
    0x101820, 0xcd5555, 0x55aa55, 0xcdad55, 0x5588cc, 0xaa55aa, 0x55aaaa, 0xd8dee9,
    0x667080, 0xff7777, 0x88dd88, 0xffdd88, 0x88bbff, 0xdd88dd, 0x88dddd, 0xffffff};

struct Cell
{
  unsigned char character, foreground, background;
};

class Terminal
{
public:

  Cell cells[max_rows][max_columns]{};
  unsigned columns = 80, rows = 30;
  bool dirty[max_rows]{};
  unsigned x = 0, y = 0;
  bool cursor_visible = true;
  bool scrolled = false;
  static constexpr unsigned history_capacity = 1024;
  unsigned history_count = 0, view_offset = 0;
  unsigned long long scroll_position = 0, generation = 0;

  void set_history(Cell (*storage)[max_columns])
  {
    history_ = storage;
    reset();
  }

  const Cell &display_cell(unsigned row, unsigned column) const
  {
    unsigned line = history_count + row - view_offset;
    return line < history_count
               ? history_[(history_head_ + line) % history_capacity][column]
               : cells[line - history_count][column];
  }

  bool display_cursor(unsigned row, unsigned column) const
  {
    return !view_offset && cursor_visible && row == y && column == x;
  }

  void scroll_history(int lines)
  {
    long long next = static_cast<long long>(view_offset) + lines;
    view_offset = next < 0 ? 0 : next > history_count ? history_count : next;
    for (auto &row : dirty)
    {
      row = true;
    }
  }

  void live()
  {
    if (view_offset)
    {
      scroll_history(-static_cast<int>(view_offset));
    }
  }

  bool configure(unsigned width, unsigned height)
  {
    if (width < 640 || height < 480 || width > max_width || height > max_height)
    {
      return false;
    }
    columns = width / cell_width;
    rows = height / cell_height;
    reset();
    return true;
  }

  void reset()
  {
    x = y = saved_x = saved_y = 0;
    foreground = 7;
    background = 0;
    bold = inverse = wrap = false;
    cursor_visible = true;
    scrolled = false;
    history_count = history_head_ = view_offset = 0;
    scroll_position = 0;
    ++generation;
    state = Normal;
    for (auto &row : cells)
    {
      for (auto &cell : row)
      {
        cell = blank();
      }
    }
    for (auto &row : dirty)
    {
      row = true;
    }
  }

  void feed(const char *data, unsigned size)
  {
    dirty[y] = true;
    for (unsigned i = 0; i < size; ++i)
    {
      put(static_cast<unsigned char>(data[i]));
    }
    dirty[y] = true;
  }

  unsigned pixel(unsigned px, unsigned py) const
  {
    if (px >= columns * cell_width || py >= rows * cell_height)
    {
      return palette[0];
    }
    const Cell &cell = display_cell(py / cell_height, px / cell_width);
    bool cursor = display_cursor(py / cell_height, px / cell_width);
    return cell_pixel(cell, px % cell_width, py % cell_height, cursor);
  }

  static unsigned cell_pixel(const Cell &cell, unsigned gx, unsigned gy, bool cursor)
  {
    unsigned coverage = font[cell.character - 32][gy * cell_width + gx];
    if (cursor && gy >= cell_height - 2)
    {
      coverage = 15 - coverage;
    }
    unsigned foreground = palette[cell.foreground];
    unsigned background = palette[cell.background];
    if (!coverage)
    {
      return background;
    }
    if (coverage == 15)
    {
      return foreground;
    }
    unsigned result = 0;
    for (unsigned shift = 0; shift < 24; shift += 8)
    {
      unsigned channel = (((foreground >> shift) & 255) * coverage +
                          ((background >> shift) & 255) * (15 - coverage) + 7) /
                         15;
      result |= channel << shift;
    }
    return result;
  }

private:

  Cell (*history_)[max_columns] = nullptr;
  unsigned history_head_ = 0;

  enum State
  {
    Normal,
    Escape,
    Csi,
    Osc,
    OscEscape
  } state = Normal;

  unsigned foreground = 7, background = 0, saved_x = 0, saved_y = 0;
  unsigned parameters[8]{}, count = 0;
  bool bold = false, inverse = false, wrap = false, private_mode = false,
       invalid = false;

  static unsigned minimum(unsigned a, unsigned b)
  {
    return a < b ? a : b;
  }

  Cell blank() const
  {
    return {' ', static_cast<unsigned char>(inverse ? background : foreground),
            static_cast<unsigned char>(inverse ? foreground : background)};
  }

  void clear(unsigned first, unsigned end)
  {
    for (unsigned i = first; i < end; ++i)
    {
      cells[i / columns][i % columns] = blank();
      dirty[i / columns] = true;
    }
  }

  void down()
  {
    if (++y == rows)
    {
      y = rows - 1;
      scrolled = true;
      if (history_)
      {
        unsigned slot = (history_head_ + history_count) % history_capacity;
        if (history_count == history_capacity)
        {
          history_head_ = (history_head_ + 1) % history_capacity;
        }
        else
        {
          ++history_count;
        }
        for (unsigned column = 0; column < columns; ++column)
        {
          history_[slot][column] = cells[0][column];
        }
      }
      ++scroll_position;
      if (view_offset)
      {
        view_offset = minimum(view_offset + 1, history_count);
      }
      for (unsigned r = 1; r < rows; ++r)
      {
        for (unsigned c = 0; c < columns; ++c)
        {
          cells[r - 1][c] = cells[r][c];
        }
      }
      clear((rows - 1) * columns, rows * columns);
      for (auto &row : dirty)
      {
        row = true;
      }
    }
  }

  unsigned argument(unsigned i, unsigned fallback = 1) const
  {
    return i <= count && parameters[i] ? parameters[i] : fallback;
  }

  void csi(unsigned char value)
  {
    unsigned n = argument(0);
    if (private_mode)
    {
      if (parameters[0] == 25 && (value == 'h' || value == 'l'))
      {
        cursor_visible = value == 'h';
      }
      return;
    }
    switch (value)
    {
    case 'A':
      y -= minimum(y, n);
      break;
    case 'B':
      y = minimum(rows - 1, y + n);
      break;
    case 'C':
      x = minimum(columns - 1, x + n);
      break;
    case 'D':
      x -= minimum(x, n);
      break;
    case 'G':
      x = minimum(columns, n) - 1;
      break;
    case 'd':
      y = minimum(rows, n) - 1;
      break;
    case 'H':
    case 'f':
      y = minimum(rows, n) - 1;
      x = minimum(columns, argument(1)) - 1;
      break;
    case 'J':
      if (parameters[0] == 3)
      {
        history_count = history_head_ = view_offset = 0;
        ++generation;
        for (auto &row : dirty)
        {
          row = true;
        }
      }
      if (parameters[0] == 0)
      {
        clear(y * columns + x, rows * columns);
      }
      if (parameters[0] == 1)
      {
        clear(0, y * columns + x + 1);
      }
      if (parameters[0] == 2)
      {
        clear(0, rows * columns);
      }
      break;
    case 'K':
      if (parameters[0] == 0)
      {
        clear(y * columns + x, (y + 1) * columns);
      }
      if (parameters[0] == 1)
      {
        clear(y * columns, y * columns + x + 1);
      }
      if (parameters[0] == 2)
      {
        clear(y * columns, (y + 1) * columns);
      }
      break;
    case 's':
      saved_x = x;
      saved_y = y;
      break;
    case 'u':
      x = saved_x;
      y = saved_y;
      break;
    case 'm':
      for (unsigned i = 0; i <= count; ++i)
      {
        unsigned p = parameters[i];
        if (p == 0)
        {
          foreground = 7;
          background = 0;
          bold = inverse = false;
        }
        else if (p == 1)
        {
          bold = true;
        }
        else if (p == 22)
        {
          bold = false;
        }
        else if (p == 7)
        {
          inverse = true;
        }
        else if (p == 27)
        {
          inverse = false;
        }
        else if (p >= 30 && p <= 37)
        {
          foreground = p - 30;
        }
        else if (p >= 40 && p <= 47)
        {
          background = p - 40;
        }
        else if (p >= 90 && p <= 97)
        {
          foreground = p - 90 + 8;
        }
        else if (p >= 100 && p <= 107)
        {
          background = p - 100 + 8;
        }
        else if (p == 39)
        {
          foreground = 7;
        }
        else if (p == 49)
        {
          background = 0;
        }
      }
      break;
    }
    if (value != 'm')
    {
      wrap = false;
    }
  }

  void put(unsigned char value)
  {
    dirty[y] = true;
    if (state == Osc || state == OscEscape)
    {
      if (value == 7 || (state == OscEscape && value == '\\'))
      {
        state = Normal;
      }
      else
      {
        state = value == 27 ? OscEscape : Osc;
      }
      return;
    }
    if (value == 27)
    {
      state = Escape;
      return;
    }
    if (state == Escape)
    {
      state = Normal;
      if (value == '[')
      {
        state = Csi;
        count = 0;
        private_mode = invalid = false;
        for (auto &p : parameters)
        {
          p = 0;
        }
      }
      else if (value == ']')
      {
        state = Osc;
      }
      else if (value == '7')
      {
        saved_x = x;
        saved_y = y;
      }
      else if (value == '8')
      {
        x = saved_x;
        y = saved_y;
        wrap = false;
      }
      else if (value == 'c')
      {
        reset();
      }
      dirty[y] = true;
      return;
    }
    if (state == Csi)
    {
      if (value >= '0' && value <= '9')
      {
        parameters[count] = minimum(9999, parameters[count] * 10 + value - '0');
      }
      else if (value == ';')
      {
        if (count < 7)
        {
          ++count;
        }
        else
        {
          invalid = true;
        }
      }
      else if (value == '?' && count == 0 && parameters[0] == 0)
      {
        private_mode = true;
      }
      else if (value >= 0x40 && value <= 0x7e)
      {
        if (!invalid)
        {
          csi(value);
        }
        state = Normal;
      }
      else
      {
        invalid = true;
      }
      dirty[y] = true;
      return;
    }
    if (value == '\r')
    {
      x = 0;
      wrap = false;
    }
    else if (value == '\n')
    {
      down();
      wrap = false;
    }
    else if (value == '\b')
    {
      if (x)
      {
        --x;
      }
      wrap = false;
    }
    else if (value == '\t')
    {
      x = minimum(columns - 1, (x / 8 + 1) * 8);
      wrap = false;
    }
    else if (value >= 32 && value != 127)
    {
      if (value >= 0x80 && value < 0xc0)
      {
        return; // UTF-8 continuation bytes.
      }
      if (wrap)
      {
        x = 0;
        down();
        wrap = false;
      }
      unsigned fg = foreground | (bold && foreground < 8 ? 8 : 0);
      cells[y][x] = {static_cast<unsigned char>(value < 127 ? value : '?'),
                     static_cast<unsigned char>(inverse ? background : fg),
                     static_cast<unsigned char>(inverse ? fg : background)};
      if (x == columns - 1)
      {
        wrap = true;
      }
      else
      {
        ++x;
      }
    }
    dirty[y] = true;
  }
};
} // namespace fbterm
