#pragma once

// Project keyboard ABI. Fixed-width little-endian VirtIO/evdev event fields;
// this is not Linux's timeval-prefixed struct input_event.
struct KeyboardEvent
{
  unsigned short type;
  unsigned short code;
  unsigned value;
};

inline constexpr unsigned short XV_KEY_SYN = 0;
inline constexpr unsigned short XV_KEY_EVENT = 1;
inline constexpr unsigned short XV_KEY_SYN_DROPPED = 3;
inline constexpr unsigned short XV_INPUT_REL = 2;
inline constexpr unsigned short XV_REL_WHEEL = 8;
static_assert(sizeof(KeyboardEvent) == 8);
