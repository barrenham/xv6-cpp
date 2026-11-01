module;
#include "kernel/support/deadline_queue.h"
module xv.process:wait_channels;

import :process;
import :scheduler;
import :poll_entry;
import xv.sync;

namespace xv::process
{
// Bucket count affects lookup collisions, never the number of waiters.
class WaitChannels final
{
public:

  constexpr explicit WaitChannels(Scheduler &scheduler) : scheduler_(scheduler)
  {
  }

  void initialize();
  void prepare(Process &process, void *channel, bool interruptible = false);
  void cancel(Process &process);
  void wake(void *channel);
  void subscribe(PollEntry &entry, void *channel);
  void unsubscribe(PollEntry &entry);
  void arm_timer(PollWait &wait, unsigned long deadline);
  void remove_timer(PollWait &wait);
  void expire_timers(unsigned long milliseconds);
  unsigned long next_timer();

private:

  struct CancelFlow;
  struct PrepareFlow;
  struct WakeFlow;

  friend class PollWait;
  xv::sync::SpinLock timer_lock_;
  xv::time::DeadlineQueue timers_;
  unsigned long next_deadline_ =
      ~0UL; // Numeric cache, safe without foreign stack reads.
  static constexpr unsigned BucketCount = 128;

  struct Bucket
  {
    xv::sync::SpinLock lock;
    Process *head = nullptr;
    PollEntry *pollers = nullptr;
  };

  static unsigned index(void *channel);
  static void unlink(Bucket &bucket, Process &process);
  Scheduler &scheduler_;
  Bucket buckets_[BucketCount]{};
};

WaitChannels &wait_channels();
} // namespace xv::process
