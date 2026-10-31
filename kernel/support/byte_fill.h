#pragma once

namespace xv::support
{
// Byte semantics for arbitrary alignment and length. may_alias gives these
// aligned word stores the same aliasing freedom as the byte accesses they
// replace; prefix/tail stores never extend outside the requested interval.
inline void *byte_fill(void *destination, int value, unsigned long count)
{
  auto *bytes = static_cast<unsigned char *>(destination);
  const auto byte = static_cast<unsigned char>(value);
  constexpr auto width = sizeof(unsigned long);
  while (count && (reinterpret_cast<unsigned long>(bytes) & (width - 1)))
  {
    *bytes++ = byte;
    --count;
  }
  typedef unsigned long AliasWord __attribute__((__may_alias__));
  const AliasWord pattern = (~AliasWord(0) / 255) * byte;
  auto *words = reinterpret_cast<AliasWord *>(bytes);
  while (count >= width)
  {
    *words++ = pattern;
    count -= width;
  }
  bytes = reinterpret_cast<unsigned char *>(words);
  while (count)
  {
    *bytes++ = byte;
    --count;
  }
  return destination;
}
} // namespace xv::support
