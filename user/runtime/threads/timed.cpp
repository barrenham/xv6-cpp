#include "internal.h"
extern "C" long __xv_clock_gettime(int, timespec *);

extern "C" int
pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock) {
  if (!attr || (clock != CLOCK_REALTIME && clock != CLOCK_MONOTONIC))
  {
    return EINVAL;
  }
  attr->clock = clock;
  return 0;
}

extern "C" int
pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock) {
  if (!attr || !clock)
  {
    return EINVAL;
  }
  *clock = attr->clock;
  return 0;
}

namespace
{
int
deadline(int clock, const timespec *absolute, unsigned long &milliseconds) {
  if (!absolute || absolute->tv_nsec < 0 || absolute->tv_nsec >= 1000000000)
  {
    return EINVAL;
  }
  timespec now{}, monotonic{};
  auto result = __xv_clock_gettime(clock, &now);
  if (result < 0)
  {
    return -result;
  }
  result = __xv_clock_gettime(CLOCK_MONOTONIC, &monotonic);
  if (result < 0)
  {
    return -result;
  }
  if (absolute->tv_sec < now.tv_sec ||
      (absolute->tv_sec == now.tv_sec && absolute->tv_nsec <= now.tv_nsec))
  {
    milliseconds = 1;
    return 0;
  }
  auto seconds = static_cast<unsigned long>(absolute->tv_sec - now.tv_sec);
  long fraction = absolute->tv_nsec - now.tv_nsec;
  if (fraction < 0)
  {
    --seconds;
    fraction += 1000000000;
  }
  auto base = static_cast<unsigned long>(monotonic.tv_sec) * 1000 +
              (monotonic.tv_nsec + 999999) / 1000000;
  milliseconds = seconds > (~0UL - base - 1000) / 1000
                     ? ~0UL
                     : base + seconds * 1000 + (fraction + 999999) / 1000000;
  if (!milliseconds)
  {
    milliseconds = 1;
  }
  return 0;
}
} // namespace

extern "C" int
pthread_cond_timedwait(pthread_cond_t *condition, pthread_mutex_t *mutex,
                       const timespec *absolute) {
  if (!condition || !mutex)
  {
    return EINVAL;
  }
  if (__atomic_load_n(&mutex->owner, __ATOMIC_RELAXED) != pthread_self())
  {
    return EPERM;
  }
  if (mutex->depth != 1)
  {
    return EINVAL;
  }
  unsigned long until;
  int status = deadline(condition->clock, absolute, until);
  if (status)
  {
    return status;
  }
  auto sequence = __atomic_load_n(&condition->sequence, __ATOMIC_ACQUIRE);
  __atomic_add_fetch(&condition->waiters, 1U, __ATOMIC_ACQ_REL);
  status = pthread_mutex_unlock(mutex);
  if (!status)
  {
    int waited = __xv_thread_word_timedwait(&condition->sequence, sequence, until);
    int locked = pthread_mutex_lock(mutex);
    status = locked ? locked : waited == -ETIMEDOUT ? ETIMEDOUT : 0;
  }
  __atomic_sub_fetch(&condition->waiters, 1U, __ATOMIC_RELEASE);
  return status;
}
