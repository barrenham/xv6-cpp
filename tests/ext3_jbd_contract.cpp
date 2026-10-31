#include "kernel/fs/ext3/jbd.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
using namespace xv::ext3;

struct Device final : JournalIo
{
  unsigned bytes;
  std::vector<u8> disk;
  int budget=-1,alloc_budget=-1,live=0,events=0;
  bool recovery=false,commit_seen=false;
  explicit Device(unsigned size):bytes(size),disk(3072*size)
  {
    auto *super=&disk[2048*bytes];put_be32(super,JournalMagic);put_be32(super+4,JournalSuperV2);
    put_be32(super+12,bytes);put_be32(super+16,1024);put_be32(super+20,1);put_be32(super+24,17);
    put_be32(super+40,1);put_be32(super+64,1);
    for(unsigned i=0;i<16;++i)super[48+i]=u8(23+i);
  }
  unsigned block_size() const override { return bytes; }
  u32 journal_capacity() const override { return 1024; }
  bool event() { ++events;if(!budget)return false;if(budget>0)--budget;return true; }
  bool read_journal(u32 logical,void *data) override
  { if(logical>=1024)return false;std::memcpy(data,&disk[(2048+logical)*bytes],bytes);return true; }
  bool write_journal(u32 logical,const void *data) override
  {
    if(!event() || logical>=1024)return false;
    if(be32(data)==JournalMagic && be32(static_cast<const u8 *>(data)+4)==Commit)commit_seen=true;
    std::memcpy(&disk[(2048+logical)*bytes],data,bytes);return true;
  }
  bool read_home(u32 block,void *data) override
  { if(!valid_target(block))return false;std::memcpy(data,&disk[block*bytes],bytes);return true; }
  bool write_home(u32 block,const void *data) override
  { if(!event() || !valid_target(block))return false;std::memcpy(&disk[block*bytes],data,bytes);return true; }
  bool valid_target(u32 block) const override { return block>0 && block<1024; }
  bool flush() override { return event(); }
  bool recovery_flag(bool flag) override { if(!event())return false;recovery=flag;return true; }
  void *allocate_page() override
  { if(!alloc_budget)return nullptr;if(alloc_budget>0)--alloc_budget;++live;return std::aligned_alloc(4096,4096); }
  void free_page(void *page) override { --live;std::free(page); }
};

void stage(Jbd &log,unsigned blocks=5)
{
  assert(log.savepoint());
  for(unsigned i=0;i<blocks;++i)
  {
    auto *data=log.edit(20+i);assert(data);std::memset(data,int(0x31+i),1024);
    if(i==1)put_be32(data,JournalMagic);
  }
  log.accept();
}
void crashes(unsigned size)
{
  Device initial(size);Jbd normal(initial);assert(normal.mount());stage(normal);
  assert(normal.commit() && !initial.recovery);unsigned events=initial.events;normal.reset();assert(!initial.live);
  for(unsigned cut=0;cut<=events;++cut)
  {
    Device device(size);Jbd writer(device);assert(writer.mount());stage(writer);
    device.budget=int(cut);(void)writer.commit();writer.reset();assert(!device.live);
    bool committed=device.commit_seen;device.budget=-1;
    Jbd recovered(device);assert(recovered.mount() && recovered.recover());
    for(unsigned i=0;i<5;++i)
    {
      auto *data=&device.disk[(20+i)*size];
      if(!committed)for(unsigned n=0;n<size;++n)assert(!data[n]);
      else for(unsigned n=0;n<1024;++n)
      {
        if(i==1 && n<4)assert(data[n]==u8(JournalMagic>>((3-n)*8)));
        else assert(data[n]==u8(0x31+i));
      }
    }
    assert(!device.recovery && !be32(&device.disk[2048*size+28]));
    recovered.reset();assert(!device.live);
  }
}
void rollback()
{
  Device device(1024);Jbd log(device);assert(log.mount());
  auto *p=log.edit(20);assert(p);p[0]=1;
  assert(log.savepoint());p=log.edit(20);assert(p);p[0]=2;assert(log.edit(21));
  log.rollback();unsigned char data[1024];assert(log.read(20,data) && data[0]==1 && log.dirty_count()==1);
  assert(log.savepoint());device.alloc_budget=0;
  assert(!log.edit(20) && log.error()==Error::NoMemory);log.rollback();
  assert(log.read(20,data) && data[0]==1);
  device.alloc_budget=-1;assert(log.commit());log.reset();assert(!device.live);
}
void multiple_descriptors()
{
  Device device(1024);Jbd log(device);assert(log.mount());
  assert(log.savepoint());
  for(unsigned i=0;i<Jbd::MaxDirty;++i){auto *p=log.edit(20+i);assert(p);p[0]=u8(i);}
  log.accept();
  // Fail checkpoint after the commit record is durable, then replay all three descriptors.
  device.budget=265;assert(!log.commit());assert(device.commit_seen);log.reset();device.budget=-1;
  assert(log.mount() && log.recover());
  for(unsigned i=0;i<Jbd::MaxDirty;++i)assert(device.disk[(20+i)*1024]==u8(i));
  log.reset();assert(!device.live);
}
void revocations(u32 sequence)
{
  Device device(1024);auto *super=&device.disk[2048*1024];put_be32(super+24,sequence);put_be32(super+28,1);device.recovery=true;
  auto head=[&](unsigned n,u32 type,u32 seq){auto *p=&device.disk[(2048+n)*1024];put_be32(p,JournalMagic);put_be32(p+4,type);put_be32(p+8,seq);return p;};
  auto *p=head(1,Descriptor,sequence);put_be32(p+12,20);put_be32(p+16,LastTag);copy(p+20,super+48,16);
  device.disk[(2048+2)*1024]=0x71;head(3,Commit,sequence);
  p=head(4,Descriptor,sequence+1);put_be32(p+12,21);put_be32(p+16,LastTag);copy(p+20,super+48,16);
  device.disk[(2048+5)*1024]=0x72;
  p=head(6,Revoke,sequence+1);put_be32(p+12,20);put_be32(p+16,20);head(7,Commit,sequence+1);
  Jbd log(device);assert(log.mount() && log.recover());
  assert(!device.disk[20*1024] && device.disk[21*1024]==0x72 && log.sequence()==sequence+2);
  log.reset();assert(!device.live);
}
void torn_descriptor(bool committed)
{
  Device device(1024);auto *super=&device.disk[2048*1024];put_be32(super+28,1);device.recovery=true;
  auto *descriptor=&device.disk[2049*1024];put_be32(descriptor,JournalMagic);
  put_be32(descriptor+4,Descriptor);put_be32(descriptor+8,17);
  // First descriptor sector contains a tag, but its UUID/trailing tags were lost.
  put_be32(descriptor+12,20);put_be32(descriptor+16,LastTag);
  if(committed){auto *commit=&device.disk[2051*1024];put_be32(commit,JournalMagic);put_be32(commit+4,Commit);put_be32(commit+8,17);}
  Jbd log(device);assert(log.mount());
  if(committed)assert(!log.recover() && log.error()==Error::Corrupt && device.recovery);
  else assert(log.recover() && !device.recovery);
  log.reset();assert(!device.live);
}
int main()
{
  torn_descriptor(false);torn_descriptor(true);crashes(1024);crashes(2048);crashes(4096);rollback();multiple_descriptors();revocations(17);revocations(0xffffffffU);
  std::puts("ext3 JBD: commit cuts, escaped data, revokes, wraparound, savepoints and OOM passed");
}
