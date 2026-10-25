#include "kernel/support/types.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"

extern "C"
{
  int tcgetattr(int fd, termios *settings)
  {
    return ioctl(fd, TCGETS, reinterpret_cast<unsigned long>(settings));
  }

  int tcsetattr(int fd, int action, const termios *settings)
  {
    unsigned request;
    switch (action)
    {
    case TCSANOW:
      request = TCSETS;
      break;
    case TCSADRAIN:
      request = TCSETSW;
      break;
    case TCSAFLUSH:
      request = TCSETSF;
      break;
    default:
      return -1;
    }
    return ioctl(fd, request, reinterpret_cast<unsigned long>(settings));
  }

  int tcflush(int fd, int queue)
  {
    return ioctl(fd, TCFLSH, static_cast<unsigned long>(queue));
  }

  void cfmakeraw(termios *settings)
  {
    settings->c_iflag = 0;
    settings->c_oflag &= ~OPOST;
    settings->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    settings->c_cflag = (settings->c_cflag & ~(CSIZE | PARENB)) | CS8;
    settings->c_cc[VMIN] = 1;
    settings->c_cc[VTIME] = 0;
  }
}
