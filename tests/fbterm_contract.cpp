#include "user/apps/fbterm/terminal.h"
#include "user/apps/fbterm/keyboard.h"
#include "user/apps/fbterm/render.h"
#include "user/apps/fbterm/pacing.h"
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <random>

static fbterm::Terminal terminal{};

static void feed(const char *s)
{
  terminal.feed(s, std::strlen(s));
}

static void expect(unsigned row, const char *s)
{
  for (unsigned x = 0; s[x]; ++x)
  {
    if (terminal.cells[row][x].character != s[x])
    {
      std::fprintf(stderr, "row %u col %u expected %s got %c\n", row, x, s,
                   terminal.cells[row][x].character);
      std::abort();
    }
  }
}

int main()
{
  // Deterministic burst: 1,000 output packets/s must not cause 60 full paints/s.
  using Pacer = fbterm::FramePacer;
  constexpr auto ms = Pacer::Millisecond;
  Pacer pacer;
  assert(pacer.timeout(0) == -1);
  pacer.output(0, 1, false);
  assert(pacer.timeout(0) == 16 && pacer.timeout(16 * ms) == 0);
  pacer.complete(16 * ms, 17 * ms);
  unsigned frames = 0;
  for (unsigned time = 20; time < 1020; ++time)
  {
    pacer.output(time * ms, 1024, true);
    if (pacer.timeout(time * ms) == 0)
    {
      ++frames;
      pacer.complete(time * ms, (time + 1) * ms);
    }
  }
  assert(frames == 9);
  // A pause publishes the tail; an endless stream cannot move its deadline.
  assert(pacer.timeout(1039 * ms) == 0);
  pacer.complete(1039 * ms, 1040 * ms);
  assert(pacer.timeout(1040 * ms) == -1);
  Pacer slow;
  slow.output(0, 4096, false);
  assert(slow.timeout(20 * ms) == 0);
  slow.complete(20 * ms, 120 * ms);
  slow.output(121 * ms, 1, true);
  assert(slow.timeout(140 * ms) == 230);
  assert(slow.timeout(370 * ms) == 0);
  // Chunky serial backpressure must not turn the quiet timer into 50 Hz scrolls.
  Pacer chunky;
  chunky.output(0, 10, true);
  chunky.complete(20 * ms, 21 * ms);
  chunky.output(30 * ms, 10, true);
  assert(chunky.timeout(50 * ms) == 71);
  Pacer volume;
  volume.output(0, 2000, false);
  volume.output(ms, 2100, false);
  assert(volume.timeout(ms) == 20);
  // Saturation remains safe even for extremely large read sizes.
  volume.output(2 * ms, ~0u, false);
  assert(volume.timeout(2 * ms) == 20);
  fbterm::Keyboard keyboard{};
  char converted[8];
  auto key = [&](unsigned short code, unsigned value)
  { return keyboard.translate({1, code, value}, converted); };
  assert(key(30, 1) == 1 && converted[0] == 'a');
  key(42, 1);
  key(54, 1);
  key(42, 0);
  assert(key(30, 2) == 1 && converted[0] == 'A');
  key(54, 0);
  key(58, 1);
  key(58, 2);
  assert(key(30, 1) == 1 && converted[0] == 'A');
  key(42, 1);
  assert(key(30, 1) == 1 && converted[0] == 'a');
  key(42, 0);
  key(29, 1);
  assert(key(46, 1) == 1 && converted[0] == 3);
  assert(key(27, 1) == 1 && converted[0] == 29);
  keyboard.translate({XV_KEY_SYN, XV_KEY_SYN_DROPPED, 0}, converted);
  assert(key(30, 1) == 1 && converted[0] == 'a');
  assert(key(103, 1) == 3 && !std::memcmp(converted, "\033[A", 3));
  assert(key(14, 1) == 1 && converted[0] == 127);
  assert(key(30, 0) == 0 && key(127, 1) == 0 && key(30, 3) == 0);
  terminal.reset();
  feed("A\r\nB\tC\bD");
  expect(0, "A");
  expect(1, "B       D");
  // Real font glyph proportions and grayscale edges, rather than stretched 5x7 rows.
  bool partial = false;
  unsigned left = fbterm::cell_width, right = 0, top = fbterm::cell_height, bottom = 0;
  for (unsigned y = 0; y < fbterm::cell_height; ++y)
  {
    for (unsigned x = 0; x < fbterm::cell_width; ++x)
    {
      unsigned alpha = fbterm::font['A' - 32][y * fbterm::cell_width + x];
      if (alpha)
      {
        if (x < left)
        {
          left = x;
        }
        if (x > right)
        {
          right = x;
        }
        if (y < top)
        {
          top = y;
        }
        if (y > bottom)
        {
          bottom = y;
        }
      }
      if (alpha > 0 && alpha < 15)
      {
        partial = true;
        unsigned shade = terminal.pixel(x, y);
        for (unsigned shift = 0; shift < 24; shift += 8)
        {
          unsigned channel = (shade >> shift) & 255;
          assert(channel > ((fbterm::palette[0] >> shift) & 255));
          assert(channel < ((fbterm::palette[7] >> shift) & 255));
        }
      }
    }
  }
  assert(partial && right - left + 1 >= 8 && bottom - top + 1 <= 14);
  assert(terminal.pixel(0, 0) == fbterm::palette[0]);
  terminal.reset();
  char full[81];
  std::memset(full, 'X', 80);
  full[80] = 0;
  feed(full);
  assert(terminal.x == 79 && terminal.y == 0);
  feed("\r\nY");
  expect(1, "Y");
  assert(terminal.y == 1);
  terminal.reset();
  feed(full);
  feed("YZ");
  expect(1, "YZ");
  assert(terminal.y == 1 && terminal.x == 2);
  terminal.reset();
  feed(full);
  feed("\033[32mYZ");
  expect(1, "YZ");
  assert(terminal.y == 1);
  // Cursor relocation must repaint both the old underline and its new row.
  for (auto &row : terminal.dirty)
  {
    row = false;
  }
  feed("\033[5;1H");
  assert(terminal.dirty[1] && terminal.dirty[4]);
  terminal.reset();
  for (unsigned i = 0; i < 35; ++i)
  {
    char line[] = "line 00\r\n";
    line[5] = '0' + i / 10;
    line[6] = '0' + i % 10;
    feed(line);
  }
  expect(0, "line 06");
  expect(28, "line 34");
  assert(terminal.y == 29);
  // Real shell editor redraw, CSI split between reads, erase, and clear.
  feed("draft\r\033[");
  feed("2Kroot$ edited\b\b");
  expect(29, "root$ edited");
  assert(terminal.x == 10);
  feed("\033[H\033[2J\033[3J");
  assert(terminal.x == 0 && terminal.y == 0);
  for (const auto &row : terminal.cells)
  {
    for (auto cell : row)
    {
      assert(cell.character == ' ');
    }
  }
  feed("\033[31;44mR\033[0mN\033[1;32mB\033[7mI\033[0m");
  assert(terminal.cells[0][0].foreground == 1 && terminal.cells[0][0].background == 4);
  assert(terminal.cells[0][1].foreground == 7 && terminal.cells[0][1].background == 0);
  assert(terminal.cells[0][2].foreground == 10);
  assert(terminal.cells[0][3].foreground == 0 && terminal.cells[0][3].background == 10);
  feed("\033[?25l");
  assert(!terminal.cursor_visible);
  feed("\033[?25h");
  assert(terminal.cursor_visible);
  feed("\033[999999;999999H");
  assert(terminal.x == 79 && terminal.y == 29);
  feed("\033[99999A\033[99999D");
  assert(terminal.x == 0 && terminal.y == 0);
  feed("\033]title\033\\Z");
  expect(0, "Z");
  feed("\033[2;3H\0337\033[H\0338");
  assert(terminal.x == 2 && terminal.y == 1);
  // A hostile byte stream must never move raster indices outside fixed arrays.
  std::mt19937 random(42);
  for (unsigned i = 0; i < 200000; ++i)
  {
    char value = random();
    terminal.feed(&value, 1);
    assert(terminal.x < 80 && terminal.y < 30);
    for (const auto &row : terminal.cells)
    {
      for (auto cell : row)
      {
        assert(cell.character >= 32 && cell.character <= 126 && cell.foreground < 16 &&
               cell.background < 16);
      }
    }
    (void)terminal.pixel(random() % 640, random() % 480);
  }
  assert(!terminal.configure(0, 0) && !terminal.configure(1928, 1080));
  assert(terminal.configure(1280, 800) && terminal.columns == 128 &&
         terminal.rows == 40);
  feed("\033[40;128HX");
  assert(terminal.cells[39][127].character == 'X');
  feed("Y");
  assert(terminal.cells[39][0].character == 'Y');
  assert(terminal.configure(1920, 1080) && terminal.columns == 192 &&
         terminal.rows == 54);
  terminal.cursor_visible = false;
  assert(terminal.pixel(1919, 1079) == fbterm::palette[0]);
  feed("\033[54;192HZ");
  assert(terminal.cells[53][191].character == 'Z');
  assert(terminal.configure(1366, 768) && terminal.columns == 136 &&
         terminal.rows == 38);
  assert(terminal.pixel(1365, 10) == fbterm::palette[0]);
  assert(terminal.configure(640, 480) && terminal.columns == 64 && terminal.rows == 24);
  // Exact-boundary input must continue across multiple complete rows.
  assert(terminal.configure(800, 600));
  char lines[247];
  std::memset(lines, 'a', sizeof(lines));
  terminal.feed(lines, sizeof(lines));
  assert(terminal.y == 3 && terminal.x == 7);
  for (unsigned row = 0; row < 3; ++row)
  {
    for (unsigned col = 0; col < 80; ++col)
    {
      assert(terminal.cells[row][col].character == 'a');
    }
  }
  // Cached shell glyphs must preserve every grayscale pixel of the reference.
  for (unsigned character = 32; character < 127; ++character)
  {
    fbterm::Cell cell{static_cast<unsigned char>(character), 7, 0};
    const unsigned *glyph = fbterm::default_glyph(cell.character);
    for (unsigned gy = 0; gy < fbterm::cell_height; ++gy)
    {
      for (unsigned gx = 0; gx < fbterm::cell_width; ++gx)
      {
        assert(glyph[gy * fbterm::cell_width + gx] ==
               fbterm::Terminal::cell_pixel(cell, gx, gy, false));
      }
    }
  }
  static unsigned frame[1920 * 1080];
  assert(terminal.configure(1920, 1080));
  unsigned presents = 0;
  auto submit = [&](unsigned y, unsigned height, unsigned *)
  {
    ++presents;
    assert(y == 0 && height == 1080);
    return true;
  };
  assert(fbterm::paint(terminal, 1920, 1080, frame, submit) && presents == 1);
  assert(fbterm::paint(terminal, 1920, 1080, frame, submit) && presents == 1);
  for (unsigned i = 0; i < 70; ++i)
  {
    feed("continuous scrolling\r\n");
  }
  assert(terminal.scrolled);
  assert(!fbterm::paint(terminal, 1920, 1080, frame,
                        [](unsigned, unsigned, unsigned *) { return false; }));
  assert(terminal.scrolled);
  assert(fbterm::paint(terminal, 1920, 1080, frame, submit) && presents == 2);
  assert(!terminal.scrolled);
  feed("retry");
  assert(!fbterm::paint(terminal, 1920, 1080, frame,
                        [](unsigned, unsigned, unsigned *) { return false; }));
  assert(terminal.dirty[terminal.y]);
  assert(terminal.configure(1366, 768));
  feed("\033[44m A\033[0m\033[32;7mI\033[0m\033[38;136HB");
  assert(fbterm::paint(terminal, 1366, 768, frame,
                       [](unsigned top, unsigned height, unsigned *)
                       { return top == 0 && height == 768; }));
  for (unsigned y = 0; y < 768; ++y)
  {
    for (unsigned x = 0; x < 1366; ++x)
    {
      assert(frame[y * 1366 + x] == terminal.pixel(x, y));
    }
  }
  std::puts(
      "fbterm contract: text, raster, scrolling, editor, ANSI and adversarial input passed");
}
