module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/io/block.h"
#include "kernel/io/dirent.h"
module xv.fs;
import :inode_backend_flow;
import :platform;
import :operations;

namespace xv::fs
{
namespace
{
void legacy_record(const dirent &entry,DirectoryRecord &record)
{
  record={};record.inode=entry.inum;unsigned length=0;
  while(length<DIRSIZ && entry.name[length])++length;
  detail::move(record.name,entry.name,length);record.name[length]=0;
}
}
int FileSystem::legacy_next_directory(Inode *node,unsigned long &cookie,DirectoryRecord &record)
{
  if(node->kind()!=T_DIR)return -1;
  unsigned long next=cookie;dirent entry{};
  for(;;)
  {
    if(next>0xffffffffUL-sizeof(entry))return -1;
    int count=read(node,0,reinterpret_cast<unsigned long>(&entry),next,sizeof(entry));
    if(!count){cookie=next;return 0;}
    if(count!=int(sizeof(entry)))return -1;
    next+=sizeof(entry);if(!entry.inum)continue;
    legacy_record(entry,record);cookie=next;return 1;
  }
}
int InodeBackend::getdents64(xv::io::Buffer destination,int count)
{
  LockedInode guard(node_->owner(),*node_);
  if(node_->kind()!=T_DIR || directory_mode_==1 || count<0)return -1;
  if(!count)return 0;
  if(count<24)return -1;
  DirectoryFlow::Context context{*this,destination,count};
  (void)DirectoryFlow::Graph::run<0>(context);
  return context.done ? context.done:(context.result==0 ? 0:-1);
}

void InodeBackend::close()
{
  {
    Transaction transaction(node_->owner().persistent());
    filesystem().release(node_);
  }
  this->~InodeBackend();
}

int InodeBackend::read(xv::io::Buffer destination, int count)
{
  LockedInode guard(node_->owner(), *node_);
  if(node_->kind()==T_DIR && directory_mode_==2)return -1;
  ReadFlow::Context context{*this, destination, count};
  (void)ReadFlow::Graph::run<0>(context);
  if(node_->kind()==T_DIR && context.result>0)directory_mode_=1;
  return context.result;
}

int InodeBackend::write(xv::io::Buffer source, int count)
{
  WriteFlow::Context context{{*this}, source, count};
  (void)WriteFlow::Graph::run<0>(context);
  return context.done ? context.done : (count == 0 ? 0 : -1);
}

long InodeBackend::seek(long offset, int origin)
{
  LockedInode guard(node_->owner(), *node_);
  SeekFlow::Context context{*this, offset, origin};
  (void)SeekFlow::Graph::run<0>(context);
  return context.result;
}

int InodeBackend::resize(unsigned size)
{
  Transaction transaction(node_->owner().persistent());
  LockedInode guard(node_->owner(), *node_);
  ResizeFlow::Context context{{*this}, size};
  (void)ResizeFlow::Graph::run<0>(context);
  return context.result;
}

int InodeBackend::sync()
{
  SyncFlow::Context context{*this};
  (void)SyncFlow::Graph::run<0>(context);
  return context.result;
}

int InodeBackend::stat(xv::io::Metadata &value)
{
  LockedInode guard(node_->owner(), *node_);
  filesystem().stat(node_, &value);
  if (snapshot_)
  {
    value.size = contents_.size();
  }
  return 0;
}

unsigned InodeBackend::poll(xv::sync::PollTable *)
{
  return POLLIN | POLLOUT;
}

void BlockBackend::close()
{
  {Transaction transaction(node_->owner().persistent());filesystem().release(node_);}
  this->~BlockBackend();
}
int BlockBackend::transfer(xv::io::Buffer buffer,int count,bool writing)
{
  if(count<0 || (writing && !writable_))return -1;
  if(!count)return 0;
  LockedInode guard(node_->owner(),*node_);
  unsigned long capacity=disk_->capacity_sectors()*512,offset=position();
  if(offset>=capacity)return writing ? -1:0;
  if(unsigned(count)>capacity-offset)count=int(capacity-offset);
  struct Scratch
  {
    void *page=detail::allocate_page();
    ~Scratch(){if(page)detail::free_page(page);}
  } scratch;
  if(!scratch.page)return -1;
  // The sector is the publication unit: never submit a partly copied user
  // sector. Partial writes preserve both untouched edges with read/modify/write.
  TransferFlow::Context context{*this,buffer,count,writing,offset,static_cast<unsigned char *>(scratch.page)};
  (void)TransferFlow::Graph::run<0>(context);
  return context.done ? context.done:-1;
}
int BlockBackend::read(xv::io::Buffer destination,int count){return transfer(destination,count,false);}
int BlockBackend::write(xv::io::Buffer source,int count){return transfer(source,count,true);}
long BlockBackend::seek(long delta,int origin)
{
  LockedInode guard(node_->owner(),*node_);
  if(origin<SEEK_SET || origin>SEEK_END)return -1;
  long capacity=disk_->capacity_sectors()*512;
  long base=origin==SEEK_SET ? 0:origin==SEEK_CUR ? position():capacity;
  if(delta < -base || delta>capacity-base)return -1;
  auto result=base+delta;__atomic_store_n(&offset_,result,__ATOMIC_RELEASE);return result;
}
int BlockBackend::stat(xv::io::Metadata &value)
{
  LockedInode guard(node_->owner(),*node_);filesystem().stat(node_,&value);
  value.size=disk_->capacity_sectors()*512;return 0;
}
int BlockBackend::sync()
{
  journal().sync();if(Transaction::secondary)Transaction::secondary->sync();disk_->flush();return 0;
}
unsigned BlockBackend::poll(xv::sync::PollTable *){return POLLIN | (writable_ ? POLLOUT:0);}
int BlockBackend::ioctl(unsigned request,xv::io::Buffer argument)
{
  if(request==BLKGETSIZE64)
  {unsigned long size=disk_->capacity_sectors()*512;return detail::copy_out(true,argument.address,&size,sizeof(size));}
  int value;
  if(request==BLKSSZGET)value=512;
  else if(request==BLKROGET)
  {auto *entry=devices().block(node_->device_major(),node_->device_minor());value=!entry || entry->read_only;}
  else return -1;
  return detail::copy_out(true,argument.address,&value,sizeof(value));
}

void DeviceBackend::close()
{
  session_->close();
  {
    Transaction transaction(node_->owner().persistent());
    filesystem().release(node_);
  }
  this->~DeviceBackend();
}

int DeviceBackend::read(xv::io::Buffer destination, int count)
{
  auto *device = session_;
  return device ? device->read({xv::drivers::AddressSpace::User, destination.address,
                                destination.nonblocking},
                               count)
                : -1;
}

int DeviceBackend::write(xv::io::Buffer source, int count)
{
  auto *device = session_;
  return device ? device->write({xv::drivers::AddressSpace::User, source.address,
                                 source.nonblocking},
                                count)
                : -1;
}

int DeviceBackend::stat(xv::io::Metadata &value)
{
  LockedInode guard(node_->owner(), *node_);
  filesystem().stat(node_, &value);
  return 0;
}

unsigned DeviceBackend::poll(xv::sync::PollTable *table)
{
  auto *device = session_;
  return device ? device->poll(table) : POLLERR;
}

int DeviceBackend::ioctl(unsigned request, xv::io::Buffer argument)
{
  auto *device = session_;
  return device ? device->ioctl(request, {xv::drivers::AddressSpace::User,
                                          argument.address, argument.nonblocking})
                : -1;
}

void PipeBackend::close()
{
  pipe_->close(writable_);
  this->~PipeBackend();
}

int PipeBackend::read(xv::io::Buffer destination, int count)
{
  return pipe_->read(destination.address, count, destination.nonblocking);
}

int PipeBackend::write(xv::io::Buffer source, int count)
{
  return pipe_->write(source.address, count, source.nonblocking);
}

unsigned PipeBackend::poll(xv::sync::PollTable *table)
{
  return pipe_->poll(writable_, table);
}

void EndpointBackend::close()
{
  endpoint_->close();
  this->~EndpointBackend();
}

int EndpointBackend::read(xv::io::Buffer destination, int count)
{
  return endpoint_->read(
      {xv::drivers::AddressSpace::User, destination.address, destination.nonblocking},
      count);
}

int EndpointBackend::write(xv::io::Buffer source, int count)
{
  return endpoint_->write(
      {xv::drivers::AddressSpace::User, source.address, source.nonblocking}, count);
}

int EndpointBackend::stat(xv::io::Metadata &value)
{
  value = {};
  value.type = static_cast<short>(xv::io::MetadataKind::Device);
  value.nlink = 1;
  return 0;
}

unsigned EndpointBackend::poll(xv::sync::PollTable *table)
{
  return endpoint_->poll(table);
}

int EndpointBackend::ioctl(unsigned request, xv::io::Buffer argument)
{
  return endpoint_->ioctl(request, {xv::drivers::AddressSpace::User, argument.address,
                                    argument.nonblocking});
}
} // namespace xv::fs
