export module xv.process:poll;
import :process;
import xv.sync;

export namespace xv::process
{
class WaitChannels;

class PollWait final
{
public:

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
  void sleep();

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
  Page *pages_ = nullptr;
  bool failed_ = false;
  bool triggered_ = false;    // Protected by process_.lock.
  bool timer_linked_ = false; // Protected by WaitChannels' timer lock.
  unsigned long deadline_ = 0;
  PollWait *timer_next_ = nullptr;
};
} // namespace xv::process
