#include "user/runtime/user.h"
#include "user/runtime/threads/internal.h"
#include "kernel/fs/fcntl.h"
#include "kernel/process/wait/wait.h"
#include "kernel/process/signals/signal.h"

extern "C" long __xv_thread_create(unsigned long, unsigned long, unsigned long,
                                   unsigned long);
extern "C" long __xv_thread_join(int, unsigned long *);
extern "C" [[noreturn]]
void __xv_thread_exit(unsigned long);
extern "C" int __xv_gettid();
extern "C" int __xv_thread_word_wait(unsigned *, unsigned);
extern "C" int __xv_thread_word_wake(unsigned *, int);

namespace
{
constexpr unsigned Page = 4096, StackBytes = 5 * Page;
unsigned ready = 0, start = 0, lock_word = 0, lifetime_word = 0;
unsigned protected_counter = 0;

void
lock() {
  unsigned expected = 0;
  if (__atomic_compare_exchange_n(&lock_word, &expected, 1U, false, __ATOMIC_ACQUIRE,
                                  __ATOMIC_RELAXED))
  {
    return;
  }
  while (__atomic_exchange_n(&lock_word, 2U, __ATOMIC_ACQUIRE))
  {
    __xv_thread_word_wait(&lock_word, 2U);
  }
}

void
unlock() {
  if (__atomic_exchange_n(&lock_word, 0U, __ATOMIC_RELEASE) == 2U)
  {
    __xv_thread_word_wake(&lock_word, 1);
  }
}

void
check(bool okay, const char *message) {
  if (!okay)
  {
    fprintf(2, "threadcore: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

struct Context
{
  unsigned index = 0, completed = 0;
  int group = 0, tid = 0, descriptor = -1;
  unsigned long tls = 0;
  void *stack = nullptr;
  xv_threads::Control *control = nullptr;
} contexts[4];

void *
stack() {
  auto *mapping = static_cast<char *>(mmap(nullptr, StackBytes, PROT_READ | PROT_WRITE,
                                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  check(mapping != MAP_FAILED && mprotect(mapping, Page, PROT_NONE) == 0,
        "allocate guarded user stack");
  return mapping;
}

unsigned long
tls() {
  unsigned long value;
  asm volatile("mv %0, tp" : "=r"(value));
  return value;
}

[[noreturn]]
void
worker(Context *context) {
  volatile unsigned long local[16];
  for (unsigned i = 0; i < 16; ++i)
  {
    local[i] = (context->index + 1) * 1000 + i;
  }
  context->tid = __xv_gettid();
  check(context->tid != context->group && getpid() == context->group,
        "distinct TID and shared process PID");
#ifdef LAB_PGTBL
  check(ugetpid() == context->group, "shared USYSCALL process identity");
#endif
  __atomic_add_fetch(&ready, 1U, __ATOMIC_RELEASE);
  while (!__atomic_load_n(&start, __ATOMIC_ACQUIRE))
  {
    sched_yield();
  }
  for (unsigned round = 0; round < 64; ++round)
  {
    check(tls() == context->tls && __xv_gettid() == context->tid,
          "trap return preserves per-thread tp and registers");
    for (unsigned i = 0; i < 16; ++i)
    {
      check(local[i] == (context->index + 1) * 1000 + i, "independent user stacks");
    }
    lock();
    auto previous = protected_counter;
    if (round < 4)
    {
      pause(1); // force other threads to queue while ownership is held
    }
    protected_counter = previous + 1;
    unlock();
    ++context->completed;
    sched_yield();
  }
  if (context->descriptor >= 0)
  {
    char byte = 0;
    check(read(context->descriptor, &byte, 1) == 1 && byte == 'A' &&
              close(context->descriptor) == 0,
          "shared descriptor table");
    check(chdir("/tmp") == 0, "shared working directory update");
    umask(0077);
  }
  __xv_thread_exit(0x12340000UL + context->index);
}

void
batch(int descriptor = -1) {
  ready = start = 0;
  lock_word = protected_counter = 0;
  int tids[4];
  for (unsigned i = 0; i < 4; ++i)
  {
    contexts[i] = {};
    contexts[i].index = i;
    contexts[i].group = getpid();
    contexts[i].control = xv_threads::allocate(0);
    check(contexts[i].control, "allocate raw-thread TLS");
    contexts[i].tls = contexts[i].control->thread_pointer;
    contexts[i].descriptor = i ? -1 : descriptor;
    contexts[i].stack = stack();
    tids[i] = __xv_thread_create(reinterpret_cast<unsigned long>(worker),
                                 reinterpret_cast<unsigned long>(&contexts[i]),
                                 reinterpret_cast<unsigned long>(contexts[i].stack) +
                                     StackBytes,
                                 contexts[i].tls);
    check(tids[i] > 0, "create independently scheduled thread");
  }
  while (__atomic_load_n(&ready, __ATOMIC_ACQUIRE) != 4)
  {
    sched_yield();
  }
  __atomic_store_n(&start, 1U, __ATOMIC_RELEASE);
  for (unsigned i = 0; i < 4; ++i)
  {
    unsigned long result = 0;
    check(__xv_thread_join(tids[i], &result) == 0 && result == 0x12340000UL + i &&
              contexts[i].tid == tids[i] && contexts[i].completed == 64,
          "join returns result and publishes shared writes");
    check(__xv_thread_join(tids[i], nullptr) == -ESRCH, "joined thread is reclaimed");
    check(munmap(contexts[i].stack, StackBytes) == 0, "release joined thread stack");
    xv_threads::release(contexts[i].control);
  }
  check(protected_counter == 4 * 64, "sleeping word mutex prevents lost updates");
}

unsigned long
free_kib() {
  char bytes[2048];
  int fd = open("/proc/meminfo", O_RDONLY);
  check(fd >= 0, "open memory statistics");
  int count = read(fd, bytes, sizeof(bytes) - 1);
  close(fd);
  check(count > 0, "read memory statistics");
  bytes[count] = 0;
  for (char *line = bytes; *line;)
  {
    if (strlen(line) >= 8 && !memcmp(line, "MemFree:", 8))
    {
      line += 8;
      while (*line == ' ')
      {
        ++line;
      }
      return atoi(line);
    }
    while (*line && *line != '\n')
    {
      ++line;
    }
    if (*line)
    {
      ++line;
    }
  }
  check(false, "MemFree field");
  return 0;
}

struct Child
{
  int descriptor = -1, leader = 0, mode = 0;
} child;

[[noreturn]]
void
child_worker(Child *value) {
  if (value->mode == 1)
  {
    // Joining the initial thread proves it has exited before the worker's I/O.
    unsigned long result = 0;
    check(__xv_thread_join(value->leader, &result) == 0 && result == 0x55,
          "join initial thread without destroying process resources");
    check(write(value->descriptor, "R", 1) == 1, "I/O survives initial thread exit");
    for (;;)
    {
      __xv_thread_word_wait(&lifetime_word, 0);
    }
  }
  for (;;)
  {
    sched_yield();
  }
}

void
lifetime(int mode) {
  int descriptors[2];
  check(pipe(descriptors) == 0, "lifetime pipe");
  int process = fork();
  check(process >= 0, "lifetime subprocess");
  if (!process)
  {
    close(descriptors[0]);
    child = {descriptors[1], getpid(), mode};
    auto *mapping = stack();
    auto *control = xv_threads::allocate(0);
    check(control, "lifetime TLS");
    check(__xv_thread_create(reinterpret_cast<unsigned long>(child_worker),
                             reinterpret_cast<unsigned long>(&child),
                             reinterpret_cast<unsigned long>(mapping) + StackBytes,
                             control->thread_pointer) > 0,
          "lifetime worker");
    if (mode == 1)
    {
      __xv_thread_exit(0x55);
    }
    exit(37);
  }
  close(descriptors[1]);
  if (mode == 1)
  {
    char byte = 0;
    check(read(descriptors[0], &byte, 1) == 1 && byte == 'R',
          "process stays alive after initial thread exit");
    int ignored = 0;
    check(waitpid(process, &ignored, WNOHANG) == 0,
          "live group is not reported as exited");
    check(sigsend(process, SIGKILL) == 0, "kill group with exited initial thread");
  }
  int status = -1;
  check(waitpid(process, &status, 0) == process, "reap process and unjoined threads");
  check(mode == 1 ? WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL
                  : WIFEXITED(status) && WEXITSTATUS(status) == 37,
        "process-wide termination preserves exit status");
  close(descriptors[0]);
}
} // namespace

extern "C" int
main() {
  check(__xv_gettid() == getpid(), "initial TID equals process PID");
  check(__xv_thread_word_wait(&lock_word, 99) == -EAGAIN &&
            __xv_thread_word_wait(reinterpret_cast<unsigned *>(1), 0) < 0 &&
            __xv_thread_word_wake(&lock_word, 0) == 0,
        "wait-word validation and compare-before-sleep");
  check(__xv_thread_join(__xv_gettid(), nullptr) == -EDEADLK, "reject self join");
  check(__xv_thread_join(0x7fffffff, nullptr) == -ESRCH, "reject foreign TID");
  check(__xv_thread_create(0, 0, 0, 0) < 0, "reject invalid thread entry/stack");
  int fd = open("/tmp/threadcore-data", O_CREATE | O_TRUNC | O_RDWR);
  check(fd >= 0 && write(fd, "AB", 2) == 2 && lseek(fd, 0, SEEK_SET) == 0,
        "shared descriptor fixture");
  batch(fd);
  char byte;
  check(read(fd, &byte, 1) < 0 && errno == EBADF,
        "worker close is visible to initial thread");
  char directory[64];
  check(getcwd(directory, sizeof(directory)) && !strcmp(directory, "/tmp"),
        "cwd is process-wide");
  check(umask(0022) == 0077 && chdir("/") == 0, "credentials mask is process-wide");
  auto before = free_kib();
  for (unsigned round = 0; round < 20; ++round)
  {
    batch();
  }
  auto after = free_kib();
  check(after + 256 >= before,
        "repeated join reclaims task, frame, stack and resource references");
  lifetime(0);
  lifetime(1);
  check(unlink("/tmp/threadcore-data") == 0, "remove fixture");
  printf(
      "threadcore: PASS: shared resources, independent contexts, sleeping mutex, join, main-thread exit and group termination\n");
  return 0;
}
