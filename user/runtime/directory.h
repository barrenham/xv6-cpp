#pragma once
#include "kernel/io/dirent.h"
#include "user/runtime/user.h"
class DirectoryReader final
{
public:
  explicit DirectoryReader(int descriptor):descriptor_(descriptor) {}
  int next(const linux_dirent64 *&entry)
  {
    entry=nullptr;if(failed_)return -1;
    if(offset_==used_)
    {
      int count=getdents64(descriptor_,bytes_,sizeof(bytes_));
      if(count<0){failed_=true;return -1;}if(!count)return 0;
      if(unsigned(count)>sizeof(bytes_)){failed_=true;return -1;}
      offset_=0;used_=count;
    }
    if(used_-offset_<24){failed_=true;return -1;}
    auto *record=reinterpret_cast<const linux_dirent64 *>(bytes_+offset_);
    unsigned size=record->d_reclen;
    if(size<24 || size%8 || size>used_-offset_){failed_=true;return -1;}
    unsigned length=0;while(DIRENT64_NAME_OFFSET+length<size && record->d_name[length])++length;
    if(length>255 || DIRENT64_NAME_OFFSET+length>=size){failed_=true;return -1;}
    offset_+=size;entry=record;return 1;
  }
private:
  int descriptor_;
  unsigned used_=0,offset_=0;
  bool failed_=false;
  alignas(8) unsigned char bytes_[512];
};
