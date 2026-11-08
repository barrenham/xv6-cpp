#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/ui.h"
static char output[4096];
static unsigned used, calls, interrupts;
static unsigned limit = ~0U;
static int fail_after = -1;
static bool zero_write;
static int destination;
static unsigned terminal_fds = 6;
static char *no_color, *term;

extern "C" char *
getenv(const char *name) {
  return !strcmp(name, "NO_COLOR") ? no_color : !strcmp(name, "TERM") ? term : nullptr;
}

extern "C" int
tcgetattr(int fd, struct termios *) {
  if (fd >= 0 && fd < 3 && (terminal_fds & (1U << fd)))
  {
    return 0;
  }
  errno = EBADF;
  return -1;
}

extern "C" ssize_t
write(int fd, const void *data, size_t count) {
  ++calls;
  destination = fd;
  if (interrupts)
  {
    --interrupts;
    errno = EINTR;
    return -1;
  }
  if (zero_write)
  {
    return 0;
  }
  if (!fail_after)
  {
    errno = EAGAIN;
    return -1;
  }
  if (fail_after > 0)
  {
    --fail_after;
  }
  if (count > limit)
  {
    count = limit;
  }
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

static bool
expect(const char *text) {
  unsigned i = 0;
  while (text[i] && i < used && text[i] == output[i])
  {
    ++i;
  }
  bool result = i == used && !text[i];
  used = 0;
  return result;
}

int
main() {
  printf("[%6d][%-5s][%05d][%8lu][%c][%%]", 12, "R", -12, 123456UL, 'X');
  if (calls != 1 || !expect("[    12][R    ][-0012][  123456][X][%]"))
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
  // A prompt without a newline must already be visible on return.
  calls = 0;
  fprintf(7, "Password: ");
  if (destination != 7 || calls != 1 || !expect("Password: "))
  {
    return 5;
  }

  char long_text[1025];
  for (unsigned i = 0; i + 1 < sizeof(long_text); ++i)
  {
    long_text[i] = 'a' + i % 26;
  }
  long_text[sizeof(long_text) - 1] = 0;
  calls = 0;
  printf("%s", long_text);
  if (!calls || calls > 8 || !expect(long_text))
  {
    return 6;
  }

  // Retry interrupted and partial writes without duplicating or losing bytes.
  calls = 0;
  limit = 7;
  interrupts = 2;
  printf("%s", long_text);
  if (calls < 3 || !expect(long_text))
  {
    return 7;
  }

  // A nonblocking destination must return after the first terminal error.
  calls = 0;
  limit = 5;
  fail_after = 2;
  errno = 0;
  printf("%s", long_text);
  char prefix[11];
  for (unsigned i = 0; i < 10; ++i)
  {
    prefix[i] = long_text[i];
  }
  prefix[10] = 0;
  if (calls != 3 || errno != EAGAIN || !expect(prefix))
  {
    return 8;
  }

  calls = 0;
  fail_after = -1;
  zero_write = true;
  printf("no-progress");
  if (calls != 1 || !expect(""))
  {
    return 9;
  }
  zero_write = false;
  limit = ~0U;
  printf("next call works");
  if (!expect("next call works"))
  {
    return 10;
  }
  calls = 0;
  ui::heading("%-6s %4u\n", "NAME", 42U);
  if (calls != 1 || !expect("\033[1mNAME     42\n\033[0m"))
  {
    return 11;
  }
  // Inspect stdout/stderr independently and preserve errno on a failed probe.
  terminal_fds = 4;
  errno = EINVAL;
  ui::success("done\n");
  if (errno != EINVAL || !expect("done\n"))
  {
    return 12;
  }
  ui::error("bad %d\n", 7);
  if (destination != 2 || !expect("\033[1;31mbad 7\n\033[0m"))
  {
    return 13;
  }
  no_color = const_cast<char *>("1");
  ui::error("disabled");
  if (!expect("disabled"))
  {
    return 14;
  }
  no_color = nullptr;
  term = const_cast<char *>("dumb");
  ui::usage("usage");
  if (!expect("usage"))
  {
    return 15;
  }
  term = nullptr;
  terminal_fds = 6;
  ui::print(1, ui::Tone::Normal, "raw");
  if (!expect("raw"))
  {
    return 16;
  }
  limit = 7;
  interrupts = 2;
  ui::success("%s", long_text);
  unsigned prefix_length = strlen("\033[32m");
  bool styled_complete =
      used == prefix_length + sizeof(long_text) - 1 + 4 &&
      !memcmp(output, "\033[32m", prefix_length) &&
      !memcmp(output + prefix_length, long_text, sizeof(long_text) - 1) &&
      !memcmp(output + used - 4, "\033[0m", 4);
  if (!styled_complete)
  {
    return 17;
  }
  return 0;
}
