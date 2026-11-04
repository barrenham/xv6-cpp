#ifndef XVFS_EXT4_H
#define XVFS_EXT4_H
#include "control.h"
#define XVFS_EXT4_INTERFACE "ext4.native"
#define XVFS_EXT4_VERSION 1u
#define XVFS_EXT4_GEOMETRY 1u
struct xvfs_ext4_geometry {
  uint64_t blocks;
  uint32_t block_size, inodes, compatible, incompatible, readonly_compatible, driver_read_only;
};
#endif
