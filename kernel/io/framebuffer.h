#pragma once
#include "kernel/support/types.h"
// Project ABI, version 1. Pixels are little-endian B,G,R,X (0x00RRGGBB).
inline constexpr unsigned XV_FB_GET_INFO=0x58460001U;
inline constexpr unsigned XV_FB_DRAW=0x58460002U;
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
