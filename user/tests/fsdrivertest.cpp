#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "user/fs/sdk/include/xvfs/ext3.h"
#include "examples/fs-plugin/driver.h"

namespace
{
void check(bool okay, const char *what)
{
  if (!okay) { printf("fsdrivertest: FAIL: %s\n", what); exit(1); }
}
void request(xvfs_control &r, const char *interface_name, unsigned command)
{
  memset(&r, 0, sizeof(r));
  r.size = sizeof(r); r.version = XVFS_CONTROL_VERSION; r.driver_version = 1;
  r.capacity = sizeof(r.data); r.command = command;
  strcpy(r.interface_name, interface_name);
}
void boundaries(int fd)
{
  xvfs_control r;
  request(r, XVFS_DRIVER_INTERFACE, 0);
  r.version = 99;
  check(xvfs_driver_control(fd, &r) < 0, "carrier version");
  request(r, XVFS_DRIVER_INTERFACE, 0); r.input_size = 513;
  check(xvfs_driver_control(fd, &r) < 0, "input bound");
  request(r, XVFS_DRIVER_INTERFACE, 0); r.capacity = 513;
  check(xvfs_driver_control(fd, &r) < 0, "output bound");
  request(r, XVFS_DRIVER_INTERFACE, 0); r.capacity = 1;
  check(xvfs_driver_control(fd, &r) < 0, "small result buffer");
  request(r, "missing.interface", 0);
  check(xvfs_driver_control(fd, &r) < 0, "unknown namespace");
  request(r, XVFS_DRIVER_INTERFACE, 0); r.driver_version = 99;
  check(xvfs_driver_control(fd, &r) < 0, "driver version");
  request(r, XVFS_DRIVER_INTERFACE, 0); memset(r.interface_name, 'x', sizeof(r.interface_name));
  check(xvfs_driver_control(fd, &r) < 0, "unterminated namespace");
  check(ioctl(fd, XVFS_CONTROL_IOCTL, ~0UL) < 0, "invalid user pointer");
  request(r, XVFS_DRIVER_INTERFACE, 0);
  check(xvfs_driver_control(fd, &r) == sizeof(xvfs_driver_info), "connection survives rejected commands");
  for (unsigned i = r.output_size; i < sizeof(r.data); ++i)
    check(!r.data[i], "unused response bytes cleared");
}
void ext3(int fd)
{
  xvfs_control r;
  request(r, XVFS_DRIVER_INTERFACE, 0);
  check(xvfs_driver_control(fd, &r) == sizeof(xvfs_driver_info), "ext3 discovery");
  xvfs_driver_info driver;
  memmove(&driver, r.data, sizeof(driver));
  check(driver.size == sizeof(driver) && driver.version == 1 &&
        !strcmp(driver.interface_name, XVFS_EXT3_INTERFACE) && driver.interface_version == 1 &&
        (driver.operations & XVFS_OP_BIT(XVFS_CONTROL)), "ext3 capabilities");
  request(r, XVFS_EXT3_INTERFACE, XVFS_EXT3_GEOMETRY);
  check(xvfs_driver_control(fd, &r) == sizeof(xvfs_ext3_geometry), "real superblock query");
  xvfs_ext3_geometry geometry;
  memmove(&geometry, r.data, sizeof(geometry));
  check(geometry.blocks > geometry.free_blocks && geometry.inodes > geometry.free_inodes &&
        geometry.journal_inode && geometry.block_size >= 1024 && geometry.block_size <= 4096,
        "ext3 allocation and journal geometry");
  request(r, XVFS_EXT3_INTERFACE, XVFS_EXT3_INODE);
  check(xvfs_driver_control(fd, &r) == sizeof(xvfs_ext3_inode), "native inode query");
  xvfs_ext3_inode inode;
  memmove(&inode, r.data, sizeof(inode));
  struct stat st;
  check(fstat(fd, &st) == 0 && inode.size == st.size &&
        inode.inode == (st.ino == 1 ? 2u : st.ino), "inode identity and full size");
  boundaries(fd);
}
const char persisted[] = "NATIVE_WRITE_PERSISTED\n";
void external(int fd, const char *path, bool readonly)
{
  xvfs_control r;
  request(r, XVFS_DRIVER_INTERFACE, XVFS_DRIVER_DESCRIBE);
  check(xvfs_driver_control(fd, &r) == sizeof(xvfs_driver_info), "negotiated discovery reply");
  xvfs_driver_info description;
  memmove(&description, r.data, sizeof(description));
  check(description.max_payload == 64 && !strcmp(description.interface_name, HELLO_INTERFACE),
        "driver-specific negotiated payload and namespace");
  request(r, HELLO_INTERFACE, HELLO_STATUS);
  check(xvfs_driver_control(fd, &r) == sizeof(hello_status), "external command");
  hello_status status;
  memmove(&status, r.data, sizeof(status));
  check(status.uid == unsigned(getuid()) && status.pid == unsigned(getpid()) &&
        bool(status.access & XVFS_ACCESS_READ_ONLY) == readonly, "authenticated context");
  request(r, HELLO_INTERFACE, HELLO_WRITE_PREFIX);
  r.input_size = sizeof(persisted) - 1; memmove(r.data, persisted, r.input_size);
  check(xvfs_driver_control(fd, &r) < 0, "read descriptor cannot issue native writes");
  boundaries(fd);
  request(r, HELLO_INTERFACE, 65535); r.capacity = 1;
  check(xvfs_driver_control(fd, &r) < 0, "malformed driver output bounded by caller capacity");
  request(r, HELLO_INTERFACE, HELLO_STATUS);
  check(xvfs_driver_control(fd, &r) == sizeof(hello_status), "recovery after bad driver reply");
  if (readonly) return;
  int writable = open(path, O_RDWR);
  check(writable >= 0, "open writable extension descriptor");
  request(r, HELLO_INTERFACE, HELLO_WRITE_PREFIX);
  r.input_size = sizeof(persisted) - 1; memmove(r.data, persisted, r.input_size);
  check(xvfs_driver_control(writable, &r) == 0, "driver-specific durable mutation");
  check(lseek(fd, 0, SEEK_END) >= sizeof(persisted) - 1 && lseek(fd, 0, SEEK_SET) == 0,
        "native mutation refreshes VFS size without a stat call");
  char buffer[sizeof(persisted)]{};
  check(read(fd, buffer, sizeof(persisted) - 1) == sizeof(persisted) - 1 &&
        !memcmp(buffer, persisted, sizeof(persisted) - 1), "native operation visible through VFS");
  int child = fork();
  check(child >= 0, "fork identity check");
  if (!child)
  {
    check(setuid(1000) == 0, "drop identity");
    request(r, HELLO_INTERFACE, HELLO_STATUS);
    check(xvfs_driver_control(writable, &r) == sizeof(hello_status), "unprivileged context query");
    memmove(&status, r.data, sizeof(status));
    check(status.uid == 1000 && (status.access & XVFS_ACCESS_WRITABLE), "inherited descriptor context");
    request(r, HELLO_INTERFACE, HELLO_WRITE_PREFIX);
    r.input_size = 1; r.data[0] = 'X';
    check(xvfs_driver_control(writable, &r) < 0, "driver authorizes its own native command");
    exit(0);
  }
  int result = -1;
  check(wait(&result) == child && result == 0, "native permission isolation");
  close(writable);
}
}
extern "C" int main(int argc, char **argv)
{
  check(argc == 3, "mode and path required");
  int fd = open(argv[2], O_RDONLY);
  check(fd >= 0, "open target");
  if (!strcmp(argv[1], "ext3")) ext3(fd);
  else if (!strcmp(argv[1], "external") || !strcmp(argv[1], "readonly"))
    external(fd, argv[2], !strcmp(argv[1], "readonly"));
  else
  {
    xvfs_control r;
    request(r, XVFS_DRIVER_INTERFACE, 0);
    check(xvfs_driver_control(fd, &r) < 0, "legacy driver rejects unsupported extensions");
  }
  close(fd);
  printf("fsdrivertest: PASS: %s\n", argv[1]);
  return 0;
}
