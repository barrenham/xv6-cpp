module xv.process;
import :fifo_queue;
import :storage_platform;

namespace xv::process
{
void FifoReadyQueue::initialize()
{
  lock_.init("ready queue");
}

void FifoReadyQueue::enqueue(Process &process)
{
  if (!process_locked(process))
  {
    detail::storage_fail("ready queue: process not locked");
  }
  xv::sync::ScopedLock guard(lock_);
  if (queued(process))
  {
    detail::storage_fail("ready queue: duplicate enqueue");
  }
  queued(process) = true;
  next(process) = nullptr;
  if (tail_)
  {
    next(*tail_) = &process;
  }
  else
  {
    head_ = &process;
  }
  tail_ = &process;
}

Process *FifoReadyQueue::take(CpuState &cpu)
{
  xv::sync::ScopedLock guard(lock_);
  Process *previous = nullptr;
  for (auto *process = head_; process; process = next(*process))
  {
    if (!eligible(*process, cpu))
    {
      previous = process;
      continue;
    }
    if (previous)
    {
      next(*previous) = next(*process);
    }
    else
    {
      head_ = next(*process);
    }
    if (tail_ == process)
    {
      tail_ = previous;
    }
    next(*process) = nullptr;
    queued(*process) = false;
    return process;
  }
  return nullptr;
}
} // namespace xv::process
