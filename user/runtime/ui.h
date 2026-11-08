#pragma once
#include "terminal_style.h"
#include <stdarg.h>

extern "C" void xv_vprintf_styled(int, const char *, const char *, va_list);

// Semantic presentation is opt-in. printf/write remain byte-transparent for
// file contents, protocols, raw modes, benchmarks and shell data pipelines.
namespace ui
{
enum class Tone
{
  Normal,
  Heading,
  Info,
  Success,
  Warning,
  Error,
  Muted
};

inline const char *
sequence(Tone tone) {
  switch (tone)
  {
  case Tone::Heading:
    return "\033[1m";
  case Tone::Info:
    return "";
  case Tone::Success:
    return "\033[32m";
  case Tone::Warning:
    return "\033[33m";
  case Tone::Error:
    return "\033[1;31m";
  case Tone::Muted:
    return "\033[90m";
  default:
    return "";
  }
}

inline void
print(int fd, Tone tone, const char *format, ...) {
  int saved_errno = errno;
  const char *style =
      *sequence(tone) && terminal_style::enabled(fd) ? sequence(tone) : nullptr;
  errno = saved_errno;
  va_list arguments;
  va_start(arguments, format);
  // Color, formatted text and reset use the same invocation-local buffer.
  xv_vprintf_styled(fd, style, format, arguments);
  va_end(arguments);
}

template <typename... Args>
inline void
heading(const char *format, Args... args) {
  print(1, Tone::Heading, format, args...);
}

template <typename... Args>
inline void
info(const char *format, Args... args) {
  print(1, Tone::Info, format, args...);
}

template <typename... Args>
inline void
success(const char *format, Args... args) {
  print(1, Tone::Success, format, args...);
}

template <typename... Args>
inline void
warning(const char *format, Args... args) {
  print(1, Tone::Warning, format, args...);
}

template <typename... Args>
inline void
muted(const char *format, Args... args) {
  print(1, Tone::Muted, format, args...);
}

template <typename... Args>
inline void
error(const char *format, Args... args) {
  print(2, Tone::Error, format, args...);
}

template <typename... Args>
inline void
usage(const char *format, Args... args) {
  print(2, Tone::Warning, format, args...);
}
} // namespace ui
