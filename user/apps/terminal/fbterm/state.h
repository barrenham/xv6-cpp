#pragma once
#include "application.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/io_status.h"
#include "kernel/fs/poll.h"
#include "kernel/io/framebuffer.h"
#include "kernel/process/wait/wait.h"
#include "kernel/support/types.h"
#include "kernel/tty/termios.h"
#include "keyboard.h"
#include "terminal.h"
#include "render.h"
#include "pacing.h"
#include "navigation.h"
#include "snapshot.h"
#include "user/runtime/user.h"
#include "user/runtime/terminal_proxy.h"

namespace fbterm_app
{
struct Diagnostics
{
  uint64 poll_ticks = 0, read_ticks = 0, feed_ticks = 0;
  uint64 paint_ticks = 0, draw_ticks = 0, mirror_ticks = 0;
  uint64 bytes = 0, mirrored = 0, pixel_bytes = 0, busy_bytes = 0;
  uint64 polls = 0, reads = 0, writes = 0, frames = 0, draws = 0;
  uint64 max_draw_ticks = 0, max_raster_ticks = 0;
};

struct FrameSample
{
  uint64 raster, draw, rows, reused;
};

extern fbterm::Terminal terminal;
extern fbterm::Renderer renderer;
extern unsigned *pixels;
extern FramebufferInfo screen;
extern FramebufferMapping mapping;
extern bool mapped_buffer;
extern FramebufferMappedStats mapped_stats, mapped_stats_begin;
extern volatile sig_atomic_t stopping;
extern Diagnostics diagnostics;
extern FrameSample frame_samples[256];
extern FramebufferStats gpu_stats;
extern FramebufferTiming gpu_timing, gpu_timing_begin;
extern bool reference_raster;
extern uint64 sample_count;
extern bool profiling, mirror_output, rasterize, present;
extern const char *profile_mode;
extern fbterm::Snapshot render_snapshot;
extern unsigned char render_wire[fbterm::Snapshot::wire_capacity];
uint64 timestamp();
bool configure_profile();
void report(uint64 elapsed);
void decimal(char *, unsigned);
void stop(int);
bool flush(int framebuffer);
bool demo(int framebuffer);
bool transfer(int master, int &keyboard, int &mouse);
} // namespace fbterm_app
