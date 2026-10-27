module;
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"

export module xv.fs:journal;
export import :cache;

export namespace xv::fs
{
class Journal final
{
public:

  constexpr explicit Journal(BlockCache &cache) : cache_(cache)
  {
  }

  Journal(const Journal &) = delete;
  Journal &operator=(const Journal &) = delete;
  void initialize(int device, const superblock *layout);
  void begin();
  void end();
  void record(Buffer &buffer);
  void sync();

private:

  struct Header
  {
    int count = 0;
    int blocks[LOGBLOCKS]{};
  } header_;

  static_assert(sizeof(Header) < BlockSize);
  void recover();
  void install(int recovering);
  void read_header();
  void write_header();
  void write_log();
  void commit();
  BlockCache &cache_;
  xv::sync::SpinLock lock_;
  int start_ = 0;
  int outstanding_ = 0;
  int committing_ = 0;
  int device_ = 0;
  int commits_ = 0;
};

Journal &journal();

class Transaction final
{
public:

  inline explicit Transaction(Journal &journal) : journal_(&journal)
  {
    journal_->begin();
  }

  inline explicit Transaction(bool persistent) : journal_(persistent ? &journal() : nullptr)
  {
    if (journal_) { journal_->begin(); other_ = secondary; if (other_) other_->begin(); }
  }

  inline static Journal *secondary = nullptr;

  inline Transaction() : Transaction(true)
  {
  }

  inline ~Transaction()
  {
    finish();
  }

  Transaction(const Transaction &) = delete;
  Transaction &operator=(const Transaction &) = delete;

  inline void finish()
  {
    if (journal_)
    {
      if (other_) { other_->end(); other_ = nullptr; }
      journal_->end();
      journal_ = nullptr;
    }
  }

private:

  Journal *journal_;
  Journal *other_ = nullptr;
};
} // namespace xv::fs
