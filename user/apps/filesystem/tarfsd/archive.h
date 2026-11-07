#pragma once
#include "kernel/fs/userfs/protocol.h"

namespace tarfs
{
constexpr unsigned Capacity = 128;

struct Node
{
  xv::userfs::Attributes attributes;
  unsigned parent = 1;
  unsigned long offset = 0;
  char name[256]{}, link[101]{};
};

extern Node nodes[Capacity];
extern unsigned used;
unsigned child(unsigned parent, const char *name);
bool load(int image);
int serve(int image, const char *type);
} // namespace tarfs
