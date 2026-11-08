#include "internal.h"

extern "C" int
pthread_rwlockattr_init(pthread_rwlockattr_t *attr) {
  if (!attr)
  {
    return EINVAL;
  }
  attr->shared = PTHREAD_PROCESS_PRIVATE;
  return 0;
}

extern "C" int
pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr) {
  return attr ? 0 : EINVAL;
}

extern "C" int
pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int shared) {
  if (!attr)
  {
    return EINVAL;
  }
  if (shared == PTHREAD_PROCESS_SHARED)
  {
    return EOPNOTSUPP;
  }
  if (shared != PTHREAD_PROCESS_PRIVATE)
  {
    return EINVAL;
  }
  attr->shared = shared;
  return 0;
}

extern "C" int
pthread_rwlock_init(pthread_rwlock_t *lock, const pthread_rwlockattr_t *attr) {
  if (!lock)
  {
    return EINVAL;
  }
  if (attr && attr->shared != PTHREAD_PROCESS_PRIVATE)
  {
    return attr->shared == PTHREAD_PROCESS_SHARED ? EOPNOTSUPP : EINVAL;
  }
  *lock = PTHREAD_RWLOCK_INITIALIZER;
  return 0;
}

extern "C" int
pthread_rwlock_destroy(pthread_rwlock_t *lock) {
  if (!lock)
  {
    return EINVAL;
  }
  pthread_mutex_lock(&lock->mutex);
  bool busy = lock->writer || lock->readers || lock->waiting_writers ||
              __atomic_load_n(&lock->readers_changed.waiters, __ATOMIC_ACQUIRE);
  pthread_mutex_unlock(&lock->mutex);
  return busy ? EBUSY : 0;
}

namespace
{
int
acquire(pthread_rwlock_t *lock, bool write, bool blocking) {
  if (!lock)
  {
    return EINVAL;
  }
  pthread_mutex_lock(&lock->mutex);
  int result = 0;
  if (lock->writer == pthread_self())
  {
    result = blocking ? EDEADLK : EBUSY;
  }
  else if (write)
  {
    if (!blocking && (lock->writer || lock->readers))
    {
      result = EBUSY;
    }
    else
    {
      ++lock->waiting_writers;
      while (lock->writer || lock->readers)
      {
        pthread_cond_wait(&lock->writers_changed, &lock->mutex);
      }
      --lock->waiting_writers;
      lock->writer = pthread_self();
    }
  }
  else
  {
    if (!blocking && lock->writer)
    {
      result = EBUSY;
    }
    else
    {
      // Readers may acquire recursively even when a writer is waiting.
      // This basic implementation does not promise writer fairness.
      while (lock->writer)
      {
        pthread_cond_wait(&lock->readers_changed, &lock->mutex);
      }
      if (lock->readers == ~0U)
      {
        result = EAGAIN;
      }
      else
      {
        ++lock->readers;
      }
    }
  }
  pthread_mutex_unlock(&lock->mutex);
  return result;
}
} // namespace

extern "C" int
pthread_rwlock_rdlock(pthread_rwlock_t *lock) {
  return acquire(lock, false, true);
}

extern "C" int
pthread_rwlock_wrlock(pthread_rwlock_t *lock) {
  return acquire(lock, true, true);
}

extern "C" int
pthread_rwlock_tryrdlock(pthread_rwlock_t *lock) {
  return acquire(lock, false, false);
}

extern "C" int
pthread_rwlock_trywrlock(pthread_rwlock_t *lock) {
  return acquire(lock, true, false);
}

extern "C" int
pthread_rwlock_unlock(pthread_rwlock_t *lock) {
  if (!lock)
  {
    return EINVAL;
  }
  pthread_mutex_lock(&lock->mutex);
  int result = 0;
  if (lock->writer)
  {
    if (lock->writer != pthread_self())
    {
      result = EPERM;
    }
    else
    {
      lock->writer = 0;
    }
  }
  else if (lock->readers)
  {
    --lock->readers;
  }
  else
  {
    result = EPERM;
  }
  if (!result && !lock->writer)
  {
    // Wake readers as well as a writer: a racing reader may acquire first.
    // Waiters register while holding this same mutex. An empty reader wait
    // set needs neither a sequence update nor a kernel wakeup.
    if (__atomic_load_n(&lock->readers_changed.waiters, __ATOMIC_ACQUIRE))
    {
      pthread_cond_broadcast(&lock->readers_changed);
    }
    if (!lock->readers && lock->waiting_writers)
    {
      pthread_cond_signal(&lock->writers_changed);
    }
  }
  pthread_mutex_unlock(&lock->mutex);
  return result;
}
