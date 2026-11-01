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
    pixels[top * width + i] = palette[0];
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
          pixels[(row * cell_height + gy) * width + column * cell_width + gx] =
              glyph && !(cursor && gy >= cell_height - 2)
                  ? glyph[gy * cell_width + gx]
                  : Terminal::cell_pixel(cell, gx, gy, cursor);
        }
      }
    }
  }
  if (!submit(top, bottom - top, pixels + top * width))
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
  bool full_damage = false;
  unsigned backing_height = 0, origin_y = 0;
  unsigned long long viewport_reuses = 0, viewport_resets = 0;
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
    unsigned old_origin_y = origin_y;
    bool viewport = backing_height >= height * 2;
    if (viewport)
    {
      long long next = static_cast<long long>(origin_y) + delta * cell_height;
      if (!valid || next < 0 || next > backing_height - height)
      {
        if (valid)
        {
          ++viewport_resets;
        }
        origin_y = 0;
        valid = false;
      }
      else
      {
        origin_y = next;
        if (delta)
        {
          ++viewport_reuses;
        }
      }
    }
    bool initialize_backing = viewport && !backing_ready_;
    if (initialize_backing)
    {
      for (unsigned i = 0; i < width * backing_height; ++i)
      {
        pixels[i] = palette[0];
      }
    }
    // Keep a conservative bound over every physical row ever written into
    // this resource. It includes offscreen tails even after a viewport wrap.
    // Outside this extent the initial host-side background remains unchanged.
    unsigned extent = initialize_backing ? 0 : resource_extent_;
    if (viewport || (valid && !full_damage))
    {
      for (unsigned row = 0; row < terminal.rows; ++row)
      {
        for (unsigned column = 0; column < terminal.columns; ++column)
        {
          const auto &a = terminal.display_cell(row, column);
          bool occupied = a.character != ' ' || a.background ||
                          terminal.display_cursor(row, column);
          if (!viewport)
          {
            const auto &b = previous_[row][column];
            occupied = occupied || b.character != ' ' || b.background ||
                       cursor_[row] == static_cast<int>(column);
          }
          if (occupied && (column + 1) * cell_width > extent)
          {
            extent = (column + 1) * cell_width;
          }
        }
      }
    }
    damage_width = full_damage || initialize_backing || (!viewport && !valid) ? width
                   : extent                                                   ? extent
                                                                              : 1;
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
      if (match && (!delta || viewport))
      {
        if (delta)
        {
          ++reused_rows;
        }
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
      unsigned *destination = pixels + (origin_y + row * cell_height) * width;
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
    unsigned padding = terminal.rows * cell_height;
    if ((!valid || old_origin_y != origin_y) && padding < height)
    {
      for (unsigned i = padding * width; i < height * width; ++i)
      {
        pixels[origin_y * width + i] = palette[0];
      }
      if (first == terminal.rows)
      {
        first = terminal.rows - 1;
      }
      end = terminal.rows;
    }
    bool changed = first != terminal.rows;
    if ((changed || old_origin_y != origin_y) &&
        !submit(changed ? first * cell_height : 0,
                initialize_backing ? backing_height
                : changed ? (end == terminal.rows ? height - first * cell_height
                                                  : (end - first) * cell_height)
                          : 0,
                pixels + (origin_y + (changed ? first * cell_height : 0)) * width))
    {
      valid_ = false;
      backing_ready_ = false;
      resource_extent_ = 0;
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
    backing_ready_ = backing_ready_ || viewport;
    resource_extent_ = viewport ? extent : 0;
    return true;
  }

private:

  Cell previous_[max_rows][max_columns]{};
  int cursor_[max_rows]{};
  unsigned width_ = 0, height_ = 0, resource_extent_ = 0;
  unsigned long long generation_ = 0;
  long long origin_ = 0;
  bool valid_ = false, backing_ready_ = false;
};
} // namespace fbterm
