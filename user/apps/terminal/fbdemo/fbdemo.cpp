#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/framebuffer.h"
#include "user/runtime/user.h"
static unsigned pixels[1920];

static bool
mapped_demo(int fd, const FramebufferInfo &info) {
  FramebufferMapping mapping{};
  FramebufferTiming before{}, after{};
  if (ioctl(fd, XV_FB_MAP, ~0UL) != -1 ||
      ioctl(fd, XV_FB_MAP, reinterpret_cast<uint64>(&mapping)) < 0 ||
      mapping.version != 1 || mapping.backing_height < info.height * 2 ||
      mapping.bytes < static_cast<uint64>(info.stride) * mapping.backing_height)
  {
    return false;
  }
  auto *shared = reinterpret_cast<volatile unsigned *>(mapping.address);
  shared[0] = 0;
  int child = fork();
  if (child == 0)
  {
    shared[0] = 0x123456;
    exit(0);
  }
  int status = -1;
  if (child < 0 || wait(&status) != child || status || shared[0] != 0x123456)
  {
    return false;
  }
  child = fork();
  if (child == 0)
  {
    char *args[] = {const_cast<char *>("fbdemo"),
                    const_cast<char *>("--expect-unmapped"), nullptr};
    execvp(args[0], args);
    exit(1);
  }
  if (child < 0 || wait(&status) != child || status)
  {
    return false;
  }
  // Paint the second screen, then select it: the QMP/VNC test sees these pixels.
  for (unsigned y = info.height; y < info.height * 2; ++y)
  {
    for (unsigned x = 0; x < info.width; ++x)
    {
      shared[y * info.width + x] = x < info.width / 3       ? 0x00ff0000
                                   : x < 2 * info.width / 3 ? 0x0000ff00
                                                            : 0x000000ff;
    }
  }
  FramebufferPresent update{0, 0, info.width, info.height, info.height, 0};
  if (ioctl(fd, XV_FB_GET_TIMING, reinterpret_cast<uint64>(&before)) < 0 ||
      ioctl(fd, XV_FB_PRESENT, reinterpret_cast<uint64>(&update)) < 0 ||
      ioctl(fd, XV_FB_GET_TIMING, reinterpret_cast<uint64>(&after)) < 0 ||
      before.copied_bytes != after.copied_bytes)
  {
    return false;
  }
  update.origin_y = info.height + 1;
  if (ioctl(fd, XV_FB_PRESENT, reinterpret_cast<uint64>(&update)) != -1 ||
      ioctl(fd, XV_FB_UNMAP, 0) < 0 || ioctl(fd, XV_FB_UNMAP, 0) != -1)
  {
    return false;
  }
  update.origin_y = 0;
  if (ioctl(fd, XV_FB_PRESENT, reinterpret_cast<uint64>(&update)) != -1)
  {
    return false;
  }
  ui::success("fbdemo: mapped fork/exec/unmap and zero-copy checks passed\n");
  return true;
}

extern "C" int
app_main(int argc, char **argv) {
  int fd = open("/dev/fb0", O_RDWR);
  if (fd < 0)
  {
    ui::error("fbdemo: no framebuffer (use qemu-graphics)\n");
    return 1;
  }
  FramebufferInfo info{};
  if (ioctl(fd, XV_FB_GET_INFO, reinterpret_cast<unsigned long>(&info)) < 0 ||
      info.version != 1 || info.width > 1920 || !info.width || !info.height ||
      info.bytes_per_pixel != 4)
  {
    ui::error("fbdemo: unsupported framebuffer\n");
    close(fd);
    return 1;
  }
  if (argc == 2 && !strcmp(argv[1], "--expect-unmapped"))
  {
    FramebufferPresent update{0, 0, 1, 1, 0, 0};
    bool okay = ioctl(fd, XV_FB_PRESENT, reinterpret_cast<uint64>(&update)) == -1 &&
                ioctl(fd, XV_FB_UNMAP, 0) == -1;
    close(fd);
    return okay ? 0 : 1;
  }
  if (argc == 2 && !strcmp(argv[1], "--mapped"))
  {
    bool okay = mapped_demo(fd, info);
    ui::print(1, okay ? ui::Tone::Success : ui::Tone::Error,
              "fbdemo: %dx%d RGB bars %s\n", info.width, info.height,
              okay ? "displayed" : "failed");
    close(fd);
    return okay ? 0 : 1;
  }
  FramebufferUpdate invalid{
      info.width, 0, 1, 1, 4, 0, reinterpret_cast<uint64>(pixels)};
  if (ioctl(fd, 0, 0) != -1 || ioctl(fd, XV_FB_GET_INFO, ~0UL) != -1 ||
      ioctl(fd, XV_FB_DRAW, reinterpret_cast<unsigned long>(&invalid)) != -1)
  {
    ui::error("fbdemo: invalid request accepted\n");
    close(fd);
    return 1;
  }
  invalid.x = 0;
  invalid.pixels = 0x3ffffff000UL;
  if (ioctl(fd, XV_FB_DRAW, reinterpret_cast<unsigned long>(&invalid)) != -1)
  {
    ui::error("fbdemo: invalid pixels accepted\n");
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
    ui::error("fbdemo: inherited descriptor permission check failed\n");
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
      ui::error("fbdemo: draw failed\n");
      close(fd);
      return 1;
    }
  }
  ui::success("fbdemo: %dx%d RGB bars displayed\n", info.width, info.height);
  close(fd);
  return 0;
}
