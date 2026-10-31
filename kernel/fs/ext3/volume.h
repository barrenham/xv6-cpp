#pragma once
#include "jbd.h"

namespace xv::ext3
{
class Io
{
public:
  virtual bool read(u64 offset,void *data,unsigned count)=0;
  virtual bool write(u64 offset,const void *data,unsigned count)=0;
  virtual bool flush()=0;
  virtual u64 size_bytes() const=0;
  virtual u32 now_seconds() const=0;
  virtual void *allocate_page()=0;
  virtual void free_page(void *page)=0;
protected:
  ~Io()=default;
};
struct Inode
{
  u32 number=0;
  u8 raw[128]{};
  u16 mode() const { return le16(raw); }
  u32 uid() const { return le16(raw+2)|(u32(le16(raw+120))<<16); }
  u32 gid() const { return le16(raw+24)|(u32(le16(raw+122))<<16); }
  u16 links() const { return le16(raw+26); }
  u32 sectors() const { return le32(raw+28); }
  u32 flags() const { return le32(raw+32); }
  u32 pointer(unsigned index) const { return le32(raw+40+index*4); }
  u64 size() const { return le32(raw+4)|((mode()&0xf000)==Regular ? u64(le32(raw+108))<<32:0); }
  bool inline_link() const { return (mode()&0xf000)==Symlink && !sectors() && size()<60; }
  u32 device_major() const { return pointer(0) ? (pointer(0)>>8)&255:(pointer(1)>>8)&4095; }
  u32 device_minor() const { return pointer(0) ? pointer(0)&255:(pointer(1)&255)|((pointer(1)>>12)&0xfff00); }
};
struct DirectoryEntry
{
  u32 inode=0;
  u8 type=0;
  unsigned length=0;
  char name[256]{};
};
struct Geometry
{
  u32 blocks=0,inodes=0,free_blocks=0,free_inodes=0;
  u32 block_size=0,blocks_per_group=0,inodes_per_group=0,first_data=0;
  u32 groups=0,inode_size=0,first_inode=0;
  u32 reserved_gdt=0;
  u32 compatible=0,incompatible=0,readonly=0,journal_inode=0;
};
class Volume final
{
public:
  constexpr explicit Volume(Io &io):io_(io),journal_io_(*this),journal_(journal_io_) {}
  Volume(const Volume&)=delete;
  Volume &operator=(const Volume&)=delete;
  bool mount();
  void reset();
  bool read_inode(u32 number,Inode &result);
  // Return bytes copied, zero at EOF, or -1 with a diagnostic in error().
  long read(const Inode &inode,u64 offset,void *destination,unsigned count);
  bool next_entry(const Inode &directory,u64 &offset,DirectoryEntry &entry);
  bool lookup(const Inode &directory,const char *name,Inode &result);
  long write(Inode &inode,u64 offset,const void *source,unsigned count);
  bool resize(Inode &inode,u64 size);
  bool set_attributes(Inode &inode,u32 uid,u32 gid,u16 permissions);
  bool set_device(Inode &inode,u32 major,u32 minor);
  bool create(u32 parent,const char *name,u16 mode,u32 uid,u32 gid,Inode &created);
  bool link(u32 parent,const char *name,u32 target);
  bool unlink(u32 parent,const char *name,u32 &removed);
  bool rename(u32 from,const char *old_name,u32 to,const char *new_name,u32 &replaced);
  bool reclaim(u32 number);
  // One atomic orphan-reclamation batch; caller commits before the next batch.
  bool reclaim_step(u32 number,bool &complete);
  bool orphan_head(u32 &number);
  bool recover_orphans();
  const Geometry &geometry() const { return geometry_; }
  Error error() const { return error_==Error::None ? journal_.error():error_; }
  Jbd &journal() { return journal_; }
private:
  class JournalDevice final : public JournalIo
  {
    Volume &volume_;
  public:
    constexpr explicit JournalDevice(Volume &volume):volume_(volume) {}
    unsigned block_size() const override { return volume_.geometry_.block_size; }
    u32 journal_capacity() const override { return volume_.journal_blocks_; }
    bool read_journal(u32 logical,void *data) override;
    bool write_journal(u32 logical,const void *data) override;
    bool read_home(u32 block,void *data) override;
    bool write_home(u32 block,const void *data) override;
    bool valid_target(u32 block) const override;
    bool flush() override { return volume_.io_.flush(); }
    bool recovery_flag(bool enabled) override { return volume_.set_recovery(enabled); }
    void *allocate_page() override { return volume_.io_.allocate_page(); }
    void free_page(void *page) override { volume_.io_.free_page(page); }
  };
  struct Mapping
  {
    Mapping *next=nullptr;
    unsigned count=0;
    u32 blocks[(4096-sizeof(void *)-sizeof(unsigned)-8)/sizeof(u32)];
  };
  struct Reserved
  {
    Reserved *next=nullptr;
    unsigned count=0;
    struct Range { u32 start,length; } ranges[(4096-sizeof(void *)-sizeof(unsigned)-8)/8];
  };
  Io &io_;
  JournalDevice journal_io_;
  Jbd journal_;
  Geometry geometry_{};
  Inode journal_inode_{};
  Mapping *mapping_=nullptr,*mapping_tail_=nullptr;
  Reserved *reserved_=nullptr;
  u32 journal_blocks_=0;
  bool ready_=false;
  Error error_=Error::None;
  void clear_error() { error_=Error::None;journal_.clear_error(); }
  bool fail(Error error) { error_=error;return false; }
  struct Operation;
  u8 *edit(u32 block);
  u8 *edit_super();
  u8 *edit_group(u32 number);
  bool put_inode(const Inode &inode,bool fresh=false);
  bool metadata_block(u32 block,u32 group,u32 bitmap,u32 inode_bitmap,u32 table) const;
  bool allocate_block(u32 preferred,u32 &block);
  bool free_block(u32 block);
  bool allocate_inode(u16 mode,u32 uid,u32 gid,u32 preferred,Inode &inode);
  bool free_inode(Inode &inode);
  bool map_allocated(Inode &inode,u64 logical,u32 &physical);
  bool resize_raw(Inode &inode,u64 size);
  bool prune(Inode &inode,u64 keep,unsigned &budget,bool &complete);
  bool orphan_step(Inode &inode,bool &complete);
  bool free_tree(Inode &inode,u32 block,unsigned level,u64 base,u64 keep,u32 &replacement,unsigned &budget,bool &complete);
  bool find_entry(const Inode &directory,const char *name,u64 &offset,u32 &number);
  bool add_entry(Inode &directory,const char *name,u32 number,u16 mode);
  bool remove_entry(Inode &directory,const char *name,u32 &number);
  bool empty_directory(const Inode &directory);
  bool add_orphan(Inode &inode);
  bool remove_orphan(Inode &inode);
  bool superblock();
  bool inode_location(u32 number,u32 &block,unsigned &offset);
  bool group(u32 number,u32 &block_bitmap,u32 &inode_bitmap,u32 &inode_table);
  bool map_block(const Inode &inode,u64 logical,u32 &physical,bool raw=false);
  bool pointer(u32 block,unsigned index,u32 &result,bool raw);
  bool read_block(u32 block,void *data,bool raw=false);
  bool journal_mapping();
  bool journal_block(u32 logical,u32 &physical) const;
  bool reserve_block(u32 block);
  bool reserved_block(u32 block) const;
  bool set_recovery(bool enabled);
};
}
