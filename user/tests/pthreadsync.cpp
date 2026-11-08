#include "user/runtime/user.h"
#include "user/include/pthread.h"
#include "user/runtime/proc.h"

namespace
{
alignas(4096) __thread unsigned page_tls = 31;
pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
unsigned readers, release_readers, writer_started, writer_done, ready, signalled;
unsigned active_readers, active_writers, value;

void
check(bool okay, const char *why) {
  if (!okay)
  {
    fprintf(2, "pthreadsync: FAIL: %s errno=%d\n", why, errno);
    exit(1);
  }
}

timespec
after(int clock, unsigned milliseconds) {
  timespec time;
  check(!clock_gettime(clock, &time), "read clock");
  time.tv_nsec += milliseconds * 1000000L;
  time.tv_sec += time.tv_nsec / 1000000000L;
  time.tv_nsec %= 1000000000L;
  return time;
}

void *
reader(void *) {
  check(!(reinterpret_cast<unsigned long>(&page_tls) % 4096) && page_tls == 31,
        "page-aligned initialized TLS");
  page_tls = 42;
  check(!pthread_rwlock_rdlock(&rw), "read lock");
  check(!pthread_rwlock_rdlock(&rw), "recursive read lock");
  __atomic_add_fetch(&readers, 1U, __ATOMIC_RELEASE);
  while (!__atomic_load_n(&release_readers, __ATOMIC_ACQUIRE))
  {
    sched_yield();
  }
  check(page_tls == 42, "aligned TLS survives sleeping and concurrent readers");
  check(!pthread_rwlock_unlock(&rw) && !pthread_rwlock_unlock(&rw), "read unlocks");
  return nullptr;
}

void *
writer(void *) {
  __atomic_store_n(&writer_started, 1U, __ATOMIC_RELEASE);
  check(!pthread_rwlock_wrlock(&rw), "writer waits for all readers");
  check(pthread_rwlock_tryrdlock(&rw) == EBUSY &&
            pthread_rwlock_trywrlock(&rw) == EBUSY &&
            pthread_rwlock_wrlock(&rw) == EDEADLK,
        "writer ownership and try locks");
  ++value;
  check(!pthread_rwlock_unlock(&rw), "write unlock");
  __atomic_store_n(&writer_done, 1U, __ATOMIC_RELEASE);
  return nullptr;
}

void *
stress(void *argument) {
  bool writing = argument != nullptr;
  for (unsigned n = 0; n < 128; ++n)
  {
    check(!(writing ? pthread_rwlock_wrlock(&rw) : pthread_rwlock_rdlock(&rw)),
          "stress lock");
    if (writing)
    {
      check(__atomic_add_fetch(&active_writers, 1U, __ATOMIC_ACQ_REL) == 1,
            "one writer at a time");
      check(!__atomic_load_n(&active_readers, __ATOMIC_ACQUIRE),
            "writer excludes readers");
      ++value;
      __atomic_sub_fetch(&active_writers, 1U, __ATOMIC_RELEASE);
    }
    else
    {
      __atomic_add_fetch(&active_readers, 1U, __ATOMIC_ACQ_REL);
      check(!__atomic_load_n(&active_writers, __ATOMIC_ACQUIRE),
            "reader excludes writer");
      sched_yield();
      __atomic_sub_fetch(&active_readers, 1U, __ATOMIC_RELEASE);
    }
    check(!pthread_rwlock_unlock(&rw), "stress unlock");
  }
  return nullptr;
}

void *
notifier(void *) {
  while (!__atomic_load_n(&ready, __ATOMIC_ACQUIRE))
  {
    sched_yield();
  }
  check(!pthread_mutex_lock(&mutex), "notifier lock");
  signalled = 1;
  check(!pthread_cond_signal(&condition) && !pthread_mutex_unlock(&mutex),
        "signal timed waiter");
  return nullptr;
}

void
timed(int clock) {
  pthread_condattr_t attr;
  clockid_t actual;
  check(!pthread_condattr_init(&attr) && !pthread_condattr_setclock(&attr, clock) &&
            !pthread_condattr_getclock(&attr, &actual) && actual == clock &&
            !pthread_cond_init(&condition, &attr),
        "condition clock attributes");
  check(!pthread_mutex_lock(&mutex), "timed waiter mutex");
  timespec bad{0, 1000000000};
  errno = 123;
  check(pthread_cond_timedwait(&condition, &mutex, &bad) == EINVAL && errno == 123,
        "invalid deadline preserves ownership and errno");
  timespec negative{-1, 0};
  check(pthread_cond_timedwait(&condition, &mutex, &negative) == ETIMEDOUT,
        "negative absolute seconds describe an already expired deadline");
  timespec expired{};
  check(pthread_cond_timedwait(&condition, &mutex, &expired) == ETIMEDOUT &&
            errno == 123,
        "past deadline reacquires mutex and returns positive error");
  auto deadline = after(clock, 25);
  auto began = clockticks();
  int result;
  do
  {
    result = pthread_cond_timedwait(&condition, &mutex, &deadline);
  } while (!result);
  check(result == ETIMEDOUT && clockticks() - began >= 200000,
        "timer queue expires a sleeping condition waiter");
  check(!pthread_mutex_unlock(&mutex), "timeout returned with mutex owned");
  for (unsigned round = 0; round < 16; ++round)
  {
    ready = signalled = 0;
    pthread_t thread;
    check(!pthread_mutex_lock(&mutex) &&
              !pthread_create(&thread, nullptr, notifier, nullptr),
          "timed signal fixture");
    __atomic_store_n(&ready, 1U, __ATOMIC_RELEASE);
    deadline = after(clock, 3000);
    while (!signalled)
    {
      check(!pthread_cond_timedwait(&condition, &mutex, &deadline),
            "notification beats deadline without a lost wakeup");
    }
    check(!pthread_mutex_unlock(&mutex) && !pthread_join(thread, nullptr),
          "join notifier");
  }
  check(!pthread_cond_destroy(&condition) && !pthread_condattr_destroy(&attr),
        "destroy timed fixtures");
}
} // namespace

extern "C" int
main() {
  pthread_t threads[4];
  check(!pthread_create(&threads[0], nullptr, reader, nullptr) &&
            !pthread_create(&threads[1], nullptr, reader, nullptr),
        "concurrent readers");
  while (__atomic_load_n(&readers, __ATOMIC_ACQUIRE) != 2)
  {
    sched_yield();
  }
  check(pthread_rwlock_trywrlock(&rw) == EBUSY && pthread_rwlock_destroy(&rw) == EBUSY,
        "readers exclude writers and destruction");
  check(!pthread_create(&threads[2], nullptr, writer, nullptr),
        "create waiting writer");
  while (!__atomic_load_n(&writer_started, __ATOMIC_ACQUIRE))
  {
    sched_yield();
  }
  poll(nullptr, 0, 5);
  check(!__atomic_load_n(&writer_done, __ATOMIC_ACQUIRE),
        "writer cannot overlap readers");
  static char snapshot[4096];
  check(proc::read("/proc/self/threads", snapshot, sizeof(snapshot)) > 0,
        "read per-process thread snapshot");
  unsigned rows = 0;
  for (char *at = snapshot; *at; ++at)
  {
    rows += *at == '\n';
  }
  check(rows == 4, "snapshot contains main, two readers and sleeping writer");
  __atomic_store_n(&release_readers, 1U, __ATOMIC_RELEASE);
  for (unsigned i = 0; i < 3; ++i)
  {
    check(!pthread_join(threads[i], nullptr), "join rw fixtures");
  }
  for (unsigned long i = 0; i < 4; ++i)
  {
    check(
        !pthread_create(&threads[i], nullptr, stress, reinterpret_cast<void *>(i & 1)),
        "stress threads");
  }
  for (auto thread : threads)
  {
    check(!pthread_join(thread, nullptr), "join stress");
  }
  check(value == 257 && !pthread_rwlock_destroy(&rw),
        "all writer increments and clean rwlock lifetime");
  timed(CLOCK_REALTIME);
  timed(CLOCK_MONOTONIC);
  printf(
      "pthreadsync: PASS: shared readers, exclusive writers, timed waits, mutex reacquisition and thread snapshots\n");
  return 0;
}
