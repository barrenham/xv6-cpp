#include "user/apps/terminal/fbterm/terminal.h"
#include "user/apps/terminal/fbterm/keyboard.h"
#include "user/apps/terminal/fbterm/render.h"
#include "user/apps/terminal/fbterm/pacing.h"
#include "user/apps/terminal/fbterm/navigation.h"
#include "user/apps/terminal/fbterm/snapshot.h"
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
  Pacer asynchronous;
  asynchronous.output(0, 4096, true);
  asynchronous.complete(20 * ms, 21 * ms);
  asynchronous.output(22 * ms, 32, true);
  asynchronous.feedback(30 * ms, 10 * ms);
  assert(asynchronous.timeout(30 * ms) == 100);
  assert(asynchronous.timeout(130 * ms) == 0);
  Pacer whole;
  whole.output(0, 4096, true);
  whole.complete(20 * ms, 21 * ms);
  whole.output(22 * ms, 1, true);
  whole.feedback(30 * ms, 5 * ms, true);
  assert(whole.timeout(30 * ms) == 250);
  assert(whole.timeout(280 * ms) == 0);
  whole.complete(280 * ms, 281 * ms);
  whole.feedback(281 * ms, 5 * ms, true);
  whole.output(300 * ms, 1, false);
  assert(whole.timeout(300 * ms) == 16); // Typing does not inherit bulk throttling.
  fbterm::Navigation navigation;
  char forwarded[4];
  int page = 0;
  assert(navigation.feed(27, 0, forwarded, page) == 0);
  assert(navigation.timeout(0) == 30);
  assert(navigation.feed('[', 1, forwarded, page) == 0);
  assert(navigation.feed('5', 2, forwarded, page) == 0);
  assert(navigation.feed('~', 3, forwarded, page) == 0 && page == 1);
  assert(navigation.timeout(4) == -1);
  assert(navigation.feed(27, 5, forwarded, page) == 0);
  assert(navigation.feed('[', 6, forwarded, page) == 0);
  assert(navigation.feed('A', 7, forwarded, page) == 3 && !page);
  assert(!std::memcmp(forwarded, "\033[A", 3));
  navigation.feed(27, 0, forwarded, page);
  assert(navigation.timeout(300000) == 0);
  assert(navigation.flush(forwarded) == 1 && forwarded[0] == 27);
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
  // Horizontal tabs advance to the next 8-column stop regardless of the
  // preceding field length, and clamp at the final column without scrolling.
  for (unsigned start = 0; start < 24; ++start)
  {
    terminal.reset();
    for (unsigned i = 0; i < start; ++i) feed("x");
    feed("\tZ");
    unsigned stop = (start / 8 + 1) * 8;
    assert(terminal.cells[0][stop].character == 'Z');
  }
  terminal.reset();
  feed("processor\t: 0\r\nhart\t\t: 0\r\nisa\t\t: rv64");
  for (unsigned row = 0; row < 3; ++row) assert(terminal.cells[row][16].character == ':');
  terminal.reset();
  terminal.x = terminal.columns - 2;
  feed("\tZ");
  assert(terminal.y == 0 && terminal.cells[0][terminal.columns - 1].character == 'Z');
  terminal.reset();
  feed("A\r\nB\tC\bD");
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
  feed("\033[2;1Hpartial reference update");
  assert(fbterm::paint(terminal, 1366, 768, frame,
                       [&](unsigned top, unsigned height, unsigned *data)
                       {
                         assert(top && height && data == frame + top * 1366);
                         return true;
                       }));
  for (unsigned y = 0; y < 768; ++y)
  {
    for (unsigned x = 0; x < 1366; ++x)
    {
      assert(frame[y * 1366 + x] == terminal.pixel(x, y));
    }
  }
  // A persistent raster must match the per-pixel reference after scrolling,
  // viewport moves in both directions, edits, cursor motion and a failed draw.
  static fbterm::Cell history[fbterm::Terminal::history_capacity][fbterm::max_columns];
  static fbterm::Renderer cached;
  terminal.set_history(history);
  assert(terminal.configure(1366, 768));
  unsigned submissions = 0;
  static unsigned displayed[1366 * 768];
  auto cached_submit = [&](unsigned top, unsigned height, unsigned *data)
  {
    assert(top < 768 && height && top + height <= 768);
    assert(data == frame + top * 1366);
    assert(cached.damage_width && cached.damage_width <= 1366);
    for (unsigned y = 0; y < height; ++y)
    {
      std::memcpy(displayed + (top + y) * 1366, data + y * 1366,
                  cached.damage_width * sizeof(unsigned));
    }
    ++submissions;
    return true;
  };
  auto compare = [&]
  {
    assert(cached.paint(terminal, 1366, 768, frame, cached_submit));
    for (unsigned y = 0; y < 768; ++y)
    {
      for (unsigned x = 0; x < 1366; ++x)
      {
        assert(frame[y * 1366 + x] == terminal.pixel(x, y));
        assert(displayed[y * 1366 + x] == terminal.pixel(x, y));
      }
    }
  };
  for (unsigned i = 0; i < 60; ++i)
  {
    char line[64];
    std::snprintf(line, sizeof(line), "row-%03u: shaded text\r\n", i);
    feed(line);
  }
  compare();
  auto rows_before = cached.raster_rows;
  auto reused_before = cached.reused_rows;
  feed("next row\r\n");
  compare();
  assert(cached.raster_rows - rows_before <= 2);
  assert(cached.reused_rows - reused_before >= terminal.rows - 2);
  assert(cached.damage_width < 1366 / 2);
  terminal.scroll_history(15);
  compare();
  auto presentations = submissions;
  feed("output while reading history\r\n");
  compare();
  assert(submissions == presentations); // Frozen history needs no redraw.
  terminal.scroll_history(-7);
  compare();
  // A pending snapshot is immutable while the producer continues consuming.
  static fbterm::Snapshot snapshot;
  static fbterm::Terminal consumer;
  assert(consumer.configure(1366, 768));
  snapshot.capture(terminal);
  static unsigned char wire[fbterm::Snapshot::wire_capacity];
  unsigned wire_size = snapshot.encode(wire, terminal.rows, terminal.columns);
  assert(wire_size < sizeof(snapshot));
  static fbterm::Snapshot decoded;
  assert(decoded.decode(wire, wire_size, terminal.rows, terminal.columns));
  for (unsigned cut :
       {0U, 7U, unsigned(sizeof(fbterm::Snapshot::Header)), wire_size - 1})
  {
    assert(!decoded.decode(wire, cut, terminal.rows, terminal.columns));
  }
  assert(decoded.decode(wire, wire_size, terminal.rows, terminal.columns));
  decoded.restore(consumer);
  assert(!consumer.cursor_visible);
  for (unsigned row = 0; row < terminal.rows; ++row)
  {
    for (unsigned column = 0; column < terminal.columns; ++column)
    {
      assert(!std::memcmp(&consumer.cells[row][column],
                          &terminal.display_cell(row, column), sizeof(fbterm::Cell)));
    }
  }
  auto saved_cell = consumer.cells[0][0];
  feed("more output while a frame is in flight\r\n");
  snapshot.restore(consumer);
  assert(!std::memcmp(&saved_cell, &consumer.cells[0][0], sizeof(saved_cell)));
  assert(consumer.configure(1920, 1080));
  for (unsigned row = 0; row < consumer.rows; ++row)
  {
    for (unsigned column = 0; column < consumer.columns; ++column)
    {
      consumer.cells[row][column] = {static_cast<unsigned char>('A' + column % 26),
                                     static_cast<unsigned char>(1 + column % 2),
                                     static_cast<unsigned char>(row % 16)};
    }
  }
  snapshot.capture(consumer);
  wire_size = snapshot.encode(wire, consumer.rows, consumer.columns);
  assert(wire_size == sizeof(wire)); // Worst case: a color change at every cell.
  assert(decoded.decode(wire, wire_size, consumer.rows, consumer.columns));
  assert(!std::memcmp(decoded.cells, snapshot.cells, sizeof(snapshot.cells)));
  wire[sizeof(fbterm::Snapshot::Header) + 2] = 127;
  assert(!decoded.decode(wire, wire_size, consumer.rows, consumer.columns));
  terminal.live();
  compare();
  feed("\033[2;3H\033[44mcolored edit\033[0m");
  compare();
  feed("\033[1;100Hwide tail\033[0m");
  compare();
  feed("\033[1;1H\033[2Kshort");
  compare();
  feed("\033[45m\033[2J\033[0m");
  compare();
  feed("\033[2Jretry");
  assert(!cached.paint(terminal, 1366, 768, frame,
                       [](unsigned, unsigned, unsigned *) { return false; }));
  compare();
  assert(terminal.configure(640, 480));
  for (unsigned i = 0; i < 1200; ++i)
  {
    char line[] = "A\r\n";
    line[0] += i % 26;
    feed(line);
  }
  assert(terminal.history_count == fbterm::Terminal::history_capacity);
  terminal.scroll_history(99999);
  assert(terminal.view_offset == terminal.history_count);
  assert(terminal.display_cell(0, 0).character ==
         'A' + (terminal.scroll_position - terminal.history_count) % 26);
  terminal.scroll_history(-99999);
  assert(terminal.view_offset == 0);
  feed("\033[3J");
  assert(terminal.history_count == 0 && terminal.view_offset == 0);
  // Model host-side resource storage separately from mapped guest memory.
  // Only submitted pixels reach the host; scanout is a moving window into it.
  static fbterm::Renderer viewport;
  constexpr unsigned viewport_width = 640, viewport_height = 481;
  auto *backing = static_cast<unsigned *>(
      std::calloc(viewport_width * viewport_height * 2, sizeof(unsigned)));
  auto *host = static_cast<unsigned *>(
      std::calloc(viewport_width * viewport_height * 2, sizeof(unsigned)));
  assert(backing && host);
  viewport.backing_height = viewport_height * 2;
  assert(terminal.configure(viewport_width, viewport_height));
  auto view_compare = [&]
  {
    assert(viewport.paint(
        terminal, viewport_width, viewport_height, backing,
        [&](unsigned top, unsigned height, unsigned *data)
        {
          assert(viewport.origin_y + top + height <= viewport_height * 2);
          for (unsigned y = 0; y < height; ++y)
          {
            std::memcpy(host + (viewport.origin_y + top + y) * viewport_width,
                        data + y * viewport_width,
                        viewport.damage_width * sizeof(unsigned));
          }
          return true;
        }));
    for (unsigned y = 0; y < viewport_height; ++y)
    {
      for (unsigned x = 0; x < viewport_width; ++x)
      {
        assert(host[(viewport.origin_y + y) * viewport_width + x] ==
               terminal.pixel(x, y));
      }
    }
  };
  view_compare();
  for (unsigned i = 0; i < 80; ++i)
  {
    feed(i % 3 ? "short scroll\r\n"
               : "\033[45mcolored, wider scroll content\033[0m\r\n");
    view_compare();
  }
  assert(viewport.viewport_reuses > 45 && viewport.viewport_resets > 1);
  assert(viewport.move_ticks == 0);
  terminal.scroll_history(15);
  view_compare();
  terminal.scroll_history(-7);
  view_compare();
  terminal.live();
  view_compare();
  feed("\033[1;100Hwide tail");
  view_compare();
  for (unsigned i = 0; i < 60; ++i)
  {
    feed("short tail erase\r\n");
    view_compare();
  }
  feed("\033[1;1H\033[2Kshort\033[3J");
  view_compare();
  feed("retry");
  assert(!viewport.paint(terminal, viewport_width, viewport_height, backing,
                         [](unsigned, unsigned, unsigned *) { return false; }));
  view_compare();
  std::free(host);
  std::free(backing);
  std::puts(
      "fbterm contract: text, raster, scrolling, editor, ANSI and adversarial input passed");
}
