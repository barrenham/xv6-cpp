module xv.process;

import :wait_channels;

namespace xv::process
{
// Callers hold the condition lock while preparing, release it, then sleep.
void prepare_sleep(void *channel)
{
  wait_channels().prepare(*current(), channel);
}

// A notification before this point clears chan, so it cannot be lost here.
void sleep()
{
  Process *p = current();

  p->lock.acquire();
  if (p->chan != 0)
  {
    p->state = ProcessState::Sleeping;
    scheduler().suspend();
  }
  p->lock.release();
}

} // namespace xv::process
