#pragma once
#include "kernel/io/boot_assets.h"

namespace xv::bootdata
{
// An ordinary boot ELF may carry immutable assets followed by this directory.
// This format knows neither filesystem types nor plugin policy.
class Archive
{
public:
  struct Asset { const char *name; const unsigned char *data; unsigned long size; unsigned flags; };
  Archive(const void *data, unsigned long size)
      : data_(static_cast<const unsigned char *>(data)), size_(size) {}

  bool valid() const
  {
    if (!data_ || size_ < Trailer) return false;
    auto *tail = data_ + size_ - Trailer;
    const char magic[] = "XVBOOT01";
    for (unsigned i = 0; i < 8; ++i) if (tail[i] != magic[i]) return false;
    if (word(tail + 8, 4) != Version || word(tail + 12, 4) != Entry || word(tail + 28, 4)) return false;
    auto table = word(tail + 16, 8), count = word(tail + 24, 4);
    if (!count || count > 128 || table > size_ - Trailer ||
        count * Entry != size_ - Trailer - table) return false;
    for (unsigned i = 0; i < count; ++i)
    {
      const auto *entry = data_ + table + i * Entry;
      unsigned length = 0;
      while (length < NameSize && entry[length])
      {
        if (entry[length] < 33 || entry[length] > 126) return false;
        ++length;
      }
      if (!length || length == NameSize) return false;
      auto offset = word(entry + 64, 8), size = word(entry + 72, 8);
      if (!size || offset > table || size > table - offset ||
          word(entry + 80, 4) & ~Executable || word(entry + 84, 4)) return false;
      for (unsigned j = 0; j < i; ++j)
        if (same(reinterpret_cast<const char *>(entry),
                 reinterpret_cast<const char *>(data_ + table + j * Entry))) return false;
    }
    return true;
  }

  bool get(unsigned index, const char *name, Asset &out) const
  {
    if (!valid()) return false;
    auto *tail = data_ + size_ - Trailer;
    auto table = word(tail + 16, 8), count = word(tail + 24, 4);
    for (unsigned i = 0; i < count; ++i)
    {
      auto *entry = data_ + table + i * Entry;
      auto *key = reinterpret_cast<const char *>(entry);
      if (name && *name ? !same(name, key) : i != index) continue;
      out = {key, data_ + word(entry + 64, 8), word(entry + 72, 8), unsigned(word(entry + 80, 4))};
      return true;
    }
    return false;
  }

private:
  static constexpr unsigned Trailer = 32, Entry = 88;
  static unsigned long word(const unsigned char *p, unsigned size)
  {
    unsigned long value = 0;
    for (unsigned i = 0; i < size; ++i) value |= static_cast<unsigned long>(p[i]) << (8 * i);
    return value;
  }
  static bool same(const char *a, const char *b)
  {
    for (unsigned i = 0; i < NameSize; ++i)
      if (a[i] != b[i]) return false;
      else if (!a[i]) return true;
    return false;
  }
  const unsigned char *data_;
  unsigned long size_;
};
}
