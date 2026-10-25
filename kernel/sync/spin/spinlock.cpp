module xv.sync;

import :platform;

namespace xv::sync
{
void SpinLock::init(const char *label)
{
  name_ = label;
  locked_ = 0;
  owner_ = nullptr;
}

void SpinLock::acquire()
{
  detail::push_interrupts_off();
  if (holding())
  {
    detail::fail("spinlock: recursive acquire");
  }

  // Acquire/release ordering publishes the protected data between CPUs.
  while (__atomic_exchange_n(&locked_, 1, __ATOMIC_ACQUIRE) != 0)
  {
  }
  __atomic_store_n(&owner_, detail::current_cpu(), __ATOMIC_RELAXED);
}

void SpinLock::release()
{
  if (!holding())
  {
    detail::fail("spinlock: release by non-owner");
  }

  __atomic_store_n(&owner_, static_cast<void *>(nullptr), __ATOMIC_RELAXED);
  __atomic_store_n(&locked_, 0, __ATOMIC_RELEASE);
  detail::pop_interrupts_off();
}

bool SpinLock::holding() const
{
  // On the kernel platform the caller must have interrupts disabled, so that
  // its CPU identity cannot change. Owner reads are atomic even on contenders.
  return __atomic_load_n(&locked_, __ATOMIC_RELAXED) != 0 &&
         __atomic_load_n(&owner_, __ATOMIC_RELAXED) == detail::current_cpu();
}
} // namespace xv::sync
