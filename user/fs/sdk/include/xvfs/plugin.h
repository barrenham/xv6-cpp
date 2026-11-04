#ifndef XVFS_PLUGIN_H
#define XVFS_PLUGIN_H
#include <stdint.h>
#include <stddef.h>

/* Process ABI 1; wire ABI 3. These structures contain no process pointers. */
#define XVFS_PLUGIN_ABI 1u
#define XVFS_WIRE_VERSION 3u
#define XVFS_DATA_SIZE 512u
#define XVFS_NAME_SIZE 256u
#define XVFS_READ_ONLY 1u
#define XVFS_SNAPSHOTS 2u
#define XVFS_SOURCE_FD 3
#define XVFS_RESOURCES_FD 4
#define XVFS_READY_FD 5
#define XVFS_ACTIVATION_FD 6
#define XVFS_IMAGE_FD 7

enum xvfs_operation {
  XVFS_LOOKUP = 1, XVFS_CREATE, XVFS_READ, XVFS_WRITE, XVFS_RESIZE,
  XVFS_SETATTR, XVFS_READDIR, XVFS_LINK, XVFS_UNLINK, XVFS_RENAME,
  XVFS_SYNC, XVFS_GETATTR, XVFS_FORGET, XVFS_SNAPSHOT_BEGIN,
  XVFS_SNAPSHOT_READ, XVFS_SNAPSHOT_END, XVFS_CONTROL
};
#define XVFS_OP_BIT(op) (UINT64_C(1) << (op))
#define XVFS_READ_OPERATIONS (XVFS_OP_BIT(XVFS_LOOKUP) | XVFS_OP_BIT(XVFS_READ) | \
  XVFS_OP_BIT(XVFS_READDIR) | XVFS_OP_BIT(XVFS_SYNC) | XVFS_OP_BIT(XVFS_GETATTR) | XVFS_OP_BIT(XVFS_FORGET))

struct xvfs_attributes {
  uint32_t node, size, uid, gid, mode, type, links, major, minor;
};
struct xvfs_context { uint32_t pid, uid, gid; };
struct xvfs_message {
  uint32_t version, operation;
  uint64_t id;
  uint32_t node, other;
  uint64_t offset;
  int32_t result;
  uint32_t count;
  struct xvfs_context context;
  struct xvfs_attributes attributes;
  char name[XVFS_NAME_SIZE], target[XVFS_NAME_SIZE];
  unsigned char data[XVFS_DATA_SIZE];
};
struct xvfs_registration {
  uint32_t size, version, flags, max_payload;
  uint64_t operations;
  char type[32];
  struct xvfs_attributes root;
};

#if defined(__riscv) && __riscv_xlen == 64
static inline long xvfs_syscall(unsigned number, long first, long second, long third)
{
  register long a0 __asm__("a0") = first;
  register long a1 __asm__("a1") = second;
  register long a2 __asm__("a2") = third;
  register long a7 __asm__("a7") = number;
  __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");
  return a0;
}
static inline int xvfs_register(struct xvfs_registration *registration)
{ return (int)xvfs_syscall(95, (long)registration, 0, 0); }
static inline int xvfs_read(int fd, void *buffer, unsigned count)
{ return (int)xvfs_syscall(5, fd, (long)buffer, count); }
static inline int xvfs_write(int fd, const void *buffer, unsigned count)
{ return (int)xvfs_syscall(16, fd, (long)buffer, count); }
static inline void xvfs_close(int fd) { (void)xvfs_syscall(21, fd, 0, 0); }
static inline long xvfs_seek(int fd, long offset, int origin)
{ return xvfs_syscall(88, fd, offset, origin); }
#endif
#endif
