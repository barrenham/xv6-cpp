#include "state.h"

namespace fbterm_app
{
bool
flush(int framebuffer) {
  if (!rasterize)
  {
    for (auto &row : terminal.dirty)
    {
      row = false;
    }
    terminal.scrolled = false;
    return true;
  }
  uint64 begin = timestamp();
  uint64 draw_before = diagnostics.draw_ticks, frames_before = diagnostics.frames;
  uint64 rows_before = renderer.raster_rows, reuse_before = renderer.reused_rows;
  auto submit = [&](unsigned top, unsigned height, unsigned *data) {
    unsigned width = reference_raster ? screen.width : renderer.damage_width;
    FramebufferUpdate update{
        0, top, width, height, screen.width * 4, 0, reinterpret_cast<uint64>(data)};
    if (profiling)
    {
      ++diagnostics.frames;
      if (reference_raster)
      {
        renderer.raster_rows +=
            (height + fbterm::cell_height - 1) / fbterm::cell_height;
      }
    }
    if (!present)
    {
      return true;
    }
    uint64 begin = timestamp();
    FramebufferPresent region{
        0, top, height ? width : 0, height, reference_raster ? 0 : renderer.origin_y,
        0};
    bool okay = mapped_buffer ? ioctl(framebuffer, XV_FB_PRESENT,
                                      reinterpret_cast<unsigned long>(&region)) == 0
                              : ioctl(framebuffer, XV_FB_DRAW,
                                      reinterpret_cast<unsigned long>(&update)) == 0;
    if (profiling)
    {
      uint64 duration = timestamp() - begin;
      diagnostics.draw_ticks += duration;
      ++diagnostics.draws;
      diagnostics.pixel_bytes += width * height * 4;
      if (duration > diagnostics.max_draw_ticks)
      {
        diagnostics.max_draw_ticks = duration;
      }
    }
    return okay;
  };
  bool okay =
      reference_raster
          ? fbterm::paint(terminal, screen.width, screen.height, pixels, submit)
          : renderer.paint(terminal, screen.width, screen.height, pixels, submit);
  if (profiling)
  {
    uint64 total = timestamp() - begin;
    diagnostics.paint_ticks += total;
    uint64 draw = diagnostics.draw_ticks - draw_before;
    uint64 raster = total - draw;
    if (raster > diagnostics.max_raster_ticks)
    {
      diagnostics.max_raster_ticks = raster;
    }
    if (diagnostics.frames != frames_before && !strcmp(profile_mode, "frames"))
    {
      frame_samples[sample_count++ % 256] = {raster, draw,
                                             renderer.raster_rows - rows_before,
                                             renderer.reused_rows - reuse_before};
    }
  }
  return okay;
}

bool
demo(int framebuffer) {
  const char *header = "fbterm: ASCII terminal\r\n";
  terminal.feed(header, strlen(header));
  for (unsigned i = 0; i < terminal.rows + 5; ++i)
  {
    char line[] =
        "scroll 00: ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789\r\n";
    line[7] = '0' + i / 10;
    line[8] = '0' + i % 10;
    terminal.feed(line, strlen(line));
  }
  const char *footer = "\033[32mfbterm demo complete\033[0m";
  terminal.feed(footer, strlen(footer));
  return flush(framebuffer);
}

} // namespace fbterm_app
