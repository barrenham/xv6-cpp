module;
#include "kernel/fs/format/layout.h"
module xv.fs;
import :disk;
import :platform;

namespace xv::fs
{
void DiskFileSystem::initialize(int device)
{
  {
    auto block=cache_.read(unsigned(device),1);const auto *bytes=block->bytes();
    unsigned magic=unsigned(bytes[0])|(unsigned(bytes[1])<<8)|(unsigned(bytes[2])<<16)|(unsigned(bytes[3])<<24);
    if(magic==FSMAGIC)selected_=&legacy_;
    else if(bytes[56]==0x53 && bytes[57]==0xef)selected_=&ext3_;
    else detail::fail("disk: unknown filesystem format");
  }
  selected_->initialize(device);
}
}
