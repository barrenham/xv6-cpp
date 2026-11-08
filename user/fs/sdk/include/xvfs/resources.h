#ifndef XVFS_RESOURCES_H
#define XVFS_RESOURCES_H
#include "plugin.h"
#define XVFS_RESOURCE_VERSION 2u
#define XVFS_RESOURCE_QUERY 0x58560002u
#define XVFS_RESOURCE_CLONE 0x58560003u
#define XVFS_RESOURCE_STORAGE 0x58560004u

enum xvfs_resource_operation {
  XVFS_DEVICE = 1, XVFS_PTY, XVFS_RETAIN_PTY, XVFS_RELEASE_PTY, XVFS_INSPECT_PTY,
  XVFS_PROCESS_AFTER, XVFS_CPU, XVFS_SYSTEM, XVFS_MOUNT, XVFS_PROVIDER,
  XVFS_FILESYSTEM, XVFS_CACHE, XVFS_KERNEL_VERSION, XVFS_CAPTURE_DESCRIPTORS,
  XVFS_DESCRIPTOR,
  XVFS_CAPTURE_EXEC,
  XVFS_READ_EXEC,
  XVFS_END_EXEC,
  XVFS_FILE_CACHE,
  XVFS_THREAD_AFTER
};
struct xvfs_resource_query {
  uint32_t size, version, operation, index;
  int32_t pid, result;
  uint64_t offset;
  uint32_t count, inspector;
#ifdef __cplusplus
  alignas(8)
#else
  _Alignas(8)
#endif
  unsigned char data[512];
};

struct xvfs_thread
{
  int32_t tid, pid, parent;
  uint64_t cpu_milliseconds, bytes;
  char state, name[16];
};
struct xvfs_system {
  uint64_t total_kib, free_kib, uptime, boot_epoch, epoch, switches, processes;
};

struct xvfs_file_cache
{
  uint64_t pages, pending, idle_evictions, mapped_evictions, target;
};
#if defined(__riscv) && __riscv_xlen == 64
static inline int xvfs_ioctl(int fd, unsigned request, uintptr_t argument)
{ return (int)xvfs_syscall(40, fd, request, (long)argument); }
#endif
#endif
