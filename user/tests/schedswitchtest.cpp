#include "user/runtime/user.h"
#include "kernel/io/scheduler.h"
#include "kernel/fs/fcntl.h"
#include "pthread.h"

namespace
{
unsigned stopping = 0;
unsigned long progress[4]{};
constexpr unsigned policies[] = {XV_SCHED_CFS, XV_SCHED_MLFQ, XV_SCHED_ROUND_ROBIN};

void check(bool valid, const char *message) {
  if (!valid)
  {
    fprintf(2, "schedswitchtest: %s\n", message);
    exit(1);
  }
}

xv_scheduler_status status() {
  xv_scheduler_status value{};
  check(schedctl(XV_SCHED_GET, 0, &value) == 0 && value.size == sizeof(value) &&
        value.policy <= XV_SCHED_MLFQ && value.running > 0, "consistent status");
  return value;
}

void *worker(void *argument) {
  unsigned index = reinterpret_cast<unsigned long>(argument);
  int file = -1;
  if (index == 1)
  {
    file = open("/tmp/scheduler-switch-io", O_CREATE | O_RDWR | O_TRUNC);
    check(file >= 0, "I/O worker open");
  }
  char bytes[64]{};
  unsigned iteration = 0;
  while (!__atomic_load_n(&stopping, __ATOMIC_ACQUIRE))
  {
    if (index == 0)
    {
      volatile unsigned long value = 1;
      for (unsigned i = 0; i < 10000; ++i) value = value * 1664525 + 1013904223;
      sched_yield();
    }
    else if (index == 1)
    {
      check(lseek(file, 0, 0) == 0 && write(file, bytes, sizeof(bytes)) == sizeof(bytes),
            "I/O survives switching");
    }
    else if (index == 3)
    {
      check(schedctl(XV_SCHED_SET, policies[iteration++ % 3], nullptr) == 0,
            "concurrent policy controller");
      pause(1);
    }
    else pause(1);
    __atomic_add_fetch(&progress[index], 1UL, __ATOMIC_RELAXED);
  }
  if (file >= 0) close(file);
  return nullptr;
}
} // namespace

extern "C" int main() {
  auto original = status();
  check(schedctl(XV_SCHED_SET, original.policy, nullptr) == 0, "no-op switch");
  check(schedctl(XV_SCHED_SET, 99, nullptr) == -1 && errno == EINVAL,
        "invalid policy rejected");
  check(schedctl(99, 0, &original) == -1 && errno == EINVAL, "invalid operation rejected");
  check(schedctl(XV_SCHED_SET, original.policy, &original) == -1 && errno == EINVAL,
        "set rejects output pointer before mutation");
  check(schedctl(XV_SCHED_GET, 0, nullptr) == -1 && errno == EFAULT,
        "invalid status buffer rejected");
  check(status().changes == original.changes, "invalid and no-op requests preserve generation");
  int child = fork();
  check(child >= 0, "permission fork");
  if (!child)
  {
    check(setuid(1000) == 0, "drop root");
    (void)status();
    check(schedctl(XV_SCHED_SET, XV_SCHED_MLFQ, nullptr) == -1 && errno == EPERM,
          "kernel rejects unprivileged change");
    exit(0);
  }
  int code = -1;
  check(wait(&code) == child && code == 0, "permission child");

  int stop[2], results[2];
  check(pipe(stop) == 0 && pipe(results) == 0, "worker pipes");
  for (unsigned index = 0; index < 4; ++index)
  {
    int pid = fork();
    check(pid >= 0, "CPU worker fork");
    if (!pid)
    {
      close(stop[1]); close(results[0]);
      check(fcntl(stop[0], F_SETFL, O_NONBLOCK) == 0, "nonblocking stop pipe");
      unsigned long work = 0;
      char token;
      for (;;)
      {
        int count = read(stop[0], &token, 1);
        if (!count) break;
        check(count < 0 && errno == EAGAIN, "CPU worker stop wait");
        volatile unsigned long value = index + 1;
        for (unsigned j = 0; j < 10000; ++j) value = value * 1664525 + 1013904223;
        ++work;
      }
      check(write(results[1], &work, sizeof(work)) == sizeof(work), "CPU worker result");
      close(stop[0]); close(results[1]); exit(0);
    }
  }
  close(stop[0]); close(results[1]);
  pthread_t threads[4];
  for (unsigned i = 0; i < 4; ++i)
    check(pthread_create(&threads[i], nullptr, worker, reinterpret_cast<void *>(i)) == 0,
          "background thread create");
  unsigned long last = original.changes;
  for (unsigned i = 0; i < 30; ++i)
  {
    check(schedctl(XV_SCHED_SET, policies[i % 3], nullptr) == 0, "live policy change");
    auto current = status();
    check(current.changes >= last, "concurrent changes monotonically published");
    last = current.changes;
    pause(1);
  }
  __atomic_store_n(&stopping, 1U, __ATOMIC_RELEASE);
  for (unsigned i = 0; i < 4; ++i)
  {
    check(pthread_join(threads[i], nullptr) == 0 && progress[i] > 0,
          "every background thread progresses and joins");
  }
  close(stop[1]);
  for (unsigned i = 0; i < 4; ++i)
  {
    unsigned long work = 0;
    check(read(results[0], &work, sizeof(work)) == sizeof(work) && work > 0,
          "every CPU process makes progress");
    check(wait(&code) > 0 && code == 0, "CPU worker reaped");
  }
  close(results[0]);
  check(last > original.changes + 10, "multiple real transitions exercised");
  check(schedctl(XV_SCHED_SET, original.policy, nullptr) == 0, "restore starting policy");
  printf("schedswitchtest: PASS processes, threads, I/O, sleep, concurrent changes and permissions\n");
  return 0;
}
