#ifndef XVFS_EXT3_H
#define XVFS_EXT3_H
#include "control.h"
/* This interface is implemented by ext3.elf, not interpreted by the kernel. */
#define XVFS_EXT3_INTERFACE "ext3.native"
#define XVFS_EXT3_VERSION 1u
#define XVFS_EXT3_GEOMETRY 1u
#define XVFS_EXT3_INODE 2u
struct xvfs_ext3_geometry {
  uint32_t blocks, free_blocks, inodes, free_inodes, block_size;
  uint32_t compatible, incompatible, readonly_compatible, journal_inode;
  unsigned char uuid[16];
};
struct xvfs_ext3_inode {
  uint64_t size;
  uint32_t inode, flags, sectors, access_time, change_time, modification_time;
  uint32_t deletion_time, generation, file_acl_block;
};
#endif
