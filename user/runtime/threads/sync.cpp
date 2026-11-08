#include "internal.h"

namespace xv_threads
{
void
lock(unsigned &word) {
  unsigned expected = 0;
  if (__atomic_compare_exchange_n(&word, &expected, 1U, false, __ATOMIC_ACQUIRE,
                                  __ATOMIC_RELAXED))
  {
    return;
  }
  while (__atomic_exchange_n(&word, 2U, __ATOMIC_ACQUIRE))
  {
    __xv_thread_word_wait(&word, 2U);
  }
}

void
unlock(unsigned &word) {
  if (__atomic_exchange_n(&word, 0U, __ATOMIC_RELEASE) == 2U)
  {
    __xv_thread_word_wake(&word, 1);
  }
}
} // namespace xv_threads

namespace
{
int
acquired(pthread_mutex_t *mutex) {
  mutex->depth = 1;
  __atomic_store_n(&mutex->owner, pthread_self(), __ATOMIC_RELAXED);
  return 0;
}

int
reenter(pthread_mutex_t *mutex, bool blocking) {
  if (__atomic_load_n(&mutex->owner, __ATOMIC_RELAXED) != pthread_self())
  {
    return EBUSY;
  }
  if (mutex->type == PTHREAD_MUTEX_RECURSIVE)
  {
    if (mutex->depth == ~0U)
    {
      return EAGAIN;
    }
    ++mutex->depth;
    return 0;
  }
  return blocking && mutex->type == PTHREAD_MUTEX_ERRORCHECK ? EDEADLK : EBUSY;
}
} // namespace

extern "C" int
pthread_mutexattr_init(pthread_mutexattr_t *attr) {
  if (!attr)
  {
    return EINVAL;
  }
  attr->type = PTHREAD_MUTEX_DEFAULT;
  return 0;
}

extern "C" int
pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
  return attr ? 0 : EINVAL;
}

extern "C" int
pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
  if (!attr || type < PTHREAD_MUTEX_NORMAL || type > PTHREAD_MUTEX_ERRORCHECK)
  {
    return EINVAL;
  }
  attr->type = type;
  return 0;
}

extern "C" int
pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int shared) {
  return !attr                               ? EINVAL
         : shared == PTHREAD_PROCESS_PRIVATE ? 0
         : shared == PTHREAD_PROCESS_SHARED  ? EOPNOTSUPP
                                             : EINVAL;
}

extern "C" int
pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
  if (!mutex)
  {
    return EINVAL;
  }
  if (attr &&
      (attr->type < PTHREAD_MUTEX_NORMAL || attr->type > PTHREAD_MUTEX_ERRORCHECK))
  {
    return EINVAL;
  }
  *mutex = {0, 0, 0, attr ? attr->type : PTHREAD_MUTEX_DEFAULT};
  return 0;
}

extern "C" int
pthread_mutex_destroy(pthread_mutex_t *mutex) {
  return !mutex ? EINVAL : __atomic_load_n(&mutex->word, __ATOMIC_ACQUIRE) ? EBUSY : 0;
}

extern "C" int
pthread_mutex_trylock(pthread_mutex_t *mutex) {
  if (!mutex)
  {
    return EINVAL;
  }
  unsigned expected = 0;
  if (__atomic_compare_exchange_n(&mutex->word, &expected, 1U, false, __ATOMIC_ACQUIRE,
                                  __ATOMIC_RELAXED))
  {
    return acquired(mutex);
  }
  return reenter(mutex, false);
}

extern "C" int
pthread_mutex_lock(pthread_mutex_t *mutex) {
  if (!mutex)
  {
    return EINVAL;
  }
  int result = pthread_mutex_trylock(mutex);
  if (result != EBUSY)
  {
    return result;
  }
  result = reenter(mutex, true);
  if (result != EBUSY)
  {
    return result;
  }
  xv_threads::lock(mutex->word);
  return acquired(mutex);
}

extern "C" int
pthread_mutex_unlock(pthread_mutex_t *mutex) {
  if (!mutex)
  {
    return EINVAL;
  }
  if (__atomic_load_n(&mutex->owner, __ATOMIC_RELAXED) != pthread_self())
  {
    return EPERM;
  }
  if (mutex->type == PTHREAD_MUTEX_RECURSIVE && mutex->depth > 1)
  {
    --mutex->depth;
    return 0;
  }
  mutex->depth = 0;
  __atomic_store_n(&mutex->owner, 0UL, __ATOMIC_RELAXED);
  xv_threads::unlock(mutex->word);
  return 0;
}

extern "C" int
pthread_condattr_init(pthread_condattr_t *attr) {
  if (!attr)
  {
    return EINVAL;
  }
  attr->clock = CLOCK_REALTIME;
  return 0;
}

extern "C" int
pthread_condattr_destroy(pthread_condattr_t *attr) {
  return attr ? 0 : EINVAL;
}

extern "C" int
pthread_condattr_setpshared(pthread_condattr_t *attr, int shared) {
  return !attr                               ? EINVAL
         : shared == PTHREAD_PROCESS_PRIVATE ? 0
         : shared == PTHREAD_PROCESS_SHARED  ? EOPNOTSUPP
                                             : EINVAL;
}

extern "C" int
pthread_cond_init(pthread_cond_t *condition, const pthread_condattr_t *attr) {
  if (!condition)
  {
    return EINVAL;
  }
  if (attr && attr->clock != CLOCK_REALTIME && attr->clock != CLOCK_MONOTONIC)
  {
    return EINVAL;
  }
  *condition = {0, 0, attr ? attr->clock : CLOCK_REALTIME};
  return 0;
}

extern "C" int
pthread_cond_destroy(pthread_cond_t *condition) {
  return !condition                                               ? EINVAL
         : __atomic_load_n(&condition->waiters, __ATOMIC_ACQUIRE) ? EBUSY
                                                                  : 0;
}

extern "C" int
pthread_cond_wait(pthread_cond_t *condition, pthread_mutex_t *mutex) {
  if (!condition || !mutex)
  {
    return EINVAL;
  }
  // A recursive mutex must be released completely before sleeping.
  if (__atomic_load_n(&mutex->owner, __ATOMIC_RELAXED) != pthread_self())
  {
    return EPERM;
  }
  if (mutex->depth != 1)
  {
    return EINVAL;
  }
  auto sequence = __atomic_load_n(&condition->sequence, __ATOMIC_ACQUIRE);
  __atomic_add_fetch(&condition->waiters, 1U, __ATOMIC_ACQ_REL);
  int status = pthread_mutex_unlock(mutex);
  if (!status)
  {
    __xv_thread_word_wait(&condition->sequence, sequence);
    status = pthread_mutex_lock(mutex);
  }
  __atomic_sub_fetch(&condition->waiters, 1U, __ATOMIC_RELEASE);
  return status;
}

extern "C" int
pthread_cond_signal(pthread_cond_t *condition) {
  if (!condition)
  {
    return EINVAL;
  }
  __atomic_add_fetch(&condition->sequence, 1U, __ATOMIC_RELEASE);
  __xv_thread_word_wake(&condition->sequence, 1);
  return 0;
}

extern "C" int
pthread_cond_broadcast(pthread_cond_t *condition) {
  if (!condition)
  {
    return EINVAL;
  }
  __atomic_add_fetch(&condition->sequence, 1U, __ATOMIC_RELEASE);
  __xv_thread_word_wake(&condition->sequence, 0x7fffffff);
  return 0;
}
