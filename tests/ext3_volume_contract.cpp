#include "kernel/fs/ext3/volume.h"
#include <cassert>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <vector>
using namespace xv::ext3;
struct Device final : Io
{
  int fd,live=0,cut=-1,events=0,allocation_budget=-1;u64 length;bool committed=false;
  explicit Device(const char *path)
  {fd=open(path,O_RDWR);assert(fd>=0);struct stat st{};assert(!fstat(fd,&st));length=st.st_size;}
  ~Device(){assert(!live);close(fd);}
  bool read(u64 offset,void *data,unsigned count) override
  {return offset<=length && count<=length-offset && pread(fd,data,count,off_t(offset))==count;}
  bool write(u64 offset,const void *data,unsigned count) override
  {
    if(cut>=0 && events++>=cut)return false;
    if(offset>length || count>length-offset || pwrite(fd,data,count,off_t(offset))!=count)return false;
    if(count>=12 && be32(static_cast<const u8 *>(data))==JournalMagic && be32(static_cast<const u8 *>(data)+4)==Commit)committed=true;
    return true;
  }
  bool flush() override {if(cut>=0 && events++>=cut)return false;return !fsync(fd);}
  u64 size_bytes() const override {return length;}
  u32 now_seconds() const override {return 0;}
  void *allocate_page() override
  {if(allocation_budget==0)return nullptr;if(allocation_budget>0)--allocation_budget;auto *p=std::aligned_alloc(4096,4096);if(p)++live;return p;}
  void free_page(void *p) override {--live;std::free(p);}
};
struct OrphanExpected {u32 blocks,inodes,number;};
OrphanExpected sparse_orphan(Volume &volume,unsigned leaves=300)
{
  Inode inode;assert(volume.create(2,"large-orphan",Regular|0600,0,0,inode) && volume.journal().commit());
  u64 fan=volume.geometry().block_size/4;
  for(unsigned i=0;i<leaves;++i)
  {
    u64 offset=(12+fan+u64(i)*fan)*volume.geometry().block_size;
    assert(volume.write(inode,offset,"x",1)==1 && volume.journal().commit());
  }
  u32 removed;assert(volume.unlink(2,"large-orphan",removed) && volume.journal().commit());
  return {volume.geometry().free_blocks+inode.sectors()/(volume.geometry().block_size/512),volume.geometry().free_inodes+1,inode.number};
}
int main(int argc,char **argv)
{
  assert(argc==2 || argc==4);Device device(argv[1]);Volume volume(device);
  if(argc==4 && !std::strcmp(argv[2],"--orphan-cut"))
  {
    device.cut=std::atoi(argv[3]);bool recovered=volume.mount();
    std::printf("%u %d\n",unsigned(recovered),device.events);volume.reset();return 0;
  }
  if(!volume.mount()){std::fprintf(stderr,"ext3 mount error %u\n",unsigned(volume.error()));return 1;}
  if(argc==4)
  {
    if(!std::strcmp(argv[2],"--orphan-prepare"))
    {
      auto expected=sparse_orphan(volume);std::printf("%u %u\n",expected.blocks,expected.inodes);
    }
    else if(!std::strcmp(argv[2],"--orphan-check"))
    {
      unsigned head;assert(volume.orphan_head(head) && !head);
      std::printf("%u %u\n",volume.geometry().free_blocks,volume.geometry().free_inodes);
    }
    else if(!std::strcmp(argv[2],"--crash"))
    {
      Inode file;assert(volume.create(2,"atomic-crash",Regular|0644,0,0,file));
      assert(volume.write(file,0,"durable",7)==7);
      device.cut=std::atoi(argv[3]);bool complete=volume.journal().commit();
      std::printf("%u %u %d\n",unsigned(complete),unsigned(device.committed),device.events);
    }
    else
    {
      assert(!std::strcmp(argv[2],"--recover"));Inode root,file;
      assert(volume.read_inode(2,root));bool present=volume.lookup(root,"atomic-crash",file);
      assert(present || volume.error()==Error::NotFound);
      if(std::atoi(argv[3]))assert(present);
      if(present){char data[8]{};assert(file.size()==7 && volume.read(file,0,data,8)==7 && !std::strcmp(data,"durable"));}
    }
    volume.reset();return 0;
  }
  assert(volume.geometry().block_size==1024 || volume.geometry().block_size==2048 || volume.geometry().block_size==4096);
  Inode root,file,link;assert(volume.read_inode(2,root));assert(volume.lookup(root,"payload",file));
  std::vector<unsigned char> bytes(2*1024*1024+17);assert(volume.read(file,0,bytes.data(),unsigned(bytes.size()))==long(bytes.size()));
  for(unsigned i=0;i<bytes.size();++i)assert(bytes[i]==u8(i*37+11));
  assert(!volume.read(file,bytes.size(),bytes.data(),1));
  assert(volume.lookup(root,"shortlink",link));char target[64]{};
  assert(volume.read(link,0,target,sizeof(target))==7 && !std::strcmp(target,"payload"));
  DirectoryEntry entry;u64 offset=0;unsigned count=0;
  while(volume.next_entry(root,offset,entry))++count;
  assert(count>=5 && volume.error()==Error::None);
  Inode directory,created,symlink;
  assert(volume.create(2,"native-dir",Directory|0750,70000,80000,directory));
  assert(volume.create(directory.number,"native-data",Regular|0640,70000,80000,created));
  assert(volume.journal().commit());
  // Cross direct/single/double boundaries using the actual mutator, committing
  // small user-write-sized batches rather than relying on the host formatter.
  for(unsigned at=0;at<bytes.size();)
  {
    unsigned take=std::min<unsigned>(3072,unsigned(bytes.size())-at);
    assert(volume.write(created,at,bytes.data()+at,take)==take);
    assert(volume.journal().commit());at+=take;
  }
  assert(volume.set_attributes(created,123456,234567,06754));
  assert(volume.link(2,"native-hard",created.number));
  // 'created' predates the hard link: a subsequent write must refresh i_links.
  assert(volume.write(created,0,bytes.data(),1)==1 && created.links()==2);
  assert(volume.journal().commit());
  u32 replaced=0;
  assert(volume.rename(directory.number,"native-data",2,"native-renamed",replaced) && !replaced);
  assert(volume.journal().commit());
  assert(volume.read_inode(created.number,created) && created.links()==2 && created.uid()==123456 && created.gid()==234567);
  std::vector<unsigned char> check(bytes.size());assert(volume.read(created,0,check.data(),unsigned(check.size()))==long(check.size()) && check==bytes);
  assert(volume.resize(created,17) && volume.journal().commit());
  assert(volume.resize(created,volume.geometry().block_size*4+9) && volume.journal().commit());
  check.assign(unsigned(created.size()),0xa7);assert(volume.read(created,0,check.data(),unsigned(check.size()))==long(check.size()));
  assert(std::equal(bytes.begin(),bytes.begin()+17,check.begin()));
  for(unsigned i=17;i<check.size();++i)assert(!check[i]);
  u32 removed=0;
  assert(volume.unlink(2,"native-hard",removed) && removed==created.number && volume.journal().commit());
  assert(volume.unlink(2,"native-renamed",removed) && volume.journal().commit());
  // The unlinked inode remains readable while open, until final reclaim.
  assert(volume.read_inode(created.number,created) && !created.links());
  assert(volume.read(created,0,check.data(),17)==17);
  assert(volume.reclaim(created.number) && volume.journal().commit());
  assert(volume.unlink(2,"native-dir",removed) && removed==directory.number);
  assert(volume.reclaim(directory.number) && volume.journal().commit());
  assert(volume.create(2,"native-link",Symlink|0777,0,0,symlink));
  assert(volume.write(symlink,0,"payload",7)==7 && volume.journal().commit());
  assert(volume.unlink(2,"native-link",removed) && volume.reclaim(removed) && volume.journal().commit());
  // Leave these alive for independent e2fsck validation: deleting every test
  // symlink before checking the image would hide an invalid wire encoding.
  for(unsigned length:{7U,59U,60U,127U})
  {
    char name[32],target[128];std::snprintf(name,sizeof(name),"kept-link-%u",length);
    std::memset(target,'s',sizeof(target));
    assert(volume.create(2,name,Symlink|0777,0,0,symlink));
    assert(volume.write(symlink,0,target,length)==long(length) && volume.journal().commit());
    assert(symlink.inline_link()==(length<60));
    char result[128]{};assert(volume.read(symlink,0,result,sizeof(result))==long(length) && !std::memcmp(target,result,length));
  }
  for(auto type:{Character,Block})for(auto major:{1U,4095U})for(auto minor:{3U,1048575U})
  {
    Inode device_node;
    assert(volume.create(2,"device-node",u16(type|0600),0,0,device_node));
    assert(volume.set_device(device_node,major,minor) && volume.journal().commit());
    assert(volume.read_inode(device_node.number,device_node));
    assert(device_node.device_major()==major && device_node.device_minor()==minor);
    assert(volume.unlink(2,"device-node",removed) && volume.reclaim(removed) && volume.journal().commit());
  }
  // Directory moves update '..' and both parents' link counts. Overwriting a
  // directory leaves its old, open inode alive on the ext3 orphan chain.
  Inode left,right,child,overwritten,parent;
  assert(volume.create(2,"left",Directory|0755,0,0,left));
  assert(volume.create(2,"right",Directory|0755,0,0,right));
  assert(volume.create(left.number,"child",Directory|0755,0,0,child));
  assert(volume.create(right.number,"replacement",Directory|0755,0,0,overwritten));
  assert(volume.journal().commit());
  assert(!volume.rename(2,"left",child.number,"cycle",replaced) && volume.error()==Error::Invalid);
  assert(volume.rename(left.number,"child",right.number,"replacement",replaced) && replaced==overwritten.number);
  assert(volume.read_inode(child.number,child) && volume.lookup(child,"..",parent) && parent.number==right.number);
  assert(volume.read_inode(left.number,left) && left.links()==2);
  assert(volume.read_inode(right.number,right) && right.links()==3);
  assert(volume.reclaim(replaced) && volume.journal().commit());
  assert(volume.unlink(right.number,"replacement",removed) && volume.reclaim(removed));
  assert(volume.unlink(2,"left",removed) && volume.reclaim(removed));
  assert(volume.unlink(2,"right",removed) && volume.reclaim(removed) && volume.journal().commit());
  // Native ext3 component names are 255 bytes; variable-size records can span
  // several directory blocks without any xv6 fixed-dirent assumptions.
  char long_name[256];std::memset(long_name,'n',255);long_name[255]=0;
  assert(volume.create(2,"long-names",Directory|0700,0,0,directory));
  for(unsigned i=0;i<40;++i)
  {long_name[0]=char('A'+i/26);long_name[1]=char('a'+i%26);assert(volume.create(directory.number,long_name,Regular|0600,0,0,created));}
  assert(volume.journal().commit());
  for(unsigned i=0;i<40;++i)
  {long_name[0]=char('A'+i/26);long_name[1]=char('a'+i%26);assert(volume.unlink(directory.number,long_name,removed) && volume.reclaim(removed));}
  assert(volume.unlink(2,"long-names",removed) && volume.reclaim(removed) && volume.journal().commit());
  // Sparse writes address the triple-indirect root without allocating the gap.
  assert(volume.create(2,"sparse",Regular|0600,0,0,created));
  u64 fan=volume.geometry().block_size/4,triple=(12+fan+fan*fan)*volume.geometry().block_size;
  assert(volume.write(created,triple+13,"triple",6)==6 && volume.journal().commit());
  char sparse[20];std::memset(sparse,0x55,sizeof(sparse));
  assert(volume.read(created,triple,sparse,sizeof(sparse))==19);
  for(unsigned i=0;i<13;++i)assert(!sparse[i]);assert(!std::memcmp(sparse+13,"triple",6));
  assert(volume.resize(created,0) && volume.unlink(2,"sparse",removed) && volume.reclaim(removed) && volume.journal().commit());
  // Every allocation failure rolls an individual operation back, including
  // edits of buffers already staged by a previous successful operation.
  bool allocation_succeeded=false;
  for(int budget=0;budget<100 && !allocation_succeeded;++budget)
  {
    Inode baseline,attempt;
    assert(volume.create(2,"baseline",Regular|0600,0,0,baseline));
    auto before=volume.geometry();unsigned pending=volume.journal().dirty_count();
    device.allocation_budget=budget;
    allocation_succeeded=volume.create(2,"allocation-failure",Directory|0755,0,0,attempt);
    device.allocation_budget=-1;
    if(!allocation_succeeded)
    {
      assert(volume.error()==Error::NoMemory);
      assert(volume.geometry().free_blocks==before.free_blocks && volume.geometry().free_inodes==before.free_inodes);
      assert(volume.journal().dirty_count()==pending);
      assert(volume.read_inode(2,root) && !volume.lookup(root,"allocation-failure",attempt) && volume.error()==Error::NotFound);
    }
    else assert(volume.unlink(2,"allocation-failure",removed) && volume.reclaim(removed));
    assert(volume.read_inode(2,root) && volume.lookup(root,"baseline",attempt) && attempt.number==baseline.number);
    assert(volume.unlink(2,"baseline",removed) && volume.reclaim(removed) && volume.journal().commit());
  }
  assert(allocation_succeeded);
  assert(volume.create(2,"credit-limit",Regular|0600,0,0,created) && volume.journal().commit());
  auto before=volume.geometry();
  assert(volume.write(created,0,bytes.data(),unsigned(bytes.size()))==-1 && volume.error()==Error::NoSpace);
  assert(!volume.journal().dirty_count() && volume.geometry().free_blocks==before.free_blocks);
  assert(volume.read_inode(created.number,created) && !created.size() && !created.sectors());
  assert(volume.unlink(2,"credit-limit",removed) && volume.reclaim(removed) && volume.journal().commit());
  // More than 256 indirect leaves used to consume one dirty journal block per
  // freed pointer block. Complete-subtree truncation must not log those blocks.
  assert(volume.create(2,"truncate-tree",Regular|0600,0,0,created) && volume.journal().commit());
  for(unsigned i=0;i<300;++i)
  {u64 at=(12+fan+u64(i)*fan)*volume.geometry().block_size;assert(volume.write(created,at,"x",1)==1 && volume.journal().commit());}
  assert(volume.resize(created,0) && volume.journal().dirty_count()<32 && volume.journal().commit());
  assert(volume.unlink(2,"truncate-tree",removed) && volume.reclaim(removed) && volume.journal().commit());
  auto expected=sparse_orphan(volume);unsigned batches=0;bool complete=false;
  auto geometry=volume.geometry();Inode old;assert(volume.read_inode(expected.number,old));
  bool accepted=false;unsigned failed_allocations=0;
  for(int budget=0;budget<128;++budget)
  {
    device.allocation_budget=budget;bool success=volume.reclaim_step(expected.number,complete);device.allocation_budget=-1;
    if(success){assert(volume.journal().dirty_count()<=32 && volume.journal().commit());accepted=true;++batches;break;}
    ++failed_allocations;assert(!complete && !volume.journal().dirty_count());
    Inode current;assert(volume.read_inode(old.number,current) && !std::memcmp(old.raw,current.raw,sizeof(old.raw)));
    assert(volume.geometry().free_blocks==geometry.free_blocks);
  }
  assert(accepted && failed_allocations>6);
  while(!complete)
  {assert(volume.reclaim_step(expected.number,complete));assert(volume.journal().dirty_count()<=32);assert(volume.journal().commit());++batches;}
  assert(batches>100 && volume.geometry().free_blocks==expected.blocks && volume.geometry().free_inodes==expected.inodes);
  // A crash after a durable partial batch leaves the normal ext3 orphan chain.
  expected=sparse_orphan(volume);complete=false;
  for(unsigned i=0;i<7;++i){assert(volume.reclaim_step(expected.number,complete) && !complete && volume.journal().commit());}
  volume.reset();assert(volume.mount());
  assert(volume.geometry().free_blocks==expected.blocks && volume.geometry().free_inodes==expected.inodes);
  // Simulate a process disappearing after unlink, before its last close.
  assert(volume.create(2,"orphan",Regular|0600,0,0,created));
  assert(volume.write(created,0,"orphan data",11)==11 && volume.journal().commit());
  before=volume.geometry();
  assert(volume.unlink(2,"orphan",removed) && volume.journal().commit());
  volume.reset();assert(volume.mount());
  assert(volume.geometry().free_inodes==before.free_inodes+1 && volume.geometry().free_blocks==before.free_blocks+1);
  assert(volume.read_inode(2,root) && !volume.lookup(root,"orphan",created));
  char wide_name[256];std::memset(wide_name,'x',255);std::memcpy(wide_name,"enum-",5);wide_name[255]=0;
  for(char suffix:{'a','b'})
  {wide_name[254]=suffix;assert(volume.create(2,wide_name,Regular|0644,0,0,created) && volume.journal().commit());}
  volume.reset();assert(!device.live);
  std::printf("ext3 reclaim: %u bounded batches, %u allocation failures rolled back\n",batches,failed_allocations);
  std::puts("ext3 volume: Linux format, mutation, sparse triple indirect, OOM rollback and orphan recovery passed");
}
