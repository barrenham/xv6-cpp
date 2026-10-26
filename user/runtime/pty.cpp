#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"
extern "C" int posix_openpt(int flags)
{
  if ((flags & O_ACCMODE) != O_RDWR || (flags & ~(O_RDWR | O_NONBLOCK))) return -1;
  return open("/dev/ptmx", flags);
}
extern "C" int grantpt(int fd)
{
  unsigned number;
  return ioctl(fd, TIOCGPTN, reinterpret_cast<unsigned long>(&number));
}
extern "C" int unlockpt(int fd)
{
  int locked = 0;
  return ioctl(fd, TIOCSPTLCK, reinterpret_cast<unsigned long>(&locked));
}
extern "C" int ptsname_r(int fd, char *buffer, unsigned length)
{
  unsigned number;
  if (!buffer || ioctl(fd, TIOCGPTN, reinterpret_cast<unsigned long>(&number)) < 0) return -1;
  char digits[12]; unsigned count = 0;
  do { digits[count++] = '0' + number % 10; number /= 10; } while (number);
  if (length < 10 + count) return -1;
  strcpy(buffer, "/dev/pts/");
  for (unsigned i = 0; i < count; ++i) buffer[9 + i] = digits[count - i - 1];
  buffer[9 + count] = 0;
  return 0;
}
