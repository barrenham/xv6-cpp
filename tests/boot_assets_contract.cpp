#include "kernel/boot/asset_archive.h"
#include <array>
#include <cassert>
#include <cstring>
#include <cstdio>

using Bytes = std::array<unsigned char, 224>;
static void word(Bytes &bytes, unsigned at, unsigned long value, unsigned size)
{
  for (unsigned i = 0; i < size; ++i) bytes[at + i] = value >> (8 * i);
}
static Bytes fixture()
{
  Bytes bytes{};
  std::memcpy(bytes.data(), "payload!", 8);
  for (unsigned i = 0; i < 2; ++i)
  {
    unsigned at = 16 + i * 88;
    std::memcpy(bytes.data() + at, i ? "second" : "first", i ? 7 : 6);
    word(bytes, at + 64, i * 4, 8);
    word(bytes, at + 72, 4, 8);
    word(bytes, at + 80, i, 4);
  }
  std::memcpy(bytes.data() + 192, "XVBOOT01", 8);
  word(bytes, 200, 1, 4);
  word(bytes, 204, 88, 4);
  word(bytes, 208, 16, 8);
  word(bytes, 216, 2, 4);
  return bytes;
}
int main()
{
  const auto good = fixture();
  xv::bootdata::Archive archive(good.data(), good.size());
  xv::bootdata::Archive::Asset asset{};
  assert(archive.valid());
  assert(archive.get(0, nullptr, asset) && asset.size == 4 && asset.flags == 0);
  assert(archive.get(0, "second", asset) && asset.flags == xv::bootdata::Executable);
  assert(!archive.get(2, nullptr, asset) && !archive.get(0, "missing", asset));
  for (unsigned length = 0; length < good.size(); ++length)
    assert(!xv::bootdata::Archive(good.data(), length).valid());
  for (auto field : {192u, 200u, 204u, 208u, 216u, 220u, 80u, 88u, 96u, 100u})
  {
    auto bytes = good;
    word(bytes, field, ~0UL, field == 208 || field == 80 || field == 88 ? 8 : 4);
    assert(!xv::bootdata::Archive(bytes.data(), bytes.size()).valid());
  }
  auto bytes = good;
  std::memcpy(bytes.data() + 104, "first", 6);
  assert(!xv::bootdata::Archive(bytes.data(), bytes.size()).valid());
  bytes = good;
  std::memset(bytes.data() + 16, 'a', 64);
  assert(!xv::bootdata::Archive(bytes.data(), bytes.size()).valid());
  bytes = good;
  bytes[16] = '\n';
  assert(!xv::bootdata::Archive(bytes.data(), bytes.size()).valid());
  assert(!xv::bootdata::Archive(nullptr, good.size()).valid());
  std::puts("Boot assets: bounds, truncation, metadata, duplicate names, flags and lookup passed");
}
