#include "volume.h"

namespace xv::ext3
{
namespace
{
class Page
{
  Io &io_;u8 *data_;
public:
  explicit Page(Io &io):io_(io),data_(static_cast<u8 *>(io.allocate_page())) {}
  ~Page(){if(data_)io_.free_page(data_);}
  u8 *get() const {return data_;}
};
void set_size(Inode &inode,u64 size)
{put32(inode.raw+4,u32(size));if((inode.mode()&0xf000)==Regular)put32(inode.raw+108,u32(size>>32));}
void set_pointer(Inode &inode,unsigned index,u32 block){put32(inode.raw+40+4*index,block);}
void touch(Inode &inode,u32 now,bool data)
{put32(inode.raw+12,now);if(data)put32(inode.raw+16,now);}
u8 directory_type(u16 mode)
{
  switch(mode&0xf000){case Regular:return 1;case Directory:return 2;case Character:return 3;case Block:return 4;case Symlink:return 7;default:return 0;}
}
bool power(u32 n,u32 base){while(n>1 && n%base==0)n/=base;return n==1;}
unsigned name_size(const char *name)
{if(!name)return 0;unsigned n=0;while(n<=255 && name[n]){if(name[n]=='/')return 0;++n;}return n<=255 ? n:0;}
bool special_name(const char *name)
{return name[0]=='.' && (!name[1] || (name[1]=='.' && !name[2]));}
}

struct Volume::Operation
{
  Volume &volume;Geometry previous;bool active=false,accepted=false;
  explicit Operation(Volume &v):volume(v),previous(v.geometry_)
  {v.error_=Error::None;if(!v.ready_)v.error_=Error::Invalid;else active=v.journal_.savepoint();}
  ~Operation(){if(active && !accepted){volume.journal_.rollback();volume.geometry_=previous;}}
  bool finish(){if(!active)return false;volume.journal_.accept();accepted=true;return true;}
};

u8 *Volume::edit(u32 block)
{
  auto *result=journal_.edit(block);if(!result && error_==Error::None)error_=journal_.error();return result;
}
u8 *Volume::edit_super()
{auto *p=edit(1024/geometry_.block_size);return p ? p+1024%geometry_.block_size:nullptr;}
u8 *Volume::edit_group(u32 number)
{
  u64 offset=u64(number)*32;auto *p=edit(geometry_.first_data+1+u32(offset/geometry_.block_size));
  return p ? p+offset%geometry_.block_size:nullptr;
}
bool Volume::put_inode(const Inode &inode,bool fresh)
{
  u32 block;unsigned offset;if(!inode_location(inode.number,block,offset))return false;
  auto *p=edit(block);if(!p)return false;
  if(fresh)zero(p+offset,geometry_.inode_size);
  copy(p+offset,inode.raw,sizeof(inode.raw));return true;
}

bool Volume::metadata_block(u32 block,u32 number,u32 bitmap,u32 inode_bitmap,u32 table) const
{
  if(block==bitmap || block==inode_bitmap || reserved_block(block))return true;
  u32 table_blocks=u32((u64(geometry_.inodes_per_group)*geometry_.inode_size+geometry_.block_size-1)/geometry_.block_size);
  if(block>=table && block-table<table_blocks)return true;
  bool super=!(geometry_.readonly&1) || number==0 || power(number,3) || power(number,5) || power(number,7);
  u32 base=geometry_.first_data+number*geometry_.blocks_per_group;
  u32 descriptors=u32((u64(geometry_.groups)*32+geometry_.block_size-1)/geometry_.block_size);
  return super && block>=base && block-base<1+descriptors+geometry_.reserved_gdt;
}

bool Volume::allocate_block(u32 preferred,u32 &result)
{
  if(!geometry_.free_blocks)return fail(Error::NoSpace);
  Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  for(u32 attempt=0;attempt<geometry_.groups;++attempt)
  {
    u32 number=(preferred+attempt)%geometry_.groups,bitmap,inode_bitmap,table;
    if(!group(number,bitmap,inode_bitmap,table))return false;
    if(!read_block(bitmap,data))return false;
    u32 base=geometry_.first_data+number*geometry_.blocks_per_group;
    u32 limit=geometry_.blocks-base;if(limit>geometry_.blocks_per_group)limit=geometry_.blocks_per_group;
    for(u32 bit=0;bit<limit;++bit)
    {
      if(data[bit/8]&(1U<<(bit%8)))continue;
      u32 block=base+bit;if(metadata_block(block,number,bitmap,inode_bitmap,table))return fail(Error::Corrupt);
      auto *bits=edit(bitmap),*descriptor=edit_group(number),*super=edit_super();
      auto *contents=edit(block);if(!bits || !descriptor || !super || !contents)return false;
      auto free=le16(descriptor+12);if(!free)return fail(Error::Corrupt);
      bits[bit/8]|=u8(1U<<(bit%8));put16(descriptor+12,u16(free-1));
      --geometry_.free_blocks;put32(super+12,geometry_.free_blocks);
      zero(contents,geometry_.block_size);result=block;return true;
    }
  }
  return fail(Error::Corrupt);
}

bool Volume::free_block(u32 block)
{
  if(block<geometry_.first_data || block>=geometry_.blocks)return fail(Error::Corrupt);
  u32 number=(block-geometry_.first_data)/geometry_.blocks_per_group,bitmap,inode_bitmap,table;
  if(!group(number,bitmap,inode_bitmap,table) || metadata_block(block,number,bitmap,inode_bitmap,table))return fail(Error::Corrupt);
  auto *bits=edit(bitmap),*descriptor=edit_group(number),*super=edit_super();
  if(!bits || !descriptor || !super)return false;
  u32 bit=(block-geometry_.first_data)%geometry_.blocks_per_group;
  if(!(bits[bit/8]&(1U<<(bit%8))))return fail(Error::Corrupt);
  bits[bit/8]&=u8(~(1U<<(bit%8)));put16(descriptor+12,u16(le16(descriptor+12)+1));
  ++geometry_.free_blocks;put32(super+12,geometry_.free_blocks);return true;
}

bool Volume::allocate_inode(u16 mode,u32 uid,u32 gid,u32 preferred,Inode &inode)
{
  if(!geometry_.free_inodes)return fail(Error::NoSpace);
  Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  for(u32 attempt=0;attempt<geometry_.groups;++attempt)
  {
    u32 number=(preferred+attempt)%geometry_.groups,bitmap,inode_bitmap,table;
    if(!group(number,bitmap,inode_bitmap,table) || !read_block(inode_bitmap,data))return false;
    u32 first=number*geometry_.inodes_per_group;
    u32 limit=geometry_.inodes-first;if(limit>geometry_.inodes_per_group)limit=geometry_.inodes_per_group;
    for(u32 bit=0;bit<limit;++bit)
    {
      if(first+bit+1<geometry_.first_inode || (data[bit/8]&(1U<<(bit%8))))continue;
      auto *bits=edit(inode_bitmap),*descriptor=edit_group(number),*super=edit_super();
      if(!bits || !descriptor || !super)return false;
      auto free=le16(descriptor+14);if(!free)return fail(Error::Corrupt);
      bits[bit/8]|=u8(1U<<(bit%8));put16(descriptor+14,u16(free-1));
      if((mode&0xf000)==Directory)put16(descriptor+16,u16(le16(descriptor+16)+1));
      --geometry_.free_inodes;put32(super+16,geometry_.free_inodes);
      inode={};inode.number=first+bit+1;put16(inode.raw,mode);
      put16(inode.raw+2,u16(uid));put16(inode.raw+120,u16(uid>>16));
      put16(inode.raw+24,u16(gid));put16(inode.raw+122,u16(gid>>16));
      put32(inode.raw+8,io_.now_seconds());touch(inode,io_.now_seconds(),true);
      return put_inode(inode,true);
    }
  }
  return fail(Error::Corrupt);
}

bool Volume::map_allocated(Inode &inode,u64 logical,u32 &physical)
{
  const u64 fan=geometry_.block_size/4;unsigned root=0,levels=0,index[3]{};
  if(logical<12)root=unsigned(logical);
  else if((logical-=12)<fan){root=12;levels=1;index[0]=unsigned(logical);}
  else if((logical-=fan)<fan*fan){root=13;levels=2;index[0]=unsigned(logical/fan);index[1]=unsigned(logical%fan);}
  else if((logical-=fan*fan)<fan*fan*fan)
  {root=14;levels=3;index[0]=unsigned(logical/(fan*fan));index[1]=unsigned(logical/fan%fan);index[2]=unsigned(logical%fan);}
  else return fail(Error::Invalid);
  u32 block=inode.pointer(root),parent=0;unsigned slot=root;
  for(unsigned level=0;;++level)
  {
    if(!block)
    {
      if(!allocate_block((inode.number-1)/geometry_.inodes_per_group,block))return false;
      if(parent){auto *p=edit(parent);if(!p)return false;put32(p+slot*4,block);}
      else set_pointer(inode,root,block);
      u32 sectors=inode.sectors();if(sectors>0xffffffffU-geometry_.block_size/512)return fail(Error::NoSpace);
      put32(inode.raw+28,sectors+geometry_.block_size/512);
    }
    if(level==levels){physical=block;return true;}
    parent=block;slot=index[level];if(!pointer(parent,slot,block,false))return false;
  }
}

long Volume::write(Inode &inode,u64 offset,const void *source,unsigned count)
{
  if(!count)return 0;
  if(offset+count<offset){fail(Error::Invalid);return -1;}
  Operation operation(*this);if(!operation.active)return -1;Inode changed;
  if(!read_inode(inode.number,changed))return -1;
  if((changed.mode()&0xf000)!=Regular && (changed.mode()&0xf000)!=Symlink){fail(Error::Invalid);return -1;}
  if((changed.flags()&0x10) || ((changed.flags()&0x20) && offset!=changed.size())){fail(Error::ReadOnly);return -1;}
  if((changed.mode()&0xf000)==Symlink)
  {
    if(offset>changed.size() || offset+count>=geometry_.block_size){fail(Error::Invalid);return -1;}
    for(unsigned i=0;i<count;++i)if(!static_cast<const u8 *>(source)[i]){fail(Error::Invalid);return -1;}
    u64 size=offset+count>changed.size() ? offset+count:changed.size();
    if(changed.inline_link() && size<60)
    {
      copy(changed.raw+40+offset,source,count);zero(changed.raw+40+size,60-unsigned(size));
      set_size(changed,size);touch(changed,io_.now_seconds(),true);
      if(!put_inode(changed) || !operation.finish())return -1;inode=changed;return count;
    }
  }
  if(changed.inline_link() && changed.size())
  {
    u8 saved[60];unsigned length=unsigned(changed.size());copy(saved,changed.raw+40,length);zero(changed.raw+40,60);
    u32 block;if(!map_allocated(changed,0,block))return -1;auto *p=edit(block);if(!p)return -1;copy(p,saved,length);
  }
  unsigned done=0;
  while(done<count)
  {
    u32 block;if(!map_allocated(changed,(offset+done)/geometry_.block_size,block))return -1;
    auto *data=edit(block);if(!data)return -1;
    unsigned position=unsigned((offset+done)%geometry_.block_size),take=geometry_.block_size-position;
    if(take>count-done)take=count-done;
    copy(data+position,static_cast<const u8 *>(source)+done,take);done+=take;
  }
  if(offset+count>changed.size())set_size(changed,offset+count);
  touch(changed,io_.now_seconds(),true);
  if(!put_inode(changed) || !operation.finish())return -1;inode=changed;return count;
}

bool Volume::free_tree(Inode &inode,u32 block,unsigned level,u64 base,u64 keep,u32 &replacement,unsigned &budget,bool &complete)
{
  replacement=block;complete=true;if(!block)return true;
  const u64 fan=geometry_.block_size/4;
  u64 span=1;for(unsigned i=1;i<level;++i)span*=fan;
  if((!level && base<keep) || (level && base+span*fan<=keep))return true;
  if(!budget){complete=false;return true;}
  if(!level)
  {
    if(!free_block(block))return false;replacement=0;--budget;
    if(inode.sectors()<geometry_.block_size/512)return fail(Error::Corrupt);
    put32(inode.raw+28,inode.sectors()-geometry_.block_size/512);return true;
  }
  Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  if(!read_block(block,data))return false;bool empty=true,modified=false;
  for(unsigned i=0;i<fan;++i)
  {
    auto child=le32(data+i*4),changed=child;bool child_complete;
    if(!free_tree(inode,child,level-1,base+u64(i)*span,keep,changed,budget,child_complete))return false;
    if(changed!=child){put32(data+i*4,changed);modified=true;}
    if(changed)empty=false;
    if(!child_complete){complete=false;empty=false;break;}
  }
  if(empty && budget)
  {
    // The bitmap and the parent's removed pointer are sufficient. Logging the
    // contents of a block that is being freed wastes credits and DMA pages.
    if(!free_block(block))return false;replacement=0;--budget;
    if(inode.sectors()<geometry_.block_size/512)return fail(Error::Corrupt);
    put32(inode.raw+28,inode.sectors()-geometry_.block_size/512);
  }
  else
  {
    if(empty)complete=false;
    if(modified){auto *p=edit(block);if(!p)return false;copy(p,data,geometry_.block_size);}
  }
  return true;
}
bool Volume::prune(Inode &inode,u64 keep,unsigned &budget,bool &complete)
{
  const u64 fan=geometry_.block_size/4;u64 base=0;complete=true;
  for(unsigned slot=0;slot<15;++slot)
  {
    unsigned level=slot<12 ? 0:slot-11;u32 replacement;bool branch_complete;
    if(!free_tree(inode,inode.pointer(slot),level,base,keep,replacement,budget,branch_complete))return false;
    set_pointer(inode,slot,replacement);
    if(!branch_complete){complete=false;break;}
    u64 span=1;for(unsigned n=0;n<level;++n)span*=fan;base+=span;
  }
  return true;
}

bool Volume::resize_raw(Inode &inode,u64 size)
{
  auto type=inode.mode()&0xf000;
  if(type!=Regular && type!=Directory && type!=Symlink)
  {
    // Device numbers are encoded in i_block, and are never disk pointers.
    if(size || inode.sectors())return fail(Error::Invalid);
    set_size(inode,0);return true;
  }
  if(inode.inline_link())
  {
    if(size>inode.size())return fail(Error::Invalid);
    zero(inode.raw+40+size,unsigned(inode.size()-size));set_size(inode,size);return true;
  }
  if(size<=inode.size())
  {
    u64 keep=(size+geometry_.block_size-1)/geometry_.block_size;
    unsigned budget=~0U;bool complete;
    if(!prune(inode,keep,budget,complete) || !complete)return false;
    if(size%geometry_.block_size)
    {
      u32 block;if(!map_block(inode,size/geometry_.block_size,block))return false;
      if(block){auto *data=edit(block);if(!data)return false;unsigned offset=unsigned(size%geometry_.block_size);zero(data+offset,geometry_.block_size-offset);}
    }
  }
  set_size(inode,size);return true;
}

bool Volume::resize(Inode &inode,u64 size)
{
  const u64 fan=geometry_.block_size/4,max=(12+fan+fan*fan+fan*fan*fan)*geometry_.block_size;
  if(size>max)return fail(Error::Invalid);
  Operation operation(*this);if(!operation.active)return false;Inode changed;
  if(!read_inode(inode.number,changed))return false;
  if((changed.mode()&0xf000)!=Regular || (changed.flags()&0x30))return fail(Error::ReadOnly);
  if(!resize_raw(changed,size))return false;touch(changed,io_.now_seconds(),true);
  if(!put_inode(changed) || !operation.finish())return false;inode=changed;return true;
}

bool Volume::set_attributes(Inode &inode,u32 uid,u32 gid,u16 permissions)
{
  if(permissions&~07777)return fail(Error::Invalid);
  Operation operation(*this);if(!operation.active)return false;Inode changed;
  if(!read_inode(inode.number,changed))return false;
  if(changed.flags()&0x10)return fail(Error::ReadOnly);
  put16(changed.raw,u16((changed.mode()&0xf000)|permissions));put16(changed.raw+2,u16(uid));put16(changed.raw+120,u16(uid>>16));
  put16(changed.raw+24,u16(gid));put16(changed.raw+122,u16(gid>>16));touch(changed,io_.now_seconds(),false);
  if(!put_inode(changed) || !operation.finish())return false;inode=changed;return true;
}

bool Volume::set_device(Inode &inode,u32 major,u32 minor)
{
  if(major>4095 || minor>1048575)return fail(Error::Invalid);
  Operation operation(*this);if(!operation.active)return false;Inode changed;
  if(!read_inode(inode.number,changed))return false;
  auto type=changed.mode()&0xf000;
  if((type!=Character && type!=Block) || changed.sectors())return fail(Error::Invalid);
  if(changed.flags()&0x10)return fail(Error::ReadOnly);
  zero(changed.raw+40,60);
  if(major<256 && minor<256)set_pointer(changed,0,(major<<8)|minor);
  else set_pointer(changed,1,(minor&255)|(major<<8)|((minor&~255U)<<12));
  touch(changed,io_.now_seconds(),false);
  if(!put_inode(changed) || !operation.finish())return false;inode=changed;return true;
}

bool Volume::find_entry(const Inode &directory,const char *name,u64 &offset,u32 &number)
{
  unsigned length=name_size(name);if(!length)return fail(Error::Invalid);
  u64 cursor=0;DirectoryEntry entry;
  while(cursor<directory.size())
  {
    u64 before=cursor;
    if(!next_entry(directory,cursor,entry))return error()==Error::None ? fail(Error::NotFound):false;
    // next_entry may skip free records; locate the live record by its end and
    // scan the containing block to recover its exact start rather than 'before'.
    if(entry.length==length && equal(entry.name,name,length))
    {
      u64 block_start=(cursor-1)/geometry_.block_size*geometry_.block_size;
      u32 block;if(!map_block(directory,block_start/geometry_.block_size,block))return false;
      Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
      if(!read_block(block,data))return false;
      unsigned at=0;while(block_start+at<cursor)
      {
        auto record=le16(data+at+4);if(record<8 || at+record>geometry_.block_size)return fail(Error::Corrupt);
        if(block_start+at+record==cursor){offset=block_start+at;number=entry.inode;return true;}at+=record;
      }
      return fail(Error::Corrupt);
    }
    if(cursor<=before)return fail(Error::Corrupt);
  }
  return fail(Error::NotFound);
}

bool Volume::add_entry(Inode &directory,const char *name,u32 number,u16 mode)
{
  unsigned length=name_size(name);if(!length || (directory.mode()&0xf000)!=Directory)return fail(Error::Invalid);
  if(directory.flags()&0x30)return fail(Error::ReadOnly);
  u64 found;u32 old;
  if(find_entry(directory,name,found,old))return fail(Error::Exists);
  if(error()!=Error::NotFound)return false;error_=Error::None;
  const unsigned needed=(8+length+3)&~3U;
  Page scratch(io_);auto *data=scratch.get();if(!data)return fail(Error::NoMemory);
  u64 offset=0;
  while(offset<directory.size())
  {
    u32 block;if(!map_block(directory,offset/geometry_.block_size,block))return false;
    if(!block)return fail(Error::Corrupt);
    if(!read_block(block,data))return false;
    for(unsigned at=0;at<geometry_.block_size;)
    {
      unsigned record=le16(data+at+4),existing=geometry_.incompatible&FileType ? data[at+6]:le16(data+at+6);
      if(record<8 || (record&3) || record>geometry_.block_size-at)return fail(Error::Corrupt);
      unsigned used=le32(data+at) ? (8+existing+3)&~3U:0;
      if(used>record)return fail(Error::Corrupt);
      if(record-used>=needed)
      {
        auto *p=edit(block);if(!p)return false;
        if(used){put16(p+at+4,u16(used));at+=used;record-=used;}
        put32(p+at,number);put16(p+at+4,u16(record));put16(p+at+6,u16(length));
        if(geometry_.incompatible&FileType)p[at+7]=directory_type(mode);
        copy(p+at+8,name,length);put32(directory.raw+32,directory.flags()&~0x1000U);
        touch(directory,io_.now_seconds(),true);return put_inode(directory);
      }
      at+=record;
    }
    offset+=geometry_.block_size;
  }
  u32 block;if(!map_allocated(directory,offset/geometry_.block_size,block))return false;
  auto *p=edit(block);if(!p)return false;zero(p,geometry_.block_size);
  put32(p,number);put16(p+4,u16(geometry_.block_size));put16(p+6,u16(length));
  if(geometry_.incompatible&FileType)p[7]=directory_type(mode);copy(p+8,name,length);
  set_size(directory,offset+geometry_.block_size);put32(directory.raw+32,directory.flags()&~0x1000U);
  touch(directory,io_.now_seconds(),true);return put_inode(directory);
}

bool Volume::remove_entry(Inode &directory,const char *name,u32 &number)
{
  if(directory.flags()&0x30)return fail(Error::ReadOnly);
  u64 offset;if(!find_entry(directory,name,offset,number))return false;
  u32 block;if(!map_block(directory,offset/geometry_.block_size,block))return false;
  auto *p=edit(block);if(!p)return false;
  unsigned at=unsigned(offset%geometry_.block_size),previous=0;
  if(at)
  {
    while(previous+le16(p+previous+4)<at)
    {auto size=le16(p+previous+4);if(size<8)return fail(Error::Corrupt);previous+=size;}
    if(previous+le16(p+previous+4)!=at)return fail(Error::Corrupt);
    put16(p+previous+4,u16(le16(p+previous+4)+le16(p+at+4)));
  }
  else put32(p+at,0);
  put32(directory.raw+32,directory.flags()&~0x1000U);touch(directory,io_.now_seconds(),true);
  return put_inode(directory);
}

bool Volume::empty_directory(const Inode &directory)
{
  u64 offset=0;DirectoryEntry entry;
  while(next_entry(directory,offset,entry))if(!special_name(entry.name))return fail(Error::NotEmpty);
  return error()==Error::None;
}

bool Volume::orphan_head(u32 &number)
{
  Page scratch(io_);auto *p=scratch.get();if(!p)return fail(Error::NoMemory);
  if(!read_block(1024/geometry_.block_size,p))return false;
  number=le32(p+1024%geometry_.block_size+232);
  return number<=geometry_.inodes ? true:fail(Error::Corrupt);
}
bool Volume::add_orphan(Inode &inode)
{
  u32 head;if(!orphan_head(head))return false;
  for(u32 at=head,n=0;at;++n)
  {
    if(n>=geometry_.inodes)return fail(Error::Corrupt);
    if(at==inode.number)return true;
    Inode next;if(!read_inode(at,next))return false;at=le32(next.raw+20);
  }
  auto *p=edit_super();if(!p)return false;put32(inode.raw+20,head);put32(p+232,inode.number);return put_inode(inode);
}
bool Volume::remove_orphan(Inode &inode)
{
  u32 head;if(!orphan_head(head))return false;
  u32 previous=0;
  for(u32 at=head,n=0;at;++n)
  {
    if(n>=geometry_.inodes)return fail(Error::Corrupt);
    Inode current;if(!read_inode(at,current))return false;
    if(at==inode.number)
    {
      if(!previous){auto *p=edit_super();if(!p)return false;put32(p+232,le32(current.raw+20));}
      else{Inode prior;if(!read_inode(previous,prior))return false;put32(prior.raw+20,le32(current.raw+20));if(!put_inode(prior))return false;}
      put32(inode.raw+20,0);return true;
    }
    previous=at;at=le32(current.raw+20);
  }
  return true;
}

bool Volume::free_inode(Inode &inode)
{
  if(inode.number<geometry_.first_inode || inode.links())return fail(Error::Invalid);
  if(!resize_raw(inode,0) || !remove_orphan(inode))return false;
  u32 number=(inode.number-1)/geometry_.inodes_per_group,bitmap,inode_bitmap,table;
  if(!group(number,bitmap,inode_bitmap,table))return false;
  auto *bits=edit(inode_bitmap),*descriptor=edit_group(number),*super=edit_super();
  if(!bits || !descriptor || !super)return false;
  unsigned bit=(inode.number-1)%geometry_.inodes_per_group;
  if(!(bits[bit/8]&(1U<<(bit%8))))return fail(Error::Corrupt);
  bits[bit/8]&=u8(~(1U<<(bit%8)));put16(descriptor+14,u16(le16(descriptor+14)+1));
  if((inode.mode()&0xf000)==Directory)
  {auto count=le16(descriptor+16);if(!count)return fail(Error::Corrupt);put16(descriptor+16,u16(count-1));}
  ++geometry_.free_inodes;put32(super+16,geometry_.free_inodes);
  zero(inode.raw,sizeof(inode.raw));put32(inode.raw+20,io_.now_seconds());return put_inode(inode);
}

bool Volume::create(u32 parent,const char *name,u16 mode,u32 uid,u32 gid,Inode &created)
{
  if(!name_size(name) || special_name(name))return fail(Error::Invalid);
  u16 type=mode&0xf000;if(type!=Regular && type!=Directory && type!=Symlink && type!=Character && type!=Block)return fail(Error::Unsupported);
  Operation operation(*this);if(!operation.active)return false;Inode directory,node;
  if(!read_inode(parent,directory) || !allocate_inode(mode,uid,gid,(parent-1)/geometry_.inodes_per_group,node))return false;
  put16(node.raw+26,type==Directory ? 2:1);
  if(type==Directory)
  {
    if(directory.links()==65535)return fail(Error::NoSpace);
    if(!add_entry(node,".",node.number,node.mode()) || !add_entry(node,"..",parent,directory.mode()))return false;
    put16(directory.raw+26,u16(directory.links()+1));
  }
  if(!put_inode(node) || !add_entry(directory,name,node.number,node.mode()) || !operation.finish())return false;
  created=node;return true;
}

bool Volume::link(u32 parent,const char *name,u32 target)
{
  if(!name_size(name) || special_name(name))return fail(Error::Invalid);
  Operation operation(*this);if(!operation.active)return false;Inode directory,node;
  if(!read_inode(parent,directory) || !read_inode(target,node))return false;
  if((node.mode()&0xf000)==Directory || (node.flags()&0x30) || !node.links())return fail(Error::Invalid);
  if(node.links()==65535)return fail(Error::NoSpace);
  if(!add_entry(directory,name,target,node.mode()))return false;
  put16(node.raw+26,u16(node.links()+1));touch(node,io_.now_seconds(),false);
  return put_inode(node) && operation.finish();
}

bool Volume::unlink(u32 parent,const char *name,u32 &removed)
{
  if(!name_size(name) || special_name(name))return fail(Error::Invalid);
  Operation operation(*this);if(!operation.active)return false;Inode directory,node;
  if(!read_inode(parent,directory) || !lookup(directory,name,node))return false;
  if(node.number==2 || (node.flags()&0x30) || !node.links())return fail(Error::Invalid);
  if((node.mode()&0xf000)==Directory)
  {
    if(!empty_directory(node) || directory.links()<2)return false;
    put16(directory.raw+26,u16(directory.links()-1));put16(node.raw+26,0);
  }
  else put16(node.raw+26,u16(node.links()-1));
  u32 number;if(!remove_entry(directory,name,number))return false;
  touch(node,io_.now_seconds(),false);
  if(!put_inode(node) || (!node.links() && !add_orphan(node)) || !operation.finish())return false;
  removed=number;return true;
}

bool Volume::rename(u32 from,const char *old_name,u32 to,const char *new_name,u32 &replaced)
{
  replaced=0;
  if(!name_size(old_name) || !name_size(new_name) || special_name(old_name) || special_name(new_name))return fail(Error::Invalid);
  Operation operation(*this);if(!operation.active)return false;
  Inode source_parent,target_parent,source,target;
  if(!read_inode(from,source_parent) || !lookup(source_parent,old_name,source))return false;
  if(source.number==2 || (source.flags()&0x30))return fail(Error::ReadOnly);
  Inode *destination=&source_parent;
  if(from!=to){if(!read_inode(to,target_parent))return false;destination=&target_parent;}
  bool directory=(source.mode()&0xf000)==Directory;
  if(directory)
  {
    Inode ancestor=*destination;
    for(u32 n=0;;++n)
    {
      if(n>=geometry_.inodes || ancestor.number==source.number)return fail(Error::Invalid);
      if(ancestor.number==2)break;
      Inode next;if(!lookup(ancestor,"..",next) || next.number==ancestor.number)return fail(Error::Corrupt);ancestor=next;
    }
  }
  bool exists=lookup(*destination,new_name,target);
  if(!exists && error()!=Error::NotFound)return false;error_=Error::None;
  if(exists && target.number==source.number)return operation.finish();
  if(exists)
  {
    if((target.flags()&0x30) || directory!=((target.mode()&0xf000)==Directory))return fail(Error::Invalid);
    if(directory)
    {if(!empty_directory(target) || destination->links()<2)return false;put16(destination->raw+26,u16(destination->links()-1));put16(target.raw+26,0);}
    else{if(!target.links())return fail(Error::Corrupt);put16(target.raw+26,u16(target.links()-1));}
    u32 removed;if(!remove_entry(*destination,new_name,removed))return false;
    touch(target,io_.now_seconds(),false);
    if(!put_inode(target) || (!target.links() && !add_orphan(target)))return false;
    replaced=target.number;
  }
  if(!add_entry(*destination,new_name,source.number,source.mode()))return false;
  u32 removed;if(!remove_entry(source_parent,old_name,removed))return false;
  if(directory && from!=to)
  {
    if(source_parent.links()<2 || destination->links()==65535)return fail(Error::Corrupt);
    put16(source_parent.raw+26,u16(source_parent.links()-1));put16(destination->raw+26,u16(destination->links()+1));
    u64 offset;u32 parent;if(!find_entry(source,"..",offset,parent))return false;
    u32 block;if(!map_block(source,offset/geometry_.block_size,block))return false;
    auto *p=edit(block);if(!p)return false;put32(p+offset%geometry_.block_size,to);
  }
  touch(source,io_.now_seconds(),false);
  return put_inode(source_parent) && (from==to || put_inode(*destination)) && put_inode(source) && operation.finish();
}

bool Volume::reclaim(u32 number)
{
  Operation operation(*this);if(!operation.active)return false;Inode inode;
  if(!read_inode(number,inode) || !free_inode(inode))return false;return operation.finish();
}

bool Volume::orphan_step(Inode &inode,bool &complete)
{
  complete=true;unsigned type=inode.mode()&0xf000;
  if((type==Regular || type==Directory || type==Symlink) && !inode.inline_link())
  {
    // At most four freed blocks and their (up to three-level) parent paths,
    // bitmaps, group descriptors, superblock and inode fit 32 journal credits.
    unsigned budget=4;
    u64 keep=inode.links() ? (inode.size()+geometry_.block_size-1)/geometry_.block_size:0;
    if(!prune(inode,keep,budget,complete))return false;
  }
  if(!complete)return put_inode(inode);
  if(!inode.links())return free_inode(inode);
  return resize_raw(inode,inode.size()) && remove_orphan(inode) && put_inode(inode);
}
bool Volume::reclaim_step(u32 number,bool &complete)
{
  complete=false;Operation operation(*this);if(!operation.active)return false;Inode inode;
  if(!read_inode(number,inode))return false;
  if(inode.links())return fail(Error::Invalid);
  bool finished;
  if(!orphan_step(inode,finished) || !operation.finish())return false;
  complete=finished;return true;
}
bool Volume::recover_orphans()
{
  if(!ready_)return fail(Error::Invalid);
  for(u32 n=0;;++n)
  {
    u32 number;if(!orphan_head(number))return false;if(!number)return true;
    if(n>=geometry_.inodes)return fail(Error::Corrupt);
    bool complete=false;
    while(!complete)
    {
      Operation operation(*this);if(!operation.active)return false;Inode inode;
      if(!read_inode(number,inode) || !orphan_step(inode,complete))return false;
      if(!operation.finish() || !journal_.commit())return false;
    }
  }
}
} // namespace xv::ext3
