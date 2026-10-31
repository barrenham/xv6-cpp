#pragma once
#include "format.h"

namespace xv::ext3
{
// The caller serializes this object. Filesystem blocks are 1, 2 or 4 KiB;
// scratch allocations always provide an aligned, 4 KiB usable page.
class JournalIo
{
public:
  virtual unsigned block_size() const = 0;
  virtual u32 journal_capacity() const = 0;
  virtual bool read_journal(u32 logical,void *data) = 0;
  virtual bool write_journal(u32 logical,const void *data) = 0;
  virtual bool read_home(u32 block,void *data) = 0;
  virtual bool write_home(u32 block,const void *data) = 0;
  virtual bool valid_target(u32 block) const = 0;
  virtual bool flush() = 0;
  // Set/clear EXT3_FEATURE_INCOMPAT_RECOVER on the home superblock. Setting
  // also updates any staged superblock so checkpoint cannot clear it early.
  virtual bool recovery_flag(bool enabled) = 0;
  virtual void *allocate_page() = 0;
  virtual void free_page(void *page) = 0;
protected:
  ~JournalIo() = default;
};

class Jbd final
{
public:
  static constexpr unsigned MaxDirty=256;
  constexpr explicit Jbd(JournalIo &io):io_(io) {}
  Jbd(const Jbd &)=delete;
  Jbd &operator=(const Jbd &)=delete;
  bool mount();
  bool recover();
  bool commit();
  // Staged buffers are independent of the filesystem cache. Home writes occur
  // only after a durable JBD commit; reads observe pending transaction data.
  u8 *edit(u32 block);
  bool read(u32 block,void *destination);
  u8 *staged(u32 block);
  bool savepoint();
  void accept();
  void rollback();
  void reset();
  Error error() const { return error_; }
  void clear_error() { error_=Error::None; }
  unsigned dirty_count() const { return count_; }
  u32 sequence() const { return sequence_; }
  unsigned capacity() const;
private:
  struct Dirty { u32 block=0;u8 *bytes=nullptr; } dirty_[MaxDirty]{};
  struct Undo { unsigned index=0;u8 *previous=nullptr; } undo_[MaxDirty]{};
  struct Revocations;
  JournalIo &io_;
  unsigned count_=0,undo_count_=0,saved_count_=0;
  u32 first_=0,length_=0,sequence_=0,start_=0;
  bool mounted_=false,saving_=false;
  Error error_=Error::None;
  u8 uuid_[16]{};
  u32 next(u32 block) const { return block+1<length_ ? block+1:first_; }
  bool fail(Error error) { error_=error;return false; }
  bool write_super(u32 start,u32 sequence);
  bool scan(unsigned pass,u32 &end_sequence,Revocations *revocations);
  bool incomplete(unsigned pass,u32 sequence,u32 cursor);
};
}
