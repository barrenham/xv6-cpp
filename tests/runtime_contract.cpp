#include <cstdio>
#include <cstdlib>

import xv.runtime;

namespace
{
void check(bool condition, const char *message)
{
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}
} // namespace

int main()
{
  namespace rt = xv::runtime;
  unsigned char bytes[8]{};
  check(rt::fill(bytes + 1, 0x180, 6) == bytes + 1, "fill returns destination");
  check(bytes[0] == 0 && bytes[1] == 0x80 && bytes[6] == 0x80 && bytes[7] == 0,
        "fill truncates byte value and preserves boundaries");
  check(rt::compare_bytes(bytes + 1, bytes, 1) > 0,
        "byte comparison treats high bits as unsigned");
  check(rt::fill(bytes, 1, 0) == bytes && bytes[0] == 0, "empty fill");

  char overlap[] = "abcdef";
  check(rt::move(overlap + 1, overlap, 5) == overlap + 1, "move returns destination");
  check(rt::compare_bytes(overlap, "aabcde", 7) == 0, "backward overlap");
  rt::move(overlap, overlap + 1, 5);
  check(rt::compare_bytes(overlap, "abcdee", 7) == 0, "forward overlap");
  rt::move(overlap, overlap, 6);
  check(rt::move(nullptr, nullptr, 0) == nullptr, "empty null move");

  char text[6] = {'!', '!', '!', '!', '!', '!'};
  rt::copy_fixed(text, "a", 4);
  check(text[0] == 'a' && text[1] == 0 && text[3] == 0 && text[4] == '!',
        "fixed copy pads exactly the supplied capacity");
  rt::copy_fixed(text, "abcdef", 4);
  check(rt::compare_bytes(text, "abcd", 4) == 0 && text[4] == '!',
        "full fixed copy does not add a terminator outside capacity");
  rt::copy_string(text, "abcdef", 4);
  check(rt::compare_bytes(text, "abc", 4) == 0 && text[4] == '!',
        "safe copy terminates a truncated string");
  rt::copy_string(text, "x", 0);
  rt::copy_string(text, "x", -1);
  check(text[0] == 'a', "nonpositive capacities do not write");
  rt::copy_string(text, "x", 1);
  check(text[0] == 0, "one-byte capacity holds a terminator");
  check(rt::length("hello") == 5 && rt::length("") == 0, "string length");
  check(rt::compare_text("abc", "abd", 2) == 0 &&
            rt::compare_text("abc", "abd", 3) < 0 && rt::compare_text("a", "ab", 3) < 0,
        "text comparison stops at capacity or terminator");
  static_assert(rt::array_size(bytes) == 8);
  std::puts("runtime contract: passed");
}
