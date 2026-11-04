#ifndef XVFS_CONTROL_H
#define XVFS_CONTROL_H
#include "plugin.h"
#define XVFS_CONTROL_IOCTL 0x58560007u
#define XVFS_CONTROL_VERSION 1u
#define XVFS_ACCESS_READABLE 1u
#define XVFS_ACCESS_WRITABLE 2u
#define XVFS_ACCESS_READ_ONLY 4u
#define XVFS_DRIVER_INTERFACE "xv.fs.driver"
#define XVFS_DRIVER_DESCRIBE 0u
struct xvfs_control {
  uint32_t size, version, driver_version, command;
  uint32_t input_size, capacity, output_size;
  char interface_name[32];
  unsigned char data[512];
};
struct xvfs_driver_info {
  uint32_t size, version, wire_version, flags;
  uint64_t operations;
  uint32_t interface_version, max_payload;
  char interface_name[32];
};
#if defined(__riscv) && __riscv_xlen == 64
static inline int xvfs_driver_control(int fd, struct xvfs_control *request)
{ return (int)xvfs_syscall(40, fd, XVFS_CONTROL_IOCTL, (long)request); }
#endif
#endif
