#include "user/runtime/user.h"
#include "user/include/pthread.h"
#include "kernel/process/wait/wait.h"

namespace
{
struct Shared
{
  unsigned ready, started;
  int leaderless;
  unsigned long counters[4];
};

Shared *shared;
pthread_t initial_thread;

void
check(bool okay, const char *message) {
  if (!okay)
  {
    fprintf(2, "threadstop: FAIL: %s\n", message);
    exit(1);
  }
}

[[noreturn]]
void
count(unsigned index) {
  for (;;)
  {
    __atomic_add_fetch(&shared->counters[index], 1UL, __ATOMIC_RELAXED);
    sched_yield();
  }
}

void *
worker(void *argument) {
  auto index = static_cast<unsigned>(reinterpret_cast<unsigned long>(argument));
  __atomic_add_fetch(&shared->ready, 1U, __ATOMIC_RELEASE);
  if (index == 1 && shared->leaderless)
  {
    while (__atomic_load_n(&shared->ready, __ATOMIC_ACQUIRE) != 3)
    {
      sched_yield();
    }
    void *result = nullptr;
    check(!pthread_join(initial_thread, &result) &&
              result == reinterpret_cast<void *>(0x55),
          "confirm initial thread has exited before group control");
    __atomic_store_n(&shared->started, 1U, __ATOMIC_RELEASE);
  }
  count(index);
}

void
run(int leaderless) {
  shared = static_cast<Shared *>(
      mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
  check(shared != MAP_FAILED, "shared control mapping");
  *shared = {};
  shared->leaderless = leaderless;
  int child = fork();
  check(child >= 0, "fork group fixture");
  if (!child)
  {
    initial_thread = pthread_self();
    pthread_t threads[3];
    for (unsigned i = 0; i < 3; ++i)
    {
      check(
          !pthread_create(&threads[i], nullptr, worker,
                          reinterpret_cast<void *>(static_cast<unsigned long>(i + 1))),
          "create group worker");
    }
    if (leaderless)
    {
      pthread_exit(reinterpret_cast<void *>(0x55));
    }
    while (__atomic_load_n(&shared->ready, __ATOMIC_ACQUIRE) != 3)
    {
      sched_yield();
    }
    __atomic_store_n(&shared->started, 1U, __ATOMIC_RELEASE);
    count(0);
  }
  while (!__atomic_load_n(&shared->started, __ATOMIC_ACQUIRE))
  {
    sched_yield();
  }
  for (unsigned cycle = 0; cycle < 3; ++cycle)
  {
    int status = 0;
    check(!sigsend(child, SIGSTOP) && waitpid(child, &status, WUNTRACED) == child &&
              WIFSTOPPED(status) && WSTOPSIG(status) == SIGSTOP,
          "one process stop notification");
    check(waitpid(child, &status, WUNTRACED | WNOHANG) == 0,
          "threads do not generate duplicate process stop events");
    unsigned long before[4];
    for (unsigned i = leaderless ? 1 : 0; i < 4; ++i)
    {
      before[i] = __atomic_load_n(&shared->counters[i], __ATOMIC_ACQUIRE);
    }
    pause(2);
    for (unsigned i = leaderless ? 1 : 0; i < 4; ++i)
    {
      check(before[i] == __atomic_load_n(&shared->counters[i], __ATOMIC_ACQUIRE),
            "every live thread stops before parent observes the stop");
    }
    if (cycle == 2)
    {
      check(!sigsend(child, SIGKILL) && waitpid(child, &status, 0) == child &&
                WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL,
            "kill and reap a stopped group");
      break;
    }
    check(!sigsend(child, SIGCONT) && waitpid(child, &status, WCONTINUED) == child &&
              WIFCONTINUED(status),
          "one process continue notification");
    check(waitpid(child, &status, WCONTINUED | WNOHANG) == 0,
          "no duplicate continue events");
    bool progressing = false;
    for (unsigned attempt = 0; attempt < 10 && !progressing; ++attempt)
    {
      pause(1);
      progressing = true;
      for (unsigned i = leaderless ? 1 : 0; i < 4; ++i)
      {
        progressing = progressing && __atomic_load_n(&shared->counters[i],
                                                     __ATOMIC_ACQUIRE) > before[i];
      }
    }
    check(progressing, "every thread resumes after continue");
  }
  check(!munmap(shared, 4096), "release group control mapping");
}
} // namespace

extern "C" int
main() {
  run(0);
  run(1);
  printf(
      "threadstop: PASS: whole-group stop/continue, one parent event, exited main thread and stopped-group kill\n");
  return 0;
}
