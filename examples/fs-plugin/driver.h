#ifndef HELLO_DRIVER_H
#define HELLO_DRIVER_H
/* A third-party command schema. The kernel does not include this header. */
#define HELLO_INTERFACE "example.file"
#define HELLO_STATUS 1u
#define HELLO_WRITE_PREFIX 2u
struct hello_status {
  unsigned size, pid, uid, gid, access;
};
#endif
