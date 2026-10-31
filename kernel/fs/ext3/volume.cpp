#include "volume.h"

namespace xv::ext3
{
namespace
{
class Page
{
  Io &io_;
  u8 *bytes_;
public:
  explicit Page(Io &io):io_(io),bytes_(static_cast<u8 *>(io.allocate_page())) {}
  ~Page(){if(bytes_)io_.free_page(bytes_);}
  u8 *get() const {return bytes_;}
};
}

bool Volume::superblock()
{
  Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  if(!io_.read(1024,data,1024))return fail(Error::Io);
  if(le16(data+56)!=Magic)return fail(Error::Corrupt);
  auto log_size=le32(data+24);if(log_size>2)return fail(Error::Unsupported);
  geometry_.block_size=1024U<<log_size;
  geometry_.blocks=le32(data+4);geometry_.inodes=le32(data);
  geometry_.free_blocks=le32(data+12);geometry_.free_inodes=le32(data+16);
  geometry_.first_data=le32(data+20);geometry_.blocks_per_group=le32(data+32);
  geometry_.inodes_per_group=le32(data+40);
  auto revision=le32(data+76);if(revision>1)return fail(Error::Unsupported);
  geometry_.inode_size=revision ? le16(data+88):128;
  geometry_.first_inode=revision ? le32(data+84):11;
  geometry_.compatible=revision ? le32(data+92):0;
  geometry_.incompatible=revision ? le32(data+96):0;
  geometry_.readonly=revision ? le32(data+100):0;
  geometry_.journal_inode=le32(data+224);
  geometry_.reserved_gdt=le16(data+206);
  if(!(geometry_.compatible&HasJournal) || (geometry_.incompatible&~(FileType|Recover)) ||
     (geometry_.readonly&~7U) || le32(data+228))return fail(Error::Unsupported);
  if(!geometry_.blocks || geometry_.blocks<=geometry_.first_data || geometry_.inodes<11 ||
     geometry_.first_data!=(geometry_.block_size==1024 ? 1U:0U) ||
     !geometry_.blocks_per_group || geometry_.blocks_per_group>geometry_.block_size*8 ||
     !geometry_.inodes_per_group || geometry_.inodes_per_group>geometry_.block_size*8 ||
     geometry_.inode_size<128 || geometry_.inode_size>geometry_.block_size ||
     (geometry_.inode_size&(geometry_.inode_size-1)) ||
     geometry_.first_inode<11 || geometry_.first_inode>geometry_.inodes ||
     geometry_.free_blocks>geometry_.blocks || geometry_.free_inodes>geometry_.inodes ||
     u64(geometry_.blocks)*geometry_.block_size>io_.size_bytes())return fail(Error::Corrupt);
  geometry_.groups=u32((u64(geometry_.blocks-geometry_.first_data)+geometry_.blocks_per_group-1)/geometry_.blocks_per_group);
  if(u64(geometry_.groups)*geometry_.inodes_per_group<geometry_.inodes ||
     u64(geometry_.groups-1)*geometry_.inodes_per_group>=geometry_.inodes ||
     geometry_.journal_inode<1 || geometry_.journal_inode>geometry_.inodes)return fail(Error::Corrupt);
  return true;
}

bool Volume::read_block(u32 block,void *data,bool raw)
{
  if(block>=geometry_.blocks)return fail(Error::Corrupt);
  if(!raw && ready_)return journal_.read(block,data);
  return io_.read(u64(block)*geometry_.block_size,data,geometry_.block_size) ? true:fail(Error::Io);
}

bool Volume::group(u32 number,u32 &block_bitmap,u32 &inode_bitmap,u32 &inode_table)
{
  if(number>=geometry_.groups)return fail(Error::Invalid);
  Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  const u64 offset=u64(number)*32;
  const u32 descriptor=geometry_.first_data+1+u32(offset/geometry_.block_size);
  if(!read_block(descriptor,data))return false;
  data+=offset%geometry_.block_size;
  block_bitmap=le32(data);inode_bitmap=le32(data+4);inode_table=le32(data+8);
  const u32 table_blocks=u32((u64(geometry_.inodes_per_group)*geometry_.inode_size+geometry_.block_size-1)/geometry_.block_size);
  if(!block_bitmap || block_bitmap>=geometry_.blocks || !inode_bitmap || inode_bitmap>=geometry_.blocks ||
     !inode_table || inode_table>=geometry_.blocks || table_blocks>geometry_.blocks-inode_table)
    return fail(Error::Corrupt);
  return true;
}

bool Volume::inode_location(u32 number,u32 &block,unsigned &offset)
{
  if(!number || number>geometry_.inodes)return fail(Error::Invalid);
  u32 block_bitmap,inode_bitmap,table;
  if(!group((number-1)/geometry_.inodes_per_group,block_bitmap,inode_bitmap,table))return false;
  u64 displacement=u64((number-1)%geometry_.inodes_per_group)*geometry_.inode_size;
  block=table+u32(displacement/geometry_.block_size);offset=unsigned(displacement%geometry_.block_size);
  return true;
}

bool Volume::read_inode(u32 number,Inode &result)
{
  clear_error();
  u32 block;unsigned offset;if(!inode_location(number,block,offset))return false;
  Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  if(!read_block(block,data))return false;
  result.number=number;copy(result.raw,data+offset,sizeof(result.raw));
  // Extents, inline data and ext4 inode layouts must never be mistaken for
  // ext3 block pointers. External ACLs need a permission implementation first.
  if((result.flags()&0x10080000U) || le32(result.raw+104))return fail(Error::Unsupported);
  return true;
}

bool Volume::pointer(u32 block,unsigned index,u32 &result,bool raw)
{
  if(index>=geometry_.block_size/4)return fail(Error::Invalid);
  if(!block){result=0;return true;}
  if(raw && !reserved_block(block) && !reserve_block(block))return false;
  Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  if(!read_block(block,data,raw))return false;
  result=le32(data+index*4);
  if(result>=geometry_.blocks)return fail(Error::Corrupt);
  return true;
}

bool Volume::map_block(const Inode &inode,u64 logical,u32 &physical,bool raw)
{
  const u64 fan=geometry_.block_size/4;
  if(logical<12)physical=inode.pointer(unsigned(logical));
  else if((logical-=12)<fan)
  {if(!pointer(inode.pointer(12),unsigned(logical),physical,raw))return false;}
  else if((logical-=fan)<fan*fan)
  {
    u32 indirect;if(!pointer(inode.pointer(13),unsigned(logical/fan),indirect,raw) ||
       !pointer(indirect,unsigned(logical%fan),physical,raw))return false;
  }
  else if((logical-=fan*fan)<fan*fan*fan)
  {
    u32 middle,indirect;if(!pointer(inode.pointer(14),unsigned(logical/(fan*fan)),middle,raw) ||
       !pointer(middle,unsigned(logical/fan%fan),indirect,raw) ||
       !pointer(indirect,unsigned(logical%fan),physical,raw))return false;
  }
  else return fail(Error::Invalid);
  return physical<geometry_.blocks ? true:fail(Error::Corrupt);
}

bool Volume::reserve_block(u32 block)
{
  if(reserved_ && reserved_->count)
  {
    auto &last=reserved_->ranges[reserved_->count-1];
    if(last.start+last.length==block){++last.length;return true;}
  }
  if(!reserved_ || reserved_->count==sizeof(reserved_->ranges)/sizeof(Reserved::Range))
  {
    auto *page=static_cast<Reserved *>(io_.allocate_page());if(!page)return fail(Error::NoMemory);
    page->next=reserved_;page->count=0;reserved_=page;
  }
  reserved_->ranges[reserved_->count++]={block,1};return true;
}
bool Volume::reserved_block(u32 block) const
{
  for(auto *page=reserved_;page;page=page->next)
    for(unsigned i=0;i<page->count;++i)
      if(block>=page->ranges[i].start && block-page->ranges[i].start<page->ranges[i].length)return true;
  return false;
}

bool Volume::journal_mapping()
{
  if(!read_inode(geometry_.journal_inode,journal_inode_))return false;
  if((journal_inode_.mode()&0xf000)!=Regular || journal_inode_.size()%geometry_.block_size ||
     journal_inode_.size()/geometry_.block_size>geometry_.blocks)return fail(Error::Corrupt);
  journal_blocks_=u32(journal_inode_.size()/geometry_.block_size);
  if(journal_blocks_<8)return fail(Error::Corrupt);
  for(u32 i=0;i<journal_blocks_;++i)
  {
    u32 physical;if(!map_block(journal_inode_,i,physical,true) || !physical)return fail(Error::Corrupt);
    if(reserved_block(physical))return fail(Error::Corrupt);
    if(!reserve_block(physical))return false;
    if(!mapping_tail_ || mapping_tail_->count==sizeof(mapping_tail_->blocks)/sizeof(u32))
    {
      auto *page=static_cast<Mapping *>(io_.allocate_page());if(!page)return fail(Error::NoMemory);
      page->next=nullptr;page->count=0;
      if(mapping_tail_)mapping_tail_->next=page;else mapping_=page;
      mapping_tail_=page;
    }
    mapping_tail_->blocks[mapping_tail_->count++]=physical;
  }
  return true;
}

bool Volume::journal_block(u32 logical,u32 &physical) const
{
  for(auto *page=mapping_;page;page=page->next)
  {
    if(logical<page->count){physical=page->blocks[logical];return true;}
    logical-=page->count;
  }
  return false;
}

bool Volume::JournalDevice::read_journal(u32 logical,void *data)
{
  u32 physical;if(!volume_.journal_block(logical,physical))return false;
  return volume_.io_.read(u64(physical)*block_size(),data,block_size());
}
bool Volume::JournalDevice::write_journal(u32 logical,const void *data)
{
  u32 physical;if(!volume_.journal_block(logical,physical))return false;
  return volume_.io_.write(u64(physical)*block_size(),data,block_size());
}
bool Volume::JournalDevice::read_home(u32 block,void *data)
{ return volume_.read_block(block,data,true); }
bool Volume::JournalDevice::write_home(u32 block,const void *data)
{ return valid_target(block) && volume_.io_.write(u64(block)*block_size(),data,block_size()); }
bool Volume::JournalDevice::valid_target(u32 block) const
{ return block<volume_.geometry_.blocks && !volume_.reserved_block(block); }

bool Volume::set_recovery(bool enabled)
{
  Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  const u32 block=1024/geometry_.block_size;const unsigned offset=1024%geometry_.block_size;
  if(!read_block(block,data,true))return false;
  auto flags=le32(data+offset+96),changed=enabled ? flags|Recover:flags&~Recover;
  if(auto *staged=journal_.staged(block))
  {auto current=le32(staged+offset+96);put32(staged+offset+96,enabled ? current|Recover:current&~Recover);}
  if(flags==changed)return true;
  put32(data+offset+96,changed);
  return io_.write(u64(block)*geometry_.block_size,data,geometry_.block_size) && io_.flush() ? true:fail(Error::Io);
}

bool Volume::mount()
{
  reset();error_=Error::None;
  if(!superblock() || !journal_mapping() || !journal_.mount() || !journal_.recover() || !superblock())return false;
  ready_=true;
  Inode root;if(!read_inode(2,root) || (root.mode()&0xf000)!=Directory)return fail(Error::Corrupt);
  return recover_orphans();
}
void Volume::reset()
{
  journal_.reset();
  while(mapping_){auto *next=mapping_->next;io_.free_page(mapping_);mapping_=next;}
  mapping_tail_=nullptr;
  while(reserved_){auto *next=reserved_->next;io_.free_page(reserved_);reserved_=next;}
  ready_=false;geometry_={};journal_blocks_=0;
}

long Volume::read(const Inode &inode,u64 offset,void *destination,unsigned count)
{
  clear_error();
  if(!count || offset>=inode.size())return 0;
  if(count>inode.size()-offset)count=unsigned(inode.size()-offset);
  if(inode.inline_link()){copy(destination,inode.raw+40+offset,count);return count;}
  Page scratch(io_);auto *data=scratch.get();if(!data){fail(Error::NoMemory);return -1;}
  unsigned done=0;
  while(done<count)
  {
    u32 physical;if(!map_block(inode,(offset+done)/geometry_.block_size,physical))return -1;
    unsigned start=unsigned((offset+done)%geometry_.block_size),take=geometry_.block_size-start;
    if(take>count-done)take=count-done;
    if(physical){if(!read_block(physical,data))return -1;}else zero(data,geometry_.block_size);
    copy(static_cast<u8 *>(destination)+done,data+start,take);done+=take;
  }
  return done;
}

bool Volume::next_entry(const Inode &directory,u64 &offset,DirectoryEntry &entry)
{
  clear_error();
  if((directory.mode()&0xf000)!=Directory)return fail(Error::Invalid);
  Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  while(offset<directory.size())
  {
    unsigned displacement=unsigned(offset%geometry_.block_size);u32 physical;
    if(!map_block(directory,offset/geometry_.block_size,physical))return false;
    if(!physical)return fail(Error::Corrupt);
    if(!read_block(physical,data))return false;
    auto *record=data+displacement;
    if(geometry_.block_size-displacement<8)return fail(Error::Corrupt);
    const unsigned inode=le32(record),length=le16(record+4);
    if(length<8 || (length&3) || length>geometry_.block_size-displacement || length>directory.size()-offset)return fail(Error::Corrupt);
    offset+=length;if(!inode)continue;
    auto name_length=geometry_.incompatible&FileType ? unsigned(record[6]):unsigned(le16(record+6));
    if(inode>geometry_.inodes || !name_length || name_length>255 || name_length>length-8)return fail(Error::Corrupt);
    entry.inode=inode;entry.type=geometry_.incompatible&FileType ? record[7]:0;entry.length=name_length;
    for(unsigned i=0;i<name_length;++i)
    {if(!record[8+i] || record[8+i]=='/')return fail(Error::Corrupt);entry.name[i]=char(record[8+i]);}
    entry.name[name_length]=0;return true;
  }
  error_=Error::None;return false;
}

bool Volume::lookup(const Inode &directory,const char *name,Inode &result)
{
  clear_error();
  unsigned length=0;while(length<256 && name[length])++length;
  if(!length || length>255)return fail(Error::Invalid);
  u64 offset=0;DirectoryEntry entry;
  while(next_entry(directory,offset,entry))
    if(entry.length==length && equal(entry.name,name,length))return read_inode(entry.inode,result);
  return error()==Error::None ? fail(Error::NotFound):false;
}
} // namespace xv::ext3
