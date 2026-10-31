#include "jbd.h"

namespace xv::ext3
{
namespace
{
class Scratch
{
  JournalIo &io_;
  u8 *data_;
public:
  explicit Scratch(JournalIo &io):io_(io),data_(static_cast<u8 *>(io.allocate_page())) {}
  ~Scratch() { if(data_)io_.free_page(data_); }
  u8 *get() const { return data_; }
};
void header(u8 *data,unsigned bytes,u32 type,u32 sequence)
{ zero(data,bytes);put_be32(data,JournalMagic);put_be32(data+4,type);put_be32(data+8,sequence); }
}

struct Jbd::Revocations
{
  struct Entry { u32 block,sequence; };
  struct Page { Page *next;unsigned count;Entry entries[(4096-sizeof(void *)-sizeof(unsigned)-8)/sizeof(Entry)]; };
  JournalIo &io;
  Page *head=nullptr;
  explicit Revocations(JournalIo &device):io(device) {}
  ~Revocations() { while(head){auto *next=head->next;io.free_page(head);head=next;} }
  bool add(u32 block,u32 sequence)
  {
    for(auto *p=head;p;p=p->next)
      for(unsigned i=0;i<p->count;++i)
        if(p->entries[i].block==block){p->entries[i].sequence=sequence;return true;}
    if(!head || head->count==sizeof(head->entries)/sizeof(Entry))
    {
      auto *p=static_cast<Page *>(io.allocate_page());if(!p)return false;
      p->next=head;p->count=0;head=p;
    }
    head->entries[head->count++]={block,sequence};return true;
  }
  bool contains(u32 block,u32 sequence) const
  {
    for(auto *p=head;p;p=p->next)
      for(unsigned i=0;i<p->count;++i)
        if(p->entries[i].block==block)return sequence_at_least(p->entries[i].sequence,sequence);
    return false;
  }
};

unsigned Jbd::capacity() const
{
  if(!mounted_)return 0;
  unsigned tags=(io_.block_size()-28)/8;
  unsigned result=length_-first_-1;
  while(result+(result+tags-1)/tags+1>length_-first_)--result;
  return result<MaxDirty ? result:MaxDirty;
}

bool Jbd::mount()
{
  reset();clear_error();
  if(io_.block_size()!=1024 && io_.block_size()!=2048 && io_.block_size()!=4096)return fail(Error::Unsupported);
  Scratch scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  if(!io_.read_journal(0,data))return fail(Error::Io);
  const auto type=be32(data+4);
  if(be32(data)!=JournalMagic || (type!=JournalSuperV1 && type!=JournalSuperV2) ||
     be32(data+12)!=io_.block_size())return fail(Error::Corrupt);
  length_=be32(data+16);first_=be32(data+20);sequence_=be32(data+24);start_=be32(data+28);
  if(first_<1 || length_<=first_+3 || length_>io_.journal_capacity() ||
     (start_ && (start_<first_ || start_>=length_)))return fail(Error::Corrupt);
  if(type==JournalSuperV2 && (be32(data+36) || (be32(data+40)&~1U) || be32(data+44) || be32(data+64)>1))
    return fail(Error::Unsupported);
  copy(uuid_,data+48,16);mounted_=true;return true;
}

bool Jbd::write_super(u32 start,u32 sequence)
{
  Scratch scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  if(!io_.read_journal(0,data))return fail(Error::Io);
  put_be32(data+24,sequence);put_be32(data+28,start);
  return io_.write_journal(0,data) && io_.flush() ? true:fail(Error::Io);
}

bool Jbd::incomplete(unsigned pass,u32 sequence,u32 cursor)
{
  if(pass)return fail(Error::Corrupt);
  // An interrupted descriptor write can leave an invalid uncommitted tail.
  // Do not discard it if a durable commit for that sequence still exists.
  Scratch scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  for(unsigned n=0;n<length_-first_;++n,cursor=next(cursor))
  {
    if(!io_.read_journal(cursor,data))return fail(Error::Io);
    if(be32(data)==JournalMagic && be32(data+4)==Commit && be32(data+8)==sequence)
      return fail(Error::Corrupt);
  }
  return true;
}

bool Jbd::scan(unsigned pass,u32 &end_sequence,Revocations *revocations)
{
  Scratch block(io_),payload(io_);auto *data=block.get(),*bytes=payload.get();
  if(!data || !bytes)return fail(Error::NoMemory);
  u32 cursor=start_,sequence=sequence_;unsigned traversed=0;
  while(traversed<length_-first_)
  {
    if(pass && sequence==end_sequence)return true;
    if(!io_.read_journal(cursor,data))return fail(Error::Io);
    if(be32(data)!=JournalMagic || be32(data+8)!=sequence)
      return incomplete(pass,sequence,cursor);
    auto type=be32(data+4);cursor=next(cursor);++traversed;
    if(type==Descriptor)
    {
      unsigned offset=12;bool last=false,have_uuid=false;
      while(!last)
      {
        if(offset+8>io_.block_size() || traversed>=length_-first_)return incomplete(pass,sequence,cursor);
        auto target=be32(data+offset),flags=be32(data+offset+4);offset+=8;
        if(flags&~15U || !io_.valid_target(target))return incomplete(pass,sequence,cursor);
        if(!(flags&SameUuid))
        {
          if(offset+16>io_.block_size() || !equal(data+offset,uuid_,16))return incomplete(pass,sequence,cursor);
          offset+=16;have_uuid=true;
        }
        else if(!have_uuid)return incomplete(pass,sequence,cursor);
        last=flags&LastTag;
        if(!(flags&Deleted))
        {
          if(pass==2 && (!revocations || !revocations->contains(target,sequence)))
          {
            if(!io_.read_journal(cursor,bytes))return fail(Error::Io);
            if(flags&Escaped)put_be32(bytes,JournalMagic);
            if(!io_.write_home(target,bytes))return fail(Error::Io);
          }
          cursor=next(cursor);++traversed;
        }
      }
    }
    else if(type==Revoke)
    {
      auto limit=be32(data+12);
      if(limit<16 || limit>io_.block_size() || (limit&3))return incomplete(pass,sequence,cursor);
      if(pass==1)
        for(unsigned offset=16;offset<limit;offset+=4)
        {
          auto target=be32(data+offset);if(!io_.valid_target(target))return fail(Error::Corrupt);
          if(!revocations->add(target,sequence))return fail(Error::NoMemory);
        }
    }
    else if(type==Commit)
    {
      ++sequence;
      if(!pass)end_sequence=sequence;
    }
    else return incomplete(pass,sequence,cursor);
  }
  return pass && sequence!=end_sequence ? fail(Error::Corrupt):true;
}

bool Jbd::recover()
{
  if(!mounted_ || count_)return fail(Error::Invalid);
  if(start_)
  {
    u32 end=sequence_;
    if(!scan(0,end,nullptr))return false;
    if(end!=sequence_)
    {
      Revocations revocations(io_);
      if(!scan(1,end,&revocations) || !scan(2,end,&revocations) || !io_.flush())return fail(error_==Error::None ? Error::Io:error_);
      sequence_=end;
    }
  }
  if((start_ && !write_super(0,sequence_)) || !io_.recovery_flag(false))return fail(Error::Io);
  start_=0;return true;
}

u8 *Jbd::edit(u32 block)
{
  if(!mounted_ || !io_.valid_target(block)){fail(Error::Invalid);return nullptr;}
  for(unsigned i=0;i<count_;++i)
    if(dirty_[i].block==block)
    {
      if(saving_ && i<saved_count_)
      {
        bool saved=false;for(unsigned k=0;k<undo_count_;++k)if(undo_[k].index==i)saved=true;
        if(!saved)
        {
          auto *replacement=static_cast<u8 *>(io_.allocate_page());
          if(!replacement){fail(Error::NoMemory);return nullptr;}
          copy(replacement,dirty_[i].bytes,io_.block_size());
          undo_[undo_count_++]={i,dirty_[i].bytes};dirty_[i].bytes=replacement;
        }
      }
      return dirty_[i].bytes;
    }
  if(count_>=capacity()){fail(Error::NoSpace);return nullptr;}
  auto *data=static_cast<u8 *>(io_.allocate_page());if(!data){fail(Error::NoMemory);return nullptr;}
  if(!io_.read_home(block,data)){io_.free_page(data);fail(Error::Io);return nullptr;}
  dirty_[count_++]={block,data};return data;
}

u8 *Jbd::staged(u32 block)
{
  for(unsigned i=0;i<count_;++i)if(dirty_[i].block==block)return dirty_[i].bytes;
  return nullptr;
}

bool Jbd::read(u32 block,void *destination)
{
  for(unsigned i=0;i<count_;++i)if(dirty_[i].block==block){copy(destination,dirty_[i].bytes,io_.block_size());return true;}
  return io_.read_home(block,destination) ? true:fail(Error::Io);
}

bool Jbd::savepoint()
{
  if(saving_)return fail(Error::Invalid);
  clear_error();saved_count_=count_;undo_count_=0;saving_=true;return true;
}
void Jbd::accept()
{
  for(unsigned i=0;i<undo_count_;++i)io_.free_page(undo_[i].previous);
  undo_count_=0;saving_=false;
}
void Jbd::rollback()
{
  if(!saving_)return;
  for(unsigned i=0;i<undo_count_;++i)
  {auto &entry=dirty_[undo_[i].index];io_.free_page(entry.bytes);entry.bytes=undo_[i].previous;}
  while(count_>saved_count_)io_.free_page(dirty_[--count_].bytes);
  undo_count_=0;saving_=false;
}
void Jbd::reset()
{
  rollback();while(count_)io_.free_page(dirty_[--count_].bytes);
  mounted_=false;first_=length_=start_=sequence_=0;
}

bool Jbd::commit()
{
  if(!mounted_ || saving_)return fail(Error::Invalid);
  if(!count_)return true;
  Scratch descriptor(io_),payload(io_);auto *head=descriptor.get(),*bytes=payload.get();
  if(!head || !bytes)return fail(Error::NoMemory);
  if(!io_.recovery_flag(true) || !write_super(first_,sequence_))return fail(Error::Io);
  u32 cursor=first_;unsigned done=0;
  while(done<count_)
  {
    header(head,io_.block_size(),Descriptor,sequence_);
    unsigned offset=12,first=done;
    while(done<count_)
    {
      unsigned needed=8+(done==first ? 16:0);if(offset+needed>io_.block_size())break;
      auto &entry=dirty_[done];u32 flags=done==first ? 0:SameUuid;
      if(be32(entry.bytes)==JournalMagic)flags|=Escaped;
      bool last=done+1==count_ || offset+needed+8>io_.block_size();if(last)flags|=LastTag;
      put_be32(head+offset,entry.block);put_be32(head+offset+4,flags);offset+=8;
      if(done==first){copy(head+offset,uuid_,16);offset+=16;}
      ++done;if(last)break;
    }
    if(!io_.write_journal(cursor,head))return fail(Error::Io);cursor=next(cursor);
    for(unsigned i=first;i<done;++i)
    {
      copy(bytes,dirty_[i].bytes,io_.block_size());if(be32(bytes)==JournalMagic)put_be32(bytes,0);
      if(!io_.write_journal(cursor,bytes))return fail(Error::Io);cursor=next(cursor);
    }
  }
  if(!io_.flush())return fail(Error::Io);
  header(head,io_.block_size(),Commit,sequence_);
  if(!io_.write_journal(cursor,head) || !io_.flush())return fail(Error::Io);
  for(unsigned i=0;i<count_;++i)if(!io_.write_home(dirty_[i].block,dirty_[i].bytes))return fail(Error::Io);
  if(!io_.flush() || !write_super(0,sequence_+1) || !io_.recovery_flag(false))return fail(Error::Io);
  ++sequence_;start_=0;
  while(count_)io_.free_page(dirty_[--count_].bytes);
  return true;
}
} // namespace xv::ext3
