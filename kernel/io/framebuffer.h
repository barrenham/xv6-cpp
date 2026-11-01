#pragma once
#include "kernel/support/types.h"
// Project ABI, version 1. Pixels are little-endian B,G,R,X (0x00RRGGBB).
inline constexpr unsigned XV_FB_GET_INFO=0x58460001U;
inline constexpr unsigned XV_FB_DRAW=0x58460002U;
inline constexpr unsigned XV_FB_GET_STATS=0x58460003U;
struct FramebufferInfo
{
  uint version,width,height,stride,bytes_per_pixel,format;
};
struct FramebufferUpdate
{
  uint x,y,width,height,stride,reserved;
  uint64 pixels;
};
static_assert(sizeof(FramebufferInfo)==24);
static_assert(sizeof(FramebufferUpdate)==32);

// Diagnostic counters, read with XV_FB_GET_STATS by root.
struct FramebufferStats
{
  unsigned long long interrupts, runtime_waits, boot_polls, failed_waits;
};
static_assert(sizeof(FramebufferStats) == 32);

// Independent diagnostic ABI: original GET_STATS remains exactly 32 bytes.
inline constexpr unsigned XV_FB_GET_TIMING = 0x58460004U;
struct FramebufferTiming
{
  uint64 copy_ms, transfer_ms, flush_ms, copied_bytes, draws;
};
static_assert(sizeof(FramebufferTiming) == 40);

// Shared framebuffer aperture. Visible dimensions still come from GET_INFO.
inline constexpr unsigned XV_FB_MAP = 0x58460005U;
inline constexpr unsigned XV_FB_UNMAP = 0x58460006U;
inline constexpr unsigned XV_FB_PRESENT = 0x58460007U;
inline constexpr unsigned XV_FB_GET_MAPPED_STATS = 0x58460008U;
struct FramebufferMapping
{
  uint64 address, bytes;
  uint backing_height, version;
};
struct FramebufferPresent
{
  uint x, y, width, height, origin_y, reserved;
};
struct FramebufferMappedStats
{
  uint64 maps, presents, uploaded_bytes, scanout_changes;
};
static_assert(sizeof(FramebufferMapping) == 24);
static_assert(sizeof(FramebufferPresent) == 24);
static_assert(sizeof(FramebufferMappedStats) == 32);
