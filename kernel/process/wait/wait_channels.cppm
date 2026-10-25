module xv.process:wait_channels;

import :process;
import :scheduler;
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
  void prepare(Process &process, void *channel);
  void cancel(Process &process);
  void wake(void *channel);

private:

  static constexpr unsigned BucketCount = 128;

  struct Bucket
  {
    xv::sync::SpinLock lock;
    Process *head = nullptr;
  };

  static unsigned index(void *channel);
  static void unlink(Bucket &bucket, Process &process);
  Scheduler &scheduler_;
  Bucket buckets_[BucketCount]{};
};

WaitChannels &wait_channels();
} // namespace xv::process
