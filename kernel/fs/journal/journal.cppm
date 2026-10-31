module;
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"

export module xv.fs:journal;
export import :cache;

export namespace xv::fs
{
class Journal
{
public:
  Journal(const Journal &)=delete;
  Journal &operator=(const Journal &)=delete;
  virtual void begin()=0;
  virtual void end()=0;
  virtual void record(Buffer &buffer)=0;
  virtual void sync()=0;
protected:
  constexpr Journal()=default;
  ~Journal()=default;
};

Journal &journal();

class Transaction final
{
public:

  inline explicit Transaction(Journal &journal) : journal_(&journal)
  {
    journal_->begin();
  }

  inline explicit Transaction(bool persistent)
      : journal_(persistent ? &journal() : nullptr)
  {
    if (journal_)
    {
      journal_->begin();
      other_ = secondary;
      if (other_)
      {
        other_->begin();
      }
    }
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
      if (other_)
      {
        other_->end();
        other_ = nullptr;
      }
      journal_->end();
      journal_ = nullptr;
    }
  }

private:

  Journal *journal_;
  Journal *other_ = nullptr;
};
} // namespace xv::fs
