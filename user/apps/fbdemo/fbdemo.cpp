#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/framebuffer.h"
#include "user/runtime/user.h"
static unsigned pixels[1920];

extern "C" int main()
{
  int fd = open("/dev/fb0", O_RDWR);
  if (fd < 0)
  {
    fprintf(2, "fbdemo: no framebuffer (use qemu-graphics)\n");
    return 1;
  }
  FramebufferInfo info{};
  if (ioctl(fd, XV_FB_GET_INFO, reinterpret_cast<unsigned long>(&info)) < 0 ||
      info.version != 1 || info.width > 1920 || !info.width || !info.height ||
      info.bytes_per_pixel != 4)
  {
    fprintf(2, "fbdemo: unsupported framebuffer\n");
    close(fd);
    return 1;
  }
  FramebufferUpdate invalid{
      info.width, 0, 1, 1, 4, 0, reinterpret_cast<uint64>(pixels)};
  if (ioctl(fd, 0, 0) != -1 || ioctl(fd, XV_FB_GET_INFO, ~0UL) != -1 ||
      ioctl(fd, XV_FB_DRAW, reinterpret_cast<unsigned long>(&invalid)) != -1)
  {
    fprintf(2, "fbdemo: invalid request accepted\n");
    close(fd);
    return 1;
  }
  invalid.x = 0;
  invalid.pixels = 0x3ffffff000UL;
  if (ioctl(fd, XV_FB_DRAW, reinterpret_cast<unsigned long>(&invalid)) != -1)
  {
    fprintf(2, "fbdemo: invalid pixels accepted\n");
    close(fd);
    return 1;
  }
  int child = fork();
  if (child == 0)
  {
    invalid.pixels = reinterpret_cast<uint64>(pixels);
    exit(setuid(1000) == 0 &&
                 ioctl(fd, XV_FB_DRAW, reinterpret_cast<unsigned long>(&invalid)) == -1
             ? 0
             : 1);
  }
  int status = -1;
  if (child < 0 || wait(&status) != child || status != 0)
  {
    fprintf(2, "fbdemo: inherited descriptor permission check failed\n");
    close(fd);
    return 1;
  }
  // Three solid bars make display output independently machine-verifiable.
  for (unsigned y = 0; y < info.height; ++y)
  {
    for (unsigned x = 0; x < info.width; ++x)
    {
      pixels[x] = x < info.width / 3       ? 0x00ff0000
                  : x < 2 * info.width / 3 ? 0x0000ff00
                                           : 0x000000ff;
    }
    FramebufferUpdate update{
        0, y, info.width, 1, info.width * 4, 0, reinterpret_cast<uint64>(pixels)};
    if (ioctl(fd, XV_FB_DRAW, reinterpret_cast<unsigned long>(&update)) < 0)
    {
      fprintf(2, "fbdemo: draw failed\n");
      close(fd);
      return 1;
    }
  }
  printf("fbdemo: %dx%d RGB bars displayed\n", info.width, info.height);
  close(fd);
  return 0;
}
