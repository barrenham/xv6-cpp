#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <sys/mman.h>
#include <unistd.h>

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
  // The actual exported runtime entry uses the shared kernel/user fill body.
  alignas(64) unsigned char actual[8448], expected[8448];
  for (unsigned offset=0;offset<32;++offset)
    for (unsigned length : {0U,1U,2U,7U,8U,9U,15U,16U,17U,63U,64U,65U,255U,4095U,4096U,8192U})
      for (int value : {-1025,-1,0,1,5,127,128,255,256,0x180,0x12345})
      {
        std::memset(actual,0xa7,sizeof(actual));std::memset(expected,0xa7,sizeof(expected));
        check(rt::fill(actual+offset,value,length)==actual+offset,"fill destination across alignments");
        std::memset(expected+offset,value,length);
        check(!std::memcmp(actual,expected,sizeof(actual)),"fill byte pattern and both boundaries");
      }
  check(rt::fill(nullptr,42,0)==nullptr,"zero null fill");
  const auto page=static_cast<unsigned long>(sysconf(_SC_PAGESIZE));
  auto *guard=static_cast<unsigned char *>(mmap(nullptr,page*3,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
  check(guard!=MAP_FAILED && !mprotect(guard+page,page,PROT_READ|PROT_WRITE),"fill guard mapping");
  for (unsigned long length=0;length<=page;++length)
  {
    rt::fill(guard+page,0x5a,length);
    rt::fill(guard+page*2-length,0xa5,length);
  }
  check(!munmap(guard,page*3),"fill guard release");
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
