#pragma once
#include "terminal.h"

namespace fbterm
{
// The wire image contains visible cells, never pointers or framebuffer pixels.
// One immutable image may be in flight; subsequent output stays in Terminal.
struct Snapshot
{
  Cell cells[max_rows][max_columns];
  unsigned x, y;
  bool cursor;
  unsigned long long origin, generation;

  struct Header
  {
    unsigned long long bytes, origin, generation;
    unsigned x, y, cursor;
  };

  static constexpr unsigned wire_capacity =
      sizeof(Header) + max_rows * (2 + max_columns * 4);

  unsigned encode(unsigned char *wire, unsigned rows, unsigned columns) const
  {
    unsigned used = sizeof(Header);
    for (unsigned row = 0; row < rows; ++row)
    {
      unsigned extent = columns;
      while (extent && cells[row][extent - 1].character == ' ' &&
             cells[row][extent - 1].background == 0 &&
             cells[row][extent - 1].foreground == 7)
      {
        --extent;
      }
      wire[used++] = extent & 255;
      wire[used++] = extent >> 8;
      unsigned foreground = 7, background = 0;
      for (unsigned column = 0; column < extent; ++column)
      {
        const auto &cell = cells[row][column];
        if (cell.foreground != foreground || cell.background != background)
        {
          wire[used++] = 0;
          wire[used++] = foreground = cell.foreground;
          wire[used++] = background = cell.background;
        }
        wire[used++] = cell.character;
      }
    }
    Header header{};
    header.bytes = used;
    header.origin = origin;
    header.generation = generation;
    header.x = x;
    header.y = y;
    header.cursor = cursor;
    for (unsigned i = 0; i < sizeof(header); ++i)
    {
      wire[i] = reinterpret_cast<const unsigned char *>(&header)[i];
    }
    return used;
  }

  bool decode(const unsigned char *wire, unsigned size, unsigned rows, unsigned columns)
  {
    if (size < sizeof(Header) || size > wire_capacity)
    {
      return false;
    }
    Header header{};
    for (unsigned i = 0; i < sizeof(header); ++i)
    {
      reinterpret_cast<unsigned char *>(&header)[i] = wire[i];
    }
    if (header.bytes != size || header.x >= columns || header.y >= rows ||
        header.cursor > 1)
    {
      return false;
    }
    x = header.x;
    y = header.y;
    cursor = header.cursor;
    origin = header.origin;
    generation = header.generation;
    unsigned used = sizeof(Header);
    for (unsigned row = 0; row < rows; ++row)
    {
      if (used + 2 > size)
      {
        return false;
      }
      unsigned extent = wire[used] | (unsigned(wire[used + 1]) << 8);
      used += 2;
      if (extent > columns)
      {
        return false;
      }
      unsigned char foreground = 7, background = 0;
      for (unsigned column = 0; column < columns; ++column)
      {
        Cell cell{' ', 7, 0};
        if (column < extent)
        {
          if (used >= size)
          {
            return false;
          }
          if (!wire[used])
          {
            if (used + 3 >= size || wire[used + 1] > 15 || wire[used + 2] > 15)
            {
              return false;
            }
            foreground = wire[used + 1];
            background = wire[used + 2];
            used += 3;
          }
          unsigned char character = wire[used++];
          if (character < 32 || character > 126)
          {
            return false;
          }
          cell = {character, foreground, background};
        }
        cells[row][column] = cell;
      }
    }
    return used == size;
  }

  void capture(const Terminal &terminal)
  {
    for (unsigned row = 0; row < terminal.rows; ++row)
    {
      for (unsigned column = 0; column < terminal.columns; ++column)
      {
        cells[row][column] = terminal.display_cell(row, column);
      }
    }
    x = terminal.x;
    y = terminal.y;
    cursor = !terminal.view_offset && terminal.cursor_visible;
    origin = terminal.scroll_position - terminal.view_offset;
    generation = terminal.generation;
  }

  void restore(Terminal &terminal) const
  {
    for (unsigned row = 0; row < terminal.rows; ++row)
    {
      for (unsigned column = 0; column < terminal.columns; ++column)
      {
        terminal.cells[row][column] = cells[row][column];
      }
      terminal.dirty[row] = true;
    }
    terminal.history_count = terminal.view_offset = 0;
    terminal.x = x;
    terminal.y = y;
    terminal.cursor_visible = cursor;
    terminal.scroll_position = origin;
    terminal.generation = generation;
  }
};
} // namespace fbterm
