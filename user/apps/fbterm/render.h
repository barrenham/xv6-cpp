#pragma once
#include "terminal.h"

namespace fbterm
{
// Cache the common shell colors once instead of blending the same font edges
// again for every character of every scrolling frame (about 75 KiB).
inline const unsigned *default_glyph(unsigned char character)
{
  static unsigned glyphs[95][cell_width * cell_height]{};
  static bool ready = false;
  if (!ready)
  {
    for (unsigned i = 0; i < 95; ++i)
    {
      Cell cell{static_cast<unsigned char>(i + 32), 7, 0};
      for (unsigned gy = 0; gy < cell_height; ++gy)
      {
        for (unsigned gx = 0; gx < cell_width; ++gx)
        {
          glyphs[i][gy * cell_width + gx] = Terminal::cell_pixel(cell, gx, gy, false);
        }
      }
    }
    ready = true;
  }
  return glyphs[character - 32];
}

// One present per update: scrolls cannot expose independently flushed strips.
// The caller supplies storage for width*height pixels, outside the user stack.
template <class Submit>
bool paint(Terminal &terminal, unsigned width, unsigned height, unsigned *pixels,
           Submit submit)
{
  unsigned first = terminal.rows;
  unsigned end = 0;
  for (unsigned row = 0; row < terminal.rows; ++row)
  {
    if (terminal.dirty[row])
    {
      if (first == terminal.rows)
      {
        first = row;
      }
      end = row + 1;
    }
  }
  if (first == terminal.rows)
  {
    return true;
  }
  unsigned top = first * cell_height;
  unsigned bottom = end == terminal.rows ? height : end * cell_height;
  // Blank cells dominate terminal frames. Fill once, then rasterize only visible
  // glyphs, colored backgrounds and the cursor instead of shading every pixel.
  for (unsigned i = 0; i < width * (bottom - top); ++i)
  {
    pixels[i] = palette[0];
  }
  for (unsigned row = first; row < end; ++row)
  {
    for (unsigned column = 0; column < terminal.columns; ++column)
    {
      const Cell &cell = terminal.cells[row][column];
      bool cursor =
          terminal.cursor_visible && row == terminal.y && column == terminal.x;
      if (cell.character == ' ' && cell.background == 0 && !cursor)
      {
        continue;
      }
      const unsigned *glyph = cell.foreground == 7 && cell.background == 0
                                  ? default_glyph(cell.character)
                                  : nullptr;
      for (unsigned gy = 0; gy < cell_height; ++gy)
      {
        for (unsigned gx = 0; gx < cell_width; ++gx)
        {
          pixels[((row - first) * cell_height + gy) * width + column * cell_width +
                 gx] = glyph && !(cursor && gy >= cell_height - 2)
                           ? glyph[gy * cell_width + gx]
                           : Terminal::cell_pixel(cell, gx, gy, cursor);
        }
      }
    }
  }
  if (!submit(top, bottom - top, pixels))
  {
    return false;
  }
  for (unsigned row = first; row < end; ++row)
  {
    terminal.dirty[row] = false;
  }
  terminal.scrolled = false;
  return true;
}
} // namespace fbterm
