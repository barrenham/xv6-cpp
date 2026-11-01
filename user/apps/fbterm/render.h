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
      const Cell &cell = terminal.display_cell(row, column);
      bool cursor = terminal.display_cursor(row, column);
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

// Owns row snapshots, while the caller owns the persistent full-screen pixels.
// Matching old rows move as pixels; only changed/new rows need rasterization.
class Renderer
{
public:

  unsigned long long raster_rows = 0, reused_rows = 0;
  unsigned damage_width = 0;
  bool trim = true;
  unsigned long long move_ticks = 0;
  unsigned long (*clock)() = nullptr;

  template <class Submit>
  bool paint(Terminal &terminal, unsigned width, unsigned height, unsigned *pixels,
             Submit submit)
  {
    bool valid = valid_ && width == width_ && height == height_ &&
                 terminal.generation == generation_;
    long long origin =
        static_cast<long long>(terminal.scroll_position) - terminal.view_offset;
    long long delta = valid ? origin - origin_ : 0;
    // Outside this extent every old and new row is the default background.
    // Include old content too: shortening a line must erase its former tail.
    damage_width = valid && trim ? 0 : width;
    if (valid && trim)
    {
      for (unsigned row = 0; row < terminal.rows; ++row)
      {
        for (unsigned column = 0; column < terminal.columns; ++column)
        {
          const auto &a = terminal.display_cell(row, column);
          const auto &b = previous_[row][column];
          if (a.character != ' ' || a.background || b.character != ' ' ||
              b.background || terminal.display_cursor(row, column) ||
              cursor_[row] == static_cast<int>(column))
          {
            if ((column + 1) * cell_width > damage_width)
            {
              damage_width = (column + 1) * cell_width;
            }
          }
        }
      }
    }
    // A blank-to-blank shift still has a valid, harmless one-pixel rectangle.
    if (!damage_width)
    {
      damage_width = 1;
    }
    unsigned first = terminal.rows, end = 0;
    for (unsigned step = 0; step < terminal.rows; ++step)
    {
      unsigned row = delta < 0 ? terminal.rows - 1 - step : step;
      long long old = static_cast<long long>(row) + delta;
      bool match = valid && old >= 0 && old < terminal.rows;
      int cursor = !terminal.view_offset && terminal.cursor_visible && row == terminal.y
                       ? static_cast<int>(terminal.x)
                       : -1;
      if (match)
      {
        match = cursor == cursor_[old];
        for (unsigned column = 0; match && column < terminal.columns; ++column)
        {
          const auto &a = terminal.display_cell(row, column);
          const auto &b = previous_[old][column];
          match = a.character == b.character && a.foreground == b.foreground &&
                  a.background == b.background;
        }
      }
      if (match && !delta)
      {
        continue;
      }
      if (row < first)
      {
        first = row;
      }
      if (row + 1 > end)
      {
        end = row + 1;
      }
      unsigned *destination = pixels + row * cell_height * width;
      if (match)
      {
        const unsigned *source = pixels + old * cell_height * width;
        // Different character rows do not overlap; row order protects later sources.
        auto begin = clock ? clock() : 0;
        for (unsigned y = 0; y < cell_height; ++y)
        {
          for (unsigned x = 0; x < damage_width; ++x)
          {
            destination[y * width + x] = source[y * width + x];
          }
        }
        move_ticks += clock ? clock() - begin : 0;
        ++reused_rows;
        continue;
      }
      ++raster_rows;
      for (unsigned i = 0; i < cell_height * width; ++i)
      {
        destination[i] = palette[0];
      }
      for (unsigned column = 0; column < terminal.columns; ++column)
      {
        const Cell &cell = terminal.display_cell(row, column);
        bool underlined = cursor == static_cast<int>(column);
        if (cell.character == ' ' && cell.background == 0 && !underlined)
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
            destination[gy * width + column * cell_width + gx] =
                glyph && !(underlined && gy >= cell_height - 2)
                    ? glyph[gy * cell_width + gx]
                    : Terminal::cell_pixel(cell, gx, gy, underlined);
          }
        }
      }
    }
    if (!valid)
    {
      for (unsigned i = terminal.rows * cell_height * width; i < width * height; ++i)
      {
        pixels[i] = palette[0];
      }
    }
    if (first != terminal.rows &&
        !submit(first * cell_height,
                end == terminal.rows ? height - first * cell_height
                                     : (end - first) * cell_height,
                pixels + first * cell_height * width))
    {
      valid_ = false;
      return false;
    }
    for (unsigned row = 0; row < terminal.rows; ++row)
    {
      for (unsigned column = 0; column < terminal.columns; ++column)
      {
        previous_[row][column] = terminal.display_cell(row, column);
      }
      cursor_[row] =
          !terminal.view_offset && terminal.cursor_visible && row == terminal.y
              ? static_cast<int>(terminal.x)
              : -1;
      terminal.dirty[row] = false;
    }
    terminal.scrolled = false;
    width_ = width;
    height_ = height;
    generation_ = terminal.generation;
    origin_ = origin;
    valid_ = true;
    return true;
  }

private:

  Cell previous_[max_rows][max_columns]{};
  int cursor_[max_rows]{};
  unsigned width_ = 0, height_ = 0;
  unsigned long long generation_ = 0;
  long long origin_ = 0;
  bool valid_ = false;
};
} // namespace fbterm
