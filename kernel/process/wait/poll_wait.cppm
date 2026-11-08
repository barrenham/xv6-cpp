module;
#include "kernel/support/deadline_queue.h"
export module xv.process:poll;
import :process;
import :poll_entry;
import xv.sync;

export namespace xv::process
{
class WaitChannels;

class PollWait final
{
  class Table final : public xv::sync::PollTable
  {
  public:
    void bind(PollWait &owner, void (*subscribe)(PollWait &, void *))
    {
      owner_ = &owner;
      subscribe_ = subscribe;
    }
    void watch(void *channel) override
    {
      if (subscribe_) subscribe_(*owner_, channel);
    }
  private:
    PollWait *owner_ = nullptr;
    void (*subscribe_)(PollWait &, void *) = nullptr;
  };
public:

  static constexpr unsigned InlineSubscriptions = 8;

  constexpr PollWait(Process &process, WaitChannels &channels)
      : process_(process), channels_(channels)
  {
  }

  ~PollWait();
  PollWait(const PollWait &) = delete;
  PollWait &operator=(const PollWait &) = delete;
  xv::sync::PollTable *table();
  void watch(void *channel);
  void set_deadline(unsigned long milliseconds);
  void cancel_deadline();
  void sleep(bool interruptible = true);

  bool failed() const
  {
    return failed_;
  }

  bool triggered() const;

private:

  struct WatchFlow;
  struct ReleaseFlow;
  struct SleepFlow;
  friend class WaitChannels;
  void notify();
  struct Page;
  Page *allocate_page();
  Process &process_;
  WaitChannels &channels_;
  Table table_;
  // Single-channel RPC waits and small polls must work with no free pages.
  // A three-lane FS pool watches six socket queues and one completion pipe.
  PollEntry inline_entries_[InlineSubscriptions]{};
  unsigned inline_used_ = 0;
  Page *pages_ = nullptr;
  bool failed_ = false;
  bool triggered_ = false;   // Protected by process_.lock.
  xv::time::Deadline timer_; // Protected by WaitChannels' timer lock.
};
} // namespace xv::process
