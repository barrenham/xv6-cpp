#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/io/dirent.h"
#include "user/fs/sdk/include/xvfs/ext4.h"

namespace
{
char path[256], other[256];
alignas(8) unsigned char buffer[1024];
void check(bool okay, const char *what)
{
  if (!okay) { printf("ext4test: FAIL: %s\n", what); exit(1); }
}
void name(const char *root, const char *suffix)
{
  check(strlen(root) + strlen(suffix) < sizeof(path), "path size");
  strcpy(path, root); strcpy(path + strlen(root), suffix);
}
int file(const char *root, const char *suffix)
{
  name(root, suffix);
  int fd = open(path, O_RDONLY);
  check(fd >= 0, suffix);
  return fd;
}
void message(const char *root, const char *suffix, const char *expected)
{
  int fd = file(root, suffix);
  unsigned count = strlen(expected);
  check(read(fd, buffer, sizeof(buffer)) == int(count) && !memcmp(buffer, expected, count), "file/link contents");
  close(fd);
}
}
extern "C" int main(int argc, char **argv)
{
  check(argc == 2, "mount path required");
  const char *root = argv[1];
  int directory = file(root, "");
  xvfs_control request{};
  request.size = sizeof(request); request.version = XVFS_CONTROL_VERSION;
  request.driver_version = XVFS_EXT4_VERSION; request.command = XVFS_EXT4_GEOMETRY;
  request.capacity = sizeof(request.data); strcpy(request.interface_name, XVFS_EXT4_INTERFACE);
  check(xvfs_driver_control(directory, &request) == sizeof(xvfs_ext4_geometry), "native ext4 geometry");
  xvfs_ext4_geometry geometry;
  memmove(&geometry, request.data, sizeof(geometry));
  check(geometry.driver_read_only && (geometry.incompatible & 0x40), "extents and readonly policy");
  check(umount(root) < 0, "open descriptor keeps mount busy");
  close(directory);
  message(root, "/message", "EXT4_READ_ONLY_OKAY\n");
  message(root, "/shortlink", "EXT4_READ_ONLY_OKAY\n");
  message(root, "/longlink", "LONG_LINK_OKAY\n");
  int fd = file(root, "/blob");
  check(lseek(fd, 2 * 1024 * 1024, SEEK_SET) == 2 * 1024 * 1024 && read(fd, buffer, sizeof(buffer)) == 17,
        "large extent-backed tail");
  for (unsigned i = 0; i < 17; ++i) check(buffer[i] == (i * 37 + 11) % 256, "large data pattern");
  check(lseek(fd, 0, SEEK_END) == 2 * 1024 * 1024 + 17, "file length");
  close(fd);
  fd = file(root, "/fragmented");
  for (unsigned block = 0; block < 64; ++block)
  {
    check(lseek(fd, block * geometry.block_size, SEEK_SET) == block * geometry.block_size &&
          read(fd, buffer, 32) == 32, "external extent tree read");
    for (unsigned i = 0; i < 32; ++i)
      check(buffer[i] == (block < 16 && (block & 1) ? 0 : (i * 37 + 11) % 256), "holes/data");
  }
  close(fd);
  fd = file(root, "/unwritten");
  for (unsigned block = 0; block < 32; ++block)
  {
    check(lseek(fd, block * geometry.block_size, SEEK_SET) == block * geometry.block_size &&
          read(fd, buffer, 32) == 32, "unwritten read");
    for (unsigned i = 0; i < 32; ++i) check(!buffer[i], "unwritten physical bytes hidden");
  }
  close(fd);
  directory = file(root, "/large");
  unsigned count = 0;
  int bytes;
  while ((bytes = getdents64(directory, buffer, sizeof(buffer))) > 0)
  {
    for (unsigned at = 0; at < unsigned(bytes);)
    {
      auto *entry = reinterpret_cast<linux_dirent64 *>(buffer + at);
      check(entry->d_reclen >= DIRENT64_NAME_OFFSET + 1 && entry->d_reclen <= unsigned(bytes) - at,
            "directory record bounds");
      ++count; at += entry->d_reclen;
    }
  }
  check(!bytes && count == 1602, "complete indexed directory");
  close(directory);
  fd = file(root, "/message");
  check(write(fd, "x", 1) < 0 && ftruncate(fd, 0) < 0, "readonly descriptor");
  close(fd);
  name(root, "/message");
  check(open(path, O_WRONLY) < 0 && open(path, O_RDWR) < 0 && open(path, O_RDONLY | O_TRUNC) < 0,
        "readonly opens");
  check(unlink(path) < 0 && chmod(path, 0777) < 0, "readonly namespace/metadata");
  strcpy(other, path); name(root, "/created");
  check(open(path, O_CREATE | O_RDWR) < 0 && mkdir(path) < 0 && rename(other, path) < 0,
        "readonly creation and rename");
  int child = fork(); check(child >= 0, "fork");
  if (!child)
  {
    check(setuid(1000) == 0, "drop identity");
    name(root, "/private"); check(open(path, O_RDONLY) < 0, "private mode respected");
    message(root, "/message", "EXT4_READ_ONLY_OKAY\n");
    exit(0);
  }
  int status = -1;
  check(wait(&status) == child && !status, "nonroot access checks");
  printf("ext4test: PASS: extents, indexed directory, links, permissions and readonly mutations\n");
  return 0;
}
