#include "kernel/support/types.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"

static int byte_value(const char *text)
{
  int value = 0;
  if (!*text)
  {
    return -1;
  }
  for (; *text; ++text)
  {
    if (*text < '0' || *text > '9')
    {
      return -1;
    }
    value = value * 10 + *text - '0';
    if (value > 255)
    {
      return -1;
    }
  }
  return value;
}

extern "C" int main(int argc, char **argv)
{
  termios settings{};
  if (tcgetattr(0, &settings) < 0)
  {
    fprintf(2, "stty: stdin is not a terminal\n");
    return 1;
  }
  if (argc == 1 || (argc == 2 && strcmp(argv[1], "-a") == 0))
  {
    printf("%s %s %s min %d time %d\n",
           settings.c_lflag & ICANON ? "icanon" : "-icanon",
           settings.c_lflag & ECHO ? "echo" : "-echo",
           settings.c_oflag & ONLCR ? "onlcr" : "-onlcr", settings.c_cc[VMIN],
           settings.c_cc[VTIME]);
    return 0;
  }
  for (int i = 1; i < argc; ++i)
  {
    const char *option = argv[i];
    if (strcmp(option, "raw") == 0)
    {
      cfmakeraw(&settings);
    }
    else if (strcmp(option, "sane") == 0)
    {
      settings = {};
      settings.c_iflag = ICRNL;
      settings.c_oflag = OPOST;
      settings.c_cflag = B38400 | CS8 | CREAD | CLOCAL;
      settings.c_lflag = ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;
      settings.c_cc[VERASE] = 127;
      settings.c_cc[VKILL] = 21;
      settings.c_cc[VEOF] = 4;
      settings.c_cc[VWERASE] = 23;
      settings.c_cc[VLNEXT] = 22;
      settings.c_cc[VMIN] = 1;
    }
    else if (strcmp(option, "echo") == 0)
    {
      settings.c_lflag |= ECHO;
    }
    else if (strcmp(option, "-echo") == 0)
    {
      settings.c_lflag &= ~ECHO;
    }
    else if (strcmp(option, "icanon") == 0)
    {
      settings.c_lflag |= ICANON;
    }
    else if (strcmp(option, "-icanon") == 0)
    {
      settings.c_lflag &= ~ICANON;
    }
    else if (strcmp(option, "onlcr") == 0)
    {
      settings.c_oflag |= OPOST | ONLCR;
    }
    else if (strcmp(option, "-onlcr") == 0)
    {
      settings.c_oflag &= ~ONLCR;
    }
    else if ((strcmp(option, "min") == 0 || strcmp(option, "time") == 0) &&
             i + 1 < argc)
    {
      int value = byte_value(argv[++i]);
      if (value < 0)
      {
        goto usage;
      }
      settings.c_cc[strcmp(option, "min") == 0 ? VMIN : VTIME] = value;
    }
    else
    {
      goto usage;
    }
  }
  if (tcsetattr(0, TCSANOW, &settings) == 0)
  {
    return 0;
  }
  fprintf(2, "stty: settings rejected\n");
  return 1;
usage:
  fprintf(2, "usage: stty [-a | sane | raw | [-]echo | [-]icanon | "
             "[-]onlcr | min N | time N] ...\n");
  return 1;
}
