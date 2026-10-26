#include "kernel/support/types.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"
namespace
{
void check(bool good, const char *message)
{
  if (!good) { fprintf(2, "devfstest: FAIL: %s\n", message); exit(1); }
}
void devices()
{
  int null = open("/dev/null", O_RDWR | O_NONBLOCK);
  int zero = open("/dev/zero", O_RDWR);
  check(null >= 0 && zero >= 0, "open builtins");
  char data[301]; memset(data, 7, sizeof(data));
  check(write(null, data, sizeof(data)) == sizeof(data) && read(null, data, 1) == 0,
        "null discard and EOF");
  check(read(zero, data, sizeof(data)) == sizeof(data), "zero read");
  for (char byte : data) check(byte == 0, "zero contents");
  check(write(zero, data, sizeof(data)) == sizeof(data), "zero discard");
  check(read(zero, reinterpret_cast<void *>(~0UL), 1) == -1, "zero invalid destination");
  check(read(zero, data, 0) == 0 && read(null, data, 0) == 0, "empty read");
  pollfd ready[] = {{null, POLLIN | POLLOUT, 0}, {zero, POLLIN | POLLOUT, 0}};
  check(poll(ready, 2, 0) == 2 && ready[0].revents == (POLLIN | POLLOUT) &&
        ready[1].revents == (POLLIN | POLLOUT), "readiness");
  check(ioctl(zero, 0xdead, reinterpret_cast<unsigned long>(data)) == -1, "unsupported ioctl");
  check((fcntl(null, F_GETFL, 0) & O_NONBLOCK) != 0, "nonblock flag");
  int copy = dup(null); check(copy >= 0, "dup"); close(null);
  check(write(copy, data, 1) == 1, "duplicate lifetime");
  int child = fork(); check(child >= 0, "fork");
  if (child == 0)
  {
    check(read(zero, data, sizeof(data)) == sizeof(data) && write(copy, data, 1) == 1,
          "inherited file descriptors");
    close(copy); close(zero); exit(0);
  }
  int status = -1; check(wait(&status) == child && status == 0, "child device use");
  close(copy); close(zero);
}
void paths()
{
  struct stat disk{}, dev{}, node{};
  check(stat("/", &disk) == 0 && stat("/dev", &dev) == 0 && disk.dev != dev.dev &&
        dev.type == T_DIR, "mounted filesystem identity");
  check(stat("/dev/console", &node) == 0 && node.type == T_DEVICE && node.dev == dev.dev,
        "console metadata");
  int dir = open("/dev", O_RDONLY); check(dir >= 0, "open directory");
  dirent entry{}; unsigned found = 0; int count;
  while ((count = read(dir, &entry, sizeof(entry))) == sizeof(entry))
  {
    if (!strcmp(entry.name, "console")) found |= 1;
    if (!strcmp(entry.name, "null")) found |= 2;
    if (!strcmp(entry.name, "zero")) found |= 4;
  }
  check(count == 0 && found == 7, "directory listing"); close(dir);
  check(chdir("/dev") == 0 && stat("./zero", &node) == 0 && node.type == T_DEVICE,
        "devfs working directory");
  check(chdir("..") == 0 && stat(".", &node) == 0 && node.dev == disk.dev && node.ino == disk.ino,
        "escape mounted root");
  check(stat("/dev/../dev//./null", &node) == 0 && node.type == T_DEVICE, "cross-mount normalization");
  check(unlink("/dev") < 0 && unlink("/dev/../dev") < 0 && unlink("/dev/null") < 0,
        "mount and nodes cannot be removed");
  check(mkdir("/dev/new") < 0 && mknod("/dev/new", 1, 0) < 0 &&
        open("/dev/new", O_CREATE | O_RDWR) < 0, "immutable namespace");
  check(link("/dev/null", "/devlink") < 0, "cross-filesystem hard link");
  int file = open("/dev-disk", O_CREATE | O_RDWR); check(file >= 0, "disk creation"); close(file);
  check(link("/dev-disk", "/dev/disk") < 0 && unlink("/dev-disk") == 0, "disk to devfs link rejected");
  file = open("/dev/zero", O_CREATE | O_RDWR | O_TRUNC);
  check(file >= 0, "create opens existing device"); close(file);
  file = open("/dev/console", O_RDWR | O_NONBLOCK); check(file >= 0, "open console"); close(file);
  check(open("/dev", O_RDWR) < 0, "directory write rejected");
}
}
extern "C" int main()
{
  paths(); devices();
  for (int i = 0; i < 400; ++i) { int fd = open("/dev/null", O_RDWR); check(fd >= 0, "repeated open"); close(fd); }
  int children[4];
  for (int &child : children)
  {
    child = fork(); check(child >= 0, "concurrent fork");
    if (child == 0) { devices(); exit(0); }
  }
  for (int child : children) { (void)child; int status; check(wait(&status) > 0 && status == 0, "concurrent child"); }
  printf("devfstest: all tests succeeded\n"); exit(0);
}
