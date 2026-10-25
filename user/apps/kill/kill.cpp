#include "kernel/support/types.h"
#include "kernel/process/signals/signal.h"
#include "user/runtime/user.h"

static bool parse_target(const char *text, int &target)
{
  bool negative = *text == '-';
  if (negative)
  {
    ++text;
  }
  if (!*text)
  {
    return false;
  }
  unsigned value = 0;
  for (; *text; ++text)
  {
    if (*text < '0' || *text > '9')
    {
      return false;
    }
    unsigned digit = *text - '0';
    if (value > (0x7fffffffU - digit) / 10)
    {
      return false;
    }
    value = value * 10 + digit;
  }
  target = negative ? -static_cast<int>(value) : static_cast<int>(value);
  return true;
}

extern "C" int main(int argc, char **argv)
{
  int first = 1;
  int signal = -1; // preserve the original one-argument kill ABI by default
  if (argc > 1)
  {
    if (strcmp(argv[1], "-INT") == 0)
    {
      signal = SIGINT;
    }
    else if (strcmp(argv[1], "-TERM") == 0)
    {
      signal = SIGTERM;
    }
    else if (strcmp(argv[1], "-KILL") == 0)
    {
      signal = SIGKILL;
    }
    else if (strcmp(argv[1], "-STOP") == 0)
    {
      signal = SIGSTOP;
    }
    else if (strcmp(argv[1], "-TSTP") == 0)
    {
      signal = SIGTSTP;
    }
    else if (strcmp(argv[1], "-CONT") == 0)
    {
      signal = SIGCONT;
    }
    else if (strcmp(argv[1], "-0") == 0)
    {
      signal = 0;
    }
    if (signal >= 0)
    {
      ++first;
    }
  }
  if (first >= argc)
  {
    fprintf(2, "usage: kill [-INT|-TERM|-KILL|-STOP|-TSTP|-CONT|-0] target...\n");
    return 1;
  }
  int failed = 0;
  for (int i = first; i < argc; ++i)
  {
    int target;
    if (!parse_target(argv[i], target) ||
        (signal < 0 ? kill(target) : sigsend(target, signal)) < 0)
    {
      fprintf(2, "kill: cannot signal %s\n", argv[i]);
      failed = 1;
    }
  }
  return failed;
}
