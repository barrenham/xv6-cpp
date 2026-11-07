#include "state.h"

namespace fbterm_app
{

fbterm::Terminal terminal{};
fbterm::Renderer renderer{};
// Allocate exactly one screen of pixels; never put it on the user stack.
unsigned *pixels = nullptr;
FramebufferInfo screen{};
FramebufferMapping mapping{};
bool mapped_buffer = false;
FramebufferMappedStats mapped_stats{}, mapped_stats_begin{};
volatile sig_atomic_t stopping = 0;

// Opt-in diagnostic modes; normal launches retain the production display path.
Diagnostics diagnostics{};

FrameSample frame_samples[256]{};
FramebufferStats gpu_stats{};
FramebufferTiming gpu_timing{}, gpu_timing_begin{};
bool reference_raster = false;
uint64 sample_count = 0;

bool profiling = false, mirror_output = true, rasterize = true, present = true;
const char *profile_mode = nullptr;

uint64
timestamp() {
  return profiling ? clockticks() : 0;
}

bool
configure_profile() {
  profile_mode = getenv("FBTERM_PROFILE");
  if (!profile_mode || !*profile_mode)
  {
    return true;
  }
  profiling = true;
  const char *raster = getenv("FBTERM_RASTER");
  if (raster && strcmp(raster, "cached") && strcmp(raster, "full"))
  {
    return false;
  }
  reference_raster = raster && !strcmp(raster, "full");
  const char *damage = getenv("FBTERM_DAMAGE");
  renderer.full_damage = damage && !strcmp(damage, "full");
  if (!strcmp(profile_mode, "normal") || !strcmp(profile_mode, "frames"))
  {
    return true;
  }
  if (!strcmp(profile_mode, "no-mirror"))
  {
    mirror_output = false;
  }
  else if (!strcmp(profile_mode, "no-present"))
  {
    present = false;
  }
  else if (!strcmp(profile_mode, "no-paint"))
  {
    rasterize = false;
  }
  else if (!strcmp(profile_mode, "parse-only"))
  {
    mirror_output = rasterize = false;
  }
  else
  {
    return false;
  }
  return true;
}

void
report(uint64 elapsed) {
  if (!profiling)
  {
    return;
  }
  // Raw 10 MHz ticks retain precision; elapsed stages include scheduling time.
  printf("FBPROFILE mode=%s elapsed=%lu poll=%lu read=%lu feed=%lu raster=%lu "
         "draw=%lu mirror=%lu bytes=%lu mirrored=%lu pixels=%lu polls=%lu "
         "reads=%lu writes=%lu frames=%lu draws=%lu maxdraw=%lu maxraster=%lu "
         "rasterrows=%lu reusedrows=%lu busybytes=%lu\n",
         profile_mode, elapsed, diagnostics.poll_ticks, diagnostics.read_ticks,
         diagnostics.feed_ticks, diagnostics.paint_ticks - diagnostics.draw_ticks,
         diagnostics.draw_ticks, diagnostics.mirror_ticks, diagnostics.bytes,
         diagnostics.mirrored, diagnostics.pixel_bytes, diagnostics.polls,
         diagnostics.reads, diagnostics.writes, diagnostics.frames, diagnostics.draws,
         diagnostics.max_draw_ticks, diagnostics.max_raster_ticks,
         static_cast<uint64>(renderer.raster_rows),
         static_cast<uint64>(renderer.reused_rows), diagnostics.busy_bytes);
  printf("GPUSTATS interrupts=%lu waits=%lu bootpolls=%lu failed=%lu\n",
         static_cast<uint64>(gpu_stats.interrupts),
         static_cast<uint64>(gpu_stats.runtime_waits),
         static_cast<uint64>(gpu_stats.boot_polls),
         static_cast<uint64>(gpu_stats.failed_waits));
  printf("FBSTAGES move=%lu copy_ms=%lu transfer_ms=%lu flush_ms=%lu "
         "copied=%lu gpudraws=%lu\n",
         static_cast<uint64>(renderer.move_ticks),
         gpu_timing.copy_ms - gpu_timing_begin.copy_ms,
         gpu_timing.transfer_ms - gpu_timing_begin.transfer_ms,
         gpu_timing.flush_ms - gpu_timing_begin.flush_ms,
         gpu_timing.copied_bytes - gpu_timing_begin.copied_bytes,
         gpu_timing.draws - gpu_timing_begin.draws);
  printf("FBMAPPED enabled=%d presents=%lu uploaded=%lu scanouts=%lu\n", mapped_buffer,
         mapped_stats.presents - mapped_stats_begin.presents,
         mapped_stats.uploaded_bytes - mapped_stats_begin.uploaded_bytes,
         mapped_stats.scanout_changes - mapped_stats_begin.scanout_changes);
  if (!strcmp(profile_mode, "frames"))
  {
    uint64 first = sample_count > 256 ? sample_count - 256 : 0;
    for (uint64 i = first; i < sample_count; ++i)
    {
      const auto &sample = frame_samples[i % 256];
      printf("FBFRAME index=%lu raster=%lu draw=%lu rows=%lu reused=%lu\n", i,
             sample.raster, sample.draw, sample.rows, sample.reused);
    }
  }
}

void
decimal(char *output, unsigned value) {
  char digits[12];
  unsigned count = 0;
  do
  {
    digits[count++] = '0' + value % 10;
    value /= 10;
  } while (value);
  for (unsigned i = 0; i < count; ++i)
  {
    output[i] = digits[count - 1 - i];
  }
  output[count] = 0;
}

void
stop(int) {
  stopping = 1;
}

} // namespace fbterm_app
