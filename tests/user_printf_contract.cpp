#include "kernel/support/types.h"
#include "user/runtime/user.h"
static char output[4096];
static unsigned used;

extern "C" ssize_t
write(int, const void *data, size_t count) {
  if (used + count >= sizeof(output))
  {
    return -1;
  }
  auto *bytes = static_cast<const char *>(data);
  for (size_t i = 0; i < count; ++i)
  {
    output[used++] = bytes[i];
  }
  output[used] = 0;
  return count;
}

static bool expect(const char *text)
{
  unsigned i = 0;
  while (text[i] && i < used && text[i] == output[i])
  {
    ++i;
  }
  bool result = i == used && !text[i];
  used = 0;
  return result;
}

int main()
{
  printf("[%6d][%-5s][%05d][%8lu][%c][%%]", 12, "R", -12, 123456UL, 'X');
  if (!expect("[    12][R    ][-0012][  123456][X][%]"))
  {
    return 1;
  }
  printf("%d %ld %lld %u %lu %llx %p", -1, -2L, -9223372036854775807LL - 1, 42U,
         18446744073709551615UL, 0xabcULL, 0x123UL);
  if (!expect(
          "-1 -2 -9223372036854775808 42 18446744073709551615 ABC 0x0000000000000123"))
  {
    return 2;
  }
  printf("%3s %s %q", "longer", static_cast<const char *>(nullptr));
  if (!expect("longer (null) %q"))
  {
    return 3;
  }
  printf("A\tB\n%-3c!", 'R');
  if (!expect("A\tB\nR  !"))
  {
    return 4;
  }
  return 0;
}
