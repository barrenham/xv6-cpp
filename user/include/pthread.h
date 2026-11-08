#pragma once
#include "sys/types.h"
#include "time.h"
#ifdef __cplusplus
extern "C"
{
#endif
  typedef unsigned long pthread_t;

  typedef struct
  {
    size_t stack_size;
  } pthread_attr_t;

  typedef struct
  {
    int type;
  } pthread_mutexattr_t;

  typedef struct
  {
    int clock;
  } pthread_condattr_t;

  typedef struct
  {
    unsigned word;
    unsigned depth;
    pthread_t owner;
    int type;
  } pthread_mutex_t;

  typedef struct
  {
    unsigned sequence;
    unsigned waiters;
    int clock;
  } pthread_cond_t;

  typedef struct
  {
    int shared;
  } pthread_rwlockattr_t;

  typedef struct
  {
    pthread_mutex_t mutex;
    pthread_cond_t readers_changed, writers_changed;
    unsigned readers, waiting_writers;
    pthread_t writer;
  } pthread_rwlock_t;

#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_RECURSIVE 1
#define PTHREAD_MUTEX_ERRORCHECK 2
#define PTHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_NORMAL
#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1
#define PTHREAD_MUTEX_INITIALIZER {0, 0, 0, PTHREAD_MUTEX_DEFAULT}
#define PTHREAD_COND_INITIALIZER {0, 0, CLOCK_REALTIME}
#define PTHREAD_RWLOCK_INITIALIZER                                                     \
  {PTHREAD_MUTEX_INITIALIZER,                                                          \
   PTHREAD_COND_INITIALIZER,                                                           \
   PTHREAD_COND_INITIALIZER,                                                           \
   0,                                                                                  \
   0,                                                                                  \
   0}
#define PTHREAD_STACK_MIN 16384
  int pthread_attr_init(pthread_attr_t *);
  int pthread_attr_destroy(pthread_attr_t *);
  int pthread_attr_setstacksize(pthread_attr_t *, size_t);
  int pthread_attr_getstacksize(const pthread_attr_t *, size_t *);
  int pthread_create(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
  int pthread_join(pthread_t, void **);
#ifdef __cplusplus
  [[noreturn]]
#else
_Noreturn
#endif
  void pthread_exit(void *);
  pthread_t pthread_self(void);
  int pthread_equal(pthread_t, pthread_t);
  int pthread_kill(pthread_t, int);
  int pthread_mutexattr_init(pthread_mutexattr_t *);
  int pthread_mutexattr_destroy(pthread_mutexattr_t *);
  int pthread_mutexattr_settype(pthread_mutexattr_t *, int);
  int pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
  int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
  int pthread_mutex_destroy(pthread_mutex_t *);
  int pthread_mutex_lock(pthread_mutex_t *);
  int pthread_mutex_trylock(pthread_mutex_t *);
  int pthread_mutex_unlock(pthread_mutex_t *);
  int pthread_condattr_init(pthread_condattr_t *);
  int pthread_condattr_destroy(pthread_condattr_t *);
  int pthread_condattr_setclock(pthread_condattr_t *, clockid_t);
  int pthread_condattr_getclock(const pthread_condattr_t *, clockid_t *);
  int pthread_condattr_setpshared(pthread_condattr_t *, int);
  int pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *);
  int pthread_cond_destroy(pthread_cond_t *);
  int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
  int pthread_cond_timedwait(pthread_cond_t *, pthread_mutex_t *,
                             const struct timespec *);
  int pthread_rwlockattr_init(pthread_rwlockattr_t *);
  int pthread_rwlockattr_destroy(pthread_rwlockattr_t *);
  int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *, int);
  int pthread_rwlock_init(pthread_rwlock_t *, const pthread_rwlockattr_t *);
  int pthread_rwlock_destroy(pthread_rwlock_t *);
  int pthread_rwlock_rdlock(pthread_rwlock_t *);
  int pthread_rwlock_wrlock(pthread_rwlock_t *);
  int pthread_rwlock_tryrdlock(pthread_rwlock_t *);
  int pthread_rwlock_trywrlock(pthread_rwlock_t *);
  int pthread_rwlock_unlock(pthread_rwlock_t *);
  int pthread_cond_signal(pthread_cond_t *);
  int pthread_cond_broadcast(pthread_cond_t *);
#ifdef __cplusplus
}
#endif
