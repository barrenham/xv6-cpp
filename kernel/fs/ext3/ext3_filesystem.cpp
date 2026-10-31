module;
#include "kernel/fs/ext3/volume.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/support/flow.h"
module xv.fs;
import :ext3;
import :platform;

namespace xv::fs
{
using WireInode=xv::ext3::Inode;

bool Ext3FileSystem::DiskIo::read(xv::ext3::u64 offset,void *data,unsigned count)
{
  auto size=size_bytes();if(offset>size || count>size-offset)return false;
  auto *out=static_cast<unsigned char *>(data);
  while(count)
  {
    unsigned at=unsigned(offset%BlockSize),take=BlockSize-at;if(take>count)take=count;
    auto block=cache_.read(device,unsigned(offset/BlockSize));
    detail::move(out,block->bytes()+at,take);out+=take;offset+=take;count-=take;
  }
  return true;
}
bool Ext3FileSystem::DiskIo::write(xv::ext3::u64 offset,const void *data,unsigned count)
{
  auto size=size_bytes();if(offset>size || count>size-offset)return false;
  auto *in=static_cast<const unsigned char *>(data);
  while(count)
  {
    unsigned at=unsigned(offset%BlockSize),take=BlockSize-at;if(take>count)take=count;
    auto block=cache_.read(device,unsigned(offset/BlockSize));
    detail::move(block->bytes()+at,in,take);cache_.write(*block);in+=take;offset+=take;count-=take;
  }
  return true;
}
bool Ext3FileSystem::DiskIo::flush(){cache_.flush();return true;}
xv::ext3::u64 Ext3FileSystem::DiskIo::size_bytes() const
{
  auto sectors=disk ? disk->capacity_sectors():0;
  // BlockCache addresses blocks with an unsigned index. Reject an oversized
  // device instead of wrapping a journal or home-block write onto its start.
  return sectors<=0x100000000UL*(BlockSize/512) ? sectors*512:0;
}
xv::ext3::u32 Ext3FileSystem::DiskIo::now_seconds() const {return xv::ext3::u32(detail::epoch_seconds());}
bool Ext3FileSystem::DiskIo::prepare_reserve()
{
  for(auto &page:reserve_)if(!page)
  {page=detail::allocate_page();if(!page){release_reserve();return false;}}
  return true;
}
void Ext3FileSystem::DiskIo::release_reserve()
{
  for(unsigned i=0;i<3;++i)
  {
    if(borrowed_[i])detail::fail("ext3: journal reserve still borrowed");
    if(reserve_[i]){detail::free_page(reserve_[i]);reserve_[i]=nullptr;}
  }
}
bool Ext3FileSystem::DiskIo::commit(xv::ext3::Volume &volume)
{
  committing_=true;bool result=volume.journal().commit();committing_=false;
  for(bool borrowed:borrowed_)if(borrowed)detail::fail("ext3: leaked commit reserve");
  return result;
}
void *Ext3FileSystem::DiskIo::allocate_page()
{
  if(auto *page=detail::allocate_page())return page;
  if(committing_)for(unsigned i=0;i<3;++i)if(reserve_[i] && !borrowed_[i])
  {borrowed_[i]=true;return reserve_[i];}
  return nullptr;
}
void Ext3FileSystem::DiskIo::free_page(void *page)
{
  for(unsigned i=0;i<3;++i)if(page==reserve_[i])
  {if(!borrowed_[i])detail::fail("ext3: duplicate reserve release");borrowed_[i]=false;return;}
  detail::free_page(page);
}

void Ext3FileSystem::Log::initialize(){state_.init("ext3 transaction");}
void Ext3FileSystem::Log::ready(unsigned capacity)
{
  if(capacity<Credits)detail::fail("ext3: journal too small");
  capacity_=capacity;
}
void Ext3FileSystem::Log::changed(unsigned used)
{xv::sync::ScopedLock guard(state_);used_=used;}
void Ext3FileSystem::Log::wait()
{detail::prepare_sleep(this);state_.release();detail::sleep();state_.acquire();}
void Ext3FileSystem::Log::begin()
{
  state_.acquire();
  while(committing_ || used_+(outstanding_+1)*Credits>capacity_)wait();
  ++outstanding_;state_.release();
}
void Ext3FileSystem::Log::commit(bool flush)
{
  // No state spinlock is held during disk I/O or while acquiring core_.
  owner_.core_.acquire();
  if(!owner_.io_.commit(owner_.volume_) || (flush && !owner_.io_.flush()))
    detail::fail("ext3: journal commit failed");
  owner_.collect_orphans();
  owner_.core_.release();
  state_.acquire();used_=0;committing_=false;detail::wakeup(this);state_.release();
}
void Ext3FileSystem::Log::end()
{
  state_.acquire();if(!outstanding_ || committing_)detail::fail("ext3: unbalanced transaction");
  if(--outstanding_){detail::wakeup(this);state_.release();return;}
  committing_=true;state_.release();commit(false);
}
void Ext3FileSystem::Log::record(Buffer &)
{detail::fail("ext3: use staged journal buffers");}
void Ext3FileSystem::Log::sync()
{
  state_.acquire();while(outstanding_ || committing_)wait();
  committing_=true;state_.release();commit(true);
}

void Ext3FileSystem::initialize_inodes()
{
  core_.init("ext3 volume");references_.init("ext3 inodes");log_.initialize();
  for(auto &node:inodes_){node.owner_=this;node.lock.init("ext3 inode");}
}
void Ext3FileSystem::initialize(int device)
{
  io_.device=unsigned(device);
  xv::sync::ScopedLock guard(core_);
  if(!io_.disk || !io_.prepare_reserve() || !volume_.mount())detail::fail("ext3: mount failed");
  log_.ready(volume_.journal().capacity());mounted_=true;
}
void Ext3FileSystem::changed(){log_.changed(volume_.journal().dirty_count());}
bool Ext3FileSystem::busy()
{
  xv::sync::ScopedLock guard(references_);
  unsigned references=0;for(auto &node:inodes_)references+=unsigned(node.ref);
  return references!=1;
}
void Ext3FileSystem::shutdown()
{
  log_.sync();xv::sync::ScopedLock guard(core_);volume_.reset();io_.release_reserve();mounted_=false;
}
Inode *Ext3FileSystem::get(unsigned number)
{
  xv::sync::ScopedLock guard(references_);Inode *empty=nullptr;
  for(auto &node:inodes_)
  {
    if(node.ref && node.inum==number){++node.ref;return &node;}
    if(!node.ref && !empty)empty=&node;
  }
  if(!empty)detail::fail("ext3: no inode cache entries");
  empty->dev=io_.device;empty->inum=number;empty->ref=1;empty->valid=0;return empty;
}
Inode *Ext3FileSystem::root(){return mounted_ ? get(2):nullptr;}
Inode *Ext3FileSystem::retain(Inode *node)
{xv::sync::ScopedLock guard(references_);if(node->ref<1)detail::fail("ext3: stale inode");++node->ref;return node;}
void Ext3FileSystem::refresh(Inode *node,const WireInode &wire)
{
  switch(wire.mode()&0xf000)
  {
  case xv::ext3::Regular:node->type=T_FILE;break;
  case xv::ext3::Directory:node->type=T_DIR;break;
  case xv::ext3::Symlink:node->type=T_SYMLINK;break;
  case xv::ext3::Character:node->type=T_DEVICE;break;
  case xv::ext3::Block:node->type=T_BLOCK;break;
  default:node->type=0;break;
  }
  node->uid_=wire.uid();node->gid_=wire.gid();node->mode_=wire.mode()&07777;
  node->nlink=short(wire.links());node->size=wire.size()>0xffffffffU ? 0xffffffffU:unsigned(wire.size());
  node->major=short(wire.device_major());node->minor=short(wire.device_minor());node->valid=1;
}
bool Ext3FileSystem::wire(Inode *node,WireInode &result)
{return volume_.read_inode(node->inum,result);}
void Ext3FileSystem::lock(Inode *node)
{
  node->lock.acquire();xv::sync::ScopedLock guard(core_);WireInode value;
  if(!wire(node,value) || !value.mode())detail::fail("ext3: invalid inode");refresh(node,value);
}
void Ext3FileSystem::unlock(Inode *node){node->lock.release();}
void Ext3FileSystem::unlock_release(Inode *node){unlock(node);release(node);}
void Ext3FileSystem::release(Inode *node)
{
  references_.acquire();
  if(node->ref<1)detail::fail("ext3: invalid release");
  if(node->ref>1){--node->ref;references_.release();return;}
  if(!node->lock.try_acquire())detail::fail("ext3: release locked inode");
  --node->ref;references_.release();node->lock.release();
  // Orphan ownership is persistent. Reclaim after the final transaction has
  // committed, when no participants can depend on its staged blocks.
}
struct Ext3FileSystem::CollectFlow
{
  struct Context
  {
    Ext3FileSystem &owner;
    unsigned number=0,next=0,visited=0;
    bool okay=false,retained=false,busy=false,complete=false;
    WireInode inode{};
  };
  static void head(Context &c) noexcept
  {
    c.okay=c.owner.volume_.orphan_head(c.number);
    if(!c.okay && c.owner.volume_.error()!=xv::ext3::Error::NoMemory)detail::fail("ext3: orphan head failed");
  }
  static bool okay(Context &c) noexcept {return c.okay;}
  static bool more(Context &c) noexcept {return c.number!=0;}
  static void inspect(Context &c) noexcept
  {
    if(c.visited++>=c.owner.volume_.geometry().inodes)detail::fail("ext3: cyclic orphan list");
    c.okay=c.owner.volume_.read_inode(c.number,c.inode);
    if(!c.okay)
    {if(c.owner.volume_.error()!=xv::ext3::Error::NoMemory)detail::fail("ext3: orphan inode failed");return;}
    c.next=xv::ext3::le32(c.inode.raw+20);c.busy=false;
    xv::sync::ScopedLock guard(c.owner.references_);
    for(auto &node:c.owner.inodes_)if(node.ref && node.inum==c.number){c.busy=true;break;}
  }
  static bool pinned(Context &c) noexcept {return c.busy || c.inode.links();}
  static void retain(Context &c) noexcept {c.retained=true;}
  static void reclaim(Context &c) noexcept
  {
    c.okay=c.owner.volume_.reclaim_step(c.number,c.complete);
    if(!c.okay && c.owner.volume_.error()!=xv::ext3::Error::NoMemory)detail::fail("ext3: orphan reclaim failed");
  }
  static void commit(Context &c) noexcept
  {if(!c.owner.io_.commit(c.owner.volume_))detail::fail("ext3: orphan commit failed");}
  static bool complete(Context &c) noexcept {return c.complete;}
  static void advance(Context &c) noexcept {c.number=c.next;}
  static void finish(Context &c) noexcept {c.owner.garbage_pending_=c.retained;}
  struct Head;struct More;struct Inspect;struct Pinned;struct Retain;struct Reclaim;
  struct Commit;struct Advance;struct Finish;
  using Graph=xv::flow::graph<Head,xv::flow::types<
    xv::flow::node<Head,xv::flow::action<&head>,xv::flow::branch<xv::flow::condition<&okay>,xv::flow::go<More>,xv::flow::stop>>,
    xv::flow::node<More,xv::flow::noop,xv::flow::branch<xv::flow::condition<&more>,xv::flow::go<Inspect>,xv::flow::go<Finish>>>,
    xv::flow::node<Inspect,xv::flow::action<&inspect>,xv::flow::branch<xv::flow::condition<&okay>,xv::flow::go<Pinned>,xv::flow::stop>>,
    xv::flow::node<Pinned,xv::flow::noop,xv::flow::branch<xv::flow::condition<&pinned>,xv::flow::go<Retain>,xv::flow::go<Reclaim>>>,
    xv::flow::node<Retain,xv::flow::action<&retain>,xv::flow::go<Advance>>,
    xv::flow::node<Reclaim,xv::flow::action<&reclaim>,xv::flow::branch<xv::flow::condition<&okay>,xv::flow::go<Commit>,xv::flow::stop>>,
    xv::flow::node<Commit,xv::flow::action<&commit>,xv::flow::branch<xv::flow::condition<&complete>,xv::flow::go<Advance>,xv::flow::go<Reclaim>>>,
    xv::flow::node<Advance,xv::flow::action<&advance>,xv::flow::go<More>>,
    xv::flow::node<Finish,xv::flow::action<&finish>,xv::flow::stop>>>;
};
void Ext3FileSystem::collect_orphans()
{
  if(!garbage_pending_)return;
  CollectFlow::Context context{*this};(void)CollectFlow::Graph::run<0>(context);
}
bool Ext3FileSystem::component(const char *source,char (&name)[256])
{unsigned i=0;for(;i<255 && source[i];++i)name[i]=source[i];name[i]=0;return !source[i];}
Inode *Ext3FileSystem::lookup_entry(Inode *directory,const char *source,unsigned *offset)
{
  char name[256];if(!component(source,name))return nullptr;xv::sync::ScopedLock guard(core_);WireInode parent,found;
  if(!wire(directory,parent) || !volume_.lookup(parent,name,found))return nullptr;
  if(offset)*offset=0;return get(found.number);
}
Inode *Ext3FileSystem::create_at(Inode *directory,const char *source,short type,short major,short minor)
{
  char name[256];if(!component(source,name))return nullptr;lock(directory);
  auto who=detail::identity();
  if(!directory->linked() || !detail::allowed(directory,3) || ((type==T_DEVICE || type==T_BLOCK) && who.uid))
  {unlock(directory);return nullptr;}
  core_.acquire();WireInode parent,created;Inode *result=nullptr;
  if(wire(directory,parent) && volume_.lookup(parent,name,created))
  {
    if(type==T_FILE && ((created.mode()&0xf000)==xv::ext3::Regular || (created.mode()&0xf000)==xv::ext3::Character || (created.mode()&0xf000)==xv::ext3::Block))result=get(created.number);
  }
  else if(volume_.error()==xv::ext3::Error::NotFound)
  {
    unsigned kind=type==T_FILE ? xv::ext3::Regular:type==T_DIR ? xv::ext3::Directory:type==T_SYMLINK ? xv::ext3::Symlink:type==T_DEVICE ? xv::ext3::Character:type==T_BLOCK ? xv::ext3::Block:0;
    unsigned mode=(type==T_DIR ? 0777:(type==T_DEVICE || type==T_BLOCK) ? 0600:0666)&~who.mask;
    if(kind && volume_.create(directory->inum,name,xv::ext3::u16(kind|mode),who.uid,who.gid,created))
    {
      if((type==T_DEVICE || type==T_BLOCK) && !volume_.set_device(created,unsigned(major),unsigned(minor)))
      {unsigned removed;if(!volume_.unlink(directory->inum,name,removed) || !volume_.reclaim(removed))detail::fail("ext3: create rollback failed");}
      else result=get(created.number);
    }
    changed();
  }
  core_.release();unlock(directory);if(result)lock(result);return result;
}
int Ext3FileSystem::link_at(Inode *directory,const char *source,Inode *target)
{
  if(&target->owner()!=this || &directory->owner()!=this)return -1;
  char name[256];if(!component(source,name))return -1;lock(directory);int result=-1;
  if(directory->linked() && detail::allowed(directory,3))
  {xv::sync::ScopedLock guard(core_);if(volume_.link(directory->inum,name,target->inum))result=0;changed();}
  unlock(directory);return result;
}
int Ext3FileSystem::unlink_at(Inode *directory,const char *source)
{
  char name[256];if(!component(source,name))return -1;lock(directory);int result=-1;unsigned removed=0;
  if(directory->linked() && detail::allowed(directory,3))
  {
    xv::sync::ScopedLock guard(core_);WireInode parent,target;Inode security;
    if(wire(directory,parent) && volume_.lookup(parent,name,target))
    {refresh(&security,target);if(detail::may_remove(directory,&security) && volume_.unlink(directory->inum,name,removed))result=0;}
    if(!result)garbage_pending_=true;
    changed();
  }
  unlock(directory);return result;
}
int Ext3FileSystem::rename_at(Inode *from,const char *old_source,Inode *to,const char *new_source)
{
  if(&from->owner()!=this || &to->owner()!=this)return -1;
  char old_name[256],new_name[256];if(!component(old_source,old_name) || !component(new_source,new_name))return -1;
  Inode *first=from->inum<=to->inum ? from:to,*second=first==from ? to:from;
  lock(first);if(first!=second)lock(second);int result=-1;unsigned replaced=0;
  if(from->linked() && to->linked() && detail::allowed(from,3) && detail::allowed(to,3))
  {
    xv::sync::ScopedLock guard(core_);WireInode parent,source,target;Inode security;
    if(wire(from,parent) && volume_.lookup(parent,old_name,source))
    {
      refresh(&security,source);bool allowed=detail::may_remove(from,&security);
      if(allowed && wire(to,parent))
      {
        bool exists=volume_.lookup(parent,new_name,target);
        if(exists){refresh(&security,target);allowed=detail::may_remove(to,&security);}
        else allowed=volume_.error()==xv::ext3::Error::NotFound;
        if(allowed && volume_.rename(from->inum,old_name,to->inum,new_name,replaced))result=0;
      }
    }
    if(!result && replaced)garbage_pending_=true;
    changed();
  }
  if(first!=second)unlock(second);unlock(first);
  return result;
}
Inode *Ext3FileSystem::allocate(unsigned,short)
{
  // Namespace operations allocate an inode and insert its name atomically in
  // Volume::create. There is no exposed, nameless half-created VFS inode.
  return nullptr;
}
void Ext3FileSystem::update(Inode *node)
{
  xv::sync::ScopedLock guard(core_);WireInode value;
  if(!wire(node,value) || !volume_.set_attributes(value,node->uid_,node->gid_,node->mode_))detail::fail("ext3: attribute update failed");
  refresh(node,value);changed();
}
int Ext3FileSystem::resize(Inode *node,unsigned size)
{
  xv::sync::ScopedLock guard(core_);WireInode value;
  bool success=wire(node,value) && volume_.resize(value,size);
  if(success)refresh(node,value);changed();return success ? 0:-1;
}
void Ext3FileSystem::truncate(Inode *node)
{if(resize(node,0)<0)detail::fail("ext3: truncate failed");}
int Ext3FileSystem::next_directory(Inode *node,unsigned long &cookie,DirectoryRecord &entry)
{
  xv::sync::ScopedLock guard(core_);WireInode inode;xv::ext3::DirectoryEntry found;
  if(!wire(node,inode))return -1;
  xv::ext3::u64 next=cookie;
  if(!volume_.next_entry(inode,next,found))
  {if(volume_.error()!=xv::ext3::Error::None)return -1;cookie=next;return 0;}
  entry={};entry.inode=found.inode;detail::move(entry.name,found.name,found.length+1);
  constexpr unsigned char types[]={0,8,4,2,6,1,12,10};entry.type=found.type<8 ? types[found.type]:0;
  cookie=next;return 1;
}
int Ext3FileSystem::read(Inode *node,int user,unsigned long destination,unsigned offset,unsigned count)
{
  auto *page=static_cast<unsigned char *>(detail::allocate_page());if(!page)return -1;
  unsigned done=0;bool failed=false;
  while(done<count)
  {
    unsigned take=count-done,block_tail=BlockSize-(offset+done)%BlockSize;
    if(take>block_tail)take=block_tail;long copied=-1;
    core_.acquire();WireInode value;
    if(wire(node,value))
    {
      if((value.mode()&0xf000)==xv::ext3::Directory)
      {
        // Existing read(dir) ABI. A separate native directory iterator can
        // expose long names without changing the legacy 16-byte disk dirent.
        copied=0;xv::ext3::u64 cursor=0;unsigned index=0;xv::ext3::DirectoryEntry entry;
        while(volume_.next_entry(value,cursor,entry))
        {
          dirent record{};record.inum=static_cast<unsigned short>(entry.inode);
          for(unsigned i=0;i<DIRSIZ && i<entry.length;++i)record.name[i]=entry.name[i];
          unsigned position=index++*sizeof(record);
          if(position+sizeof(record)<=offset+done)continue;
          unsigned begin=offset+done>position ? offset+done-position:0,n=sizeof(record)-begin;
          if(n>take-unsigned(copied))n=take-unsigned(copied);
          detail::move(page+copied,reinterpret_cast<unsigned char *>(&record)+begin,n);copied+=n;
          if(unsigned(copied)==take)break;
        }
        if(volume_.error()!=xv::ext3::Error::None)copied=-1;
      }
      else copied=volume_.read(value,xv::ext3::u64(offset)+done,page,take);
    }
    core_.release();
    if(copied<0){failed=true;break;}if(!copied)break;
    if(detail::copy_out(user,destination+done,page,unsigned(copied))<0){failed=true;break;}done+=unsigned(copied);
  }
  detail::free_page(page);return failed && !done ? -1:int(done);
}
int Ext3FileSystem::write(Inode *node,int user,unsigned long source,unsigned offset,unsigned count)
{
  if(count>0xffffffffU-offset)return -1;
  auto *page=static_cast<unsigned char *>(detail::allocate_page());if(!page)return -1;
  unsigned done=0;bool failed=false;
  while(done<count)
  {
    unsigned take=count-done,block_tail=BlockSize-(offset+done)%BlockSize;
    if(take>block_tail)take=block_tail;
    unsigned copied=0;
    while(copied<take)
    {
      unsigned part=take-copied;
      if(user){unsigned tail=4096-(source+done+copied)%4096;if(part>tail)part=tail;}
      if(detail::copy_in(page+copied,user,source+done+copied,part)<0)break;
      copied+=part;
    }
    const bool fault=copied!=take;
    core_.acquire();WireInode value;
    long written=-1;
    if(wire(node,value))
    {
      // The existing write ABI excludes a faulted 1 KiB part from the return
      // count and file growth, while retaining already copied overwrites.
      // Page-bounded user copies tell us exactly which prefix is valid.
      unsigned publish=copied;
      if(fault)
      {
        auto position=xv::ext3::u64(offset)+done;
        auto existing=position<value.size() ? value.size()-position:0;
        if(publish>existing)publish=unsigned(existing);
      }
      written=publish ? volume_.write(value,offset+done,page,publish):0;
      if(written>0)refresh(node,value);
    }
    changed();core_.release();
    if(written<0 || fault){failed=true;break;}done+=take;
  }
  detail::free_page(page);return failed && !done ? -1:int(done);
}
void Ext3FileSystem::stat(Inode *node,Stat *result)
{
  xv::sync::ScopedLock guard(core_);WireInode value;
  if(!wire(node,value))detail::fail("ext3: stat failed");refresh(node,value);
  result->dev=node->dev;result->ino=node->inum;result->type=node->type;
  result->nlink=node->nlink;result->size=value.size();node->security_stat(result);
}
xv::drivers::BlockDevice *Ext3FileSystem::open_block_device(Inode *node,bool writable)
{return open_registered_block(node,writable);}
xv::drivers::CharacterDevice *Ext3FileSystem::open_device(Inode *node,bool nonblocking)
{return open_registered_device(node,nonblocking);}
}
