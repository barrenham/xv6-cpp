#include "ext3_image.h"
#include "kernel/fs/ext3/volume.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <new>

namespace
{
using namespace xv::ext3;
class MemoryIo final : public Io
{
  u8 *bytes_;u64 size_;
public:
  MemoryIo(void *bytes,u64 size):bytes_(static_cast<u8 *>(bytes)),size_(size) {}
  bool read(u64 offset,void *out,unsigned count) override
  {if(offset>size_ || count>size_-offset)return false;std::memcpy(out,bytes_+offset,count);return true;}
  bool write(u64 offset,const void *in,unsigned count) override
  {if(offset>size_ || count>size_-offset)return false;std::memcpy(bytes_+offset,in,count);return true;}
  bool flush() override {return true;}
  u64 size_bytes() const override {return size_;}
  u32 now_seconds() const override {return u32(std::time(nullptr));}
  void *allocate_page() override {return std::aligned_alloc(4096,4096);}
  void free_page(void *page) override {std::free(page);}
};
struct Image
{
  MemoryIo io;Volume volume;
  Image(void *bytes,u64 size):io(bytes,size),volume(io) {}
  ~Image(){volume.reset();}
};
Image &get(void *image){return *static_cast<Image *>(image);}
u16 disk_type(unsigned kind)
{
  switch(kind){case 1:return Directory;case 2:return Regular;case 3:return Character;case 5:return Symlink;case 6:return Block;default:return 0;}
}
unsigned host_type(u16 mode)
{
  switch(mode&0xf000){case Directory:return 1;case Regular:return 2;case Character:return 3;case Symlink:return 5;case Block:return 6;default:return 0;}
}
}
extern "C"
{
unsigned xv_ext3_abi(){return 1;}
void *xv_ext3_open(void *bytes,uint64_t length,unsigned *error)
{
  if(error)*error=unsigned(Error::Invalid);
  if(!bytes || length<2048)return nullptr;
  auto *image=new(std::nothrow) Image(bytes,length);
  if(!image){if(error)*error=unsigned(Error::NoMemory);return nullptr;}
  if(!image->volume.mount()){if(error)*error=unsigned(image->volume.error());delete image;return nullptr;}
  if(error)*error=0;return image;
}
void xv_ext3_close(void *image){delete static_cast<Image *>(image);}
unsigned xv_ext3_error(void *image){return unsigned(get(image).volume.error());}
int xv_ext3_info(void *image,uint32_t number,xv_ext3_metadata *metadata)
{
  Inode node;if(!get(image).volume.read_inode(number,node))return -1;
  *metadata={host_type(node.mode()),node.uid(),node.gid(),unsigned(node.mode()&07777),node.links(),node.device_major(),node.device_minor(),node.size()};return 0;
}
int xv_ext3_read(void *image,uint32_t number,uint64_t offset,void *bytes,unsigned count)
{
  auto &volume=get(image).volume;Inode node;
  return volume.read_inode(number,node) ? int(volume.read(node,offset,bytes,count)):-1;
}
int xv_ext3_next(void *image,uint32_t number,uint64_t *offset,char *name,unsigned *length,uint32_t *child)
{
  auto &volume=get(image).volume;Inode node;DirectoryEntry entry;u64 cursor=*offset;
  if(!volume.read_inode(number,node))return -1;
  if(!volume.next_entry(node,cursor,entry))return volume.error()==Error::None ? 0:-1;
  *offset=cursor;*length=entry.length;*child=entry.inode;std::memcpy(name,entry.name,entry.length+1);return 1;
}
int xv_ext3_attributes(void *image,uint32_t number,uint32_t uid,uint32_t gid,unsigned mode)
{
  auto &volume=get(image).volume;Inode node;
  return mode<=07777 && volume.read_inode(number,node) && volume.set_attributes(node,uid,gid,u16(mode)) && volume.journal().commit() ? 0:-1;
}
int xv_ext3_create(void *image,uint32_t parent,const char *name,unsigned kind,uint32_t uid,uint32_t gid,unsigned mode,uint32_t *child)
{
  auto &volume=get(image).volume;Inode node;auto type=disk_type(kind);
  if(!type || mode>07777 || !volume.create(parent,name,u16(type|mode),uid,gid,node) || !volume.journal().commit())return -1;
  *child=node.number;return 0;
}
int xv_ext3_link(void *image,uint32_t parent,const char *name,uint32_t child)
{
  auto &volume=get(image).volume;
  return volume.link(parent,name,child) && volume.journal().commit() ? 0:-1;
}
int xv_ext3_unlink(void *image,uint32_t parent,const char *name)
{
  auto &volume=get(image).volume;u32 number;Inode node;
  if(!volume.unlink(parent,name,number) || !volume.read_inode(number,node))return -1;
  if(!volume.journal().commit())return -1;
  if(!node.links())
  {
    bool complete=false;
    while(!complete)if(!volume.reclaim_step(number,complete) || !volume.journal().commit())return -1;
  }
  return 0;
}
int xv_ext3_write(void *image,uint32_t number,uint64_t offset,const void *bytes,uint64_t count)
{
  if(count>UINT64_MAX-offset || (!bytes && count))return -1;
  auto &volume=get(image).volume;Inode node;if(!volume.read_inode(number,node))return -1;
  auto *input=static_cast<const u8 *>(bytes);
  while(count)
  {
    unsigned take=count>3072 ? 3072:unsigned(count);
    if(volume.write(node,offset,input,take)!=long(take) || !volume.journal().commit())return -1;
    offset+=take;input+=take;count-=take;
  }
  return 0;
}
int xv_ext3_device(void *image,uint32_t number,uint32_t major,uint32_t minor)
{
  auto &volume=get(image).volume;Inode node;
  return volume.read_inode(number,node) && volume.set_device(node,major,minor) && volume.journal().commit() ? 0:-1;
}
}
