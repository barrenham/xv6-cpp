#pragma once
#include "kernel/support/types.h"
#define T_DIR    1 // Directory
#define T_FILE   2 // File
#define T_DEVICE 3 // Device
#define T_SOCKET 4
#define T_SYMLINK 5
#define T_BLOCK 6 // Block device

struct stat {
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short type;  // Type of file
  short nlink; // Number of links to file
  uint64 size; // Size of file in bytes
  uint uid, gid;
  uint mode;
};
