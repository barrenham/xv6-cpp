#include "user/runtime/user.h"
#include "user/include/pthread.h"
#include "kernel/process/wait/wait.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"

namespace
{
__thread unsigned initialized = 73;
__thread unsigned zeroed;
alignas(128) __thread unsigned char aligned_tls[256];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
unsigned waiting, released, counter;
pthread_t main_thread;

void
check(bool okay, const char *message) {
  if (!okay)
  {
    fprintf(2, "pthreadtest: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

void *
worker(void *argument) {
  auto index = reinterpret_cast<unsigned long>(argument);
  check(pthread_self() && !pthread_equal(pthread_self(), main_thread),
        "independent pthread identity");
  check(initialized == 73 && zeroed == 0 && errno == 0,
        "TLS initialized from ELF and zero-filled");
  check(!(reinterpret_cast<unsigned long>(aligned_tls) % 128) && !aligned_tls[0] &&
            !aligned_tls[255],
        "ELF TLS alignment and zero-fill in compact allocation");
  memset(aligned_tls, index + 1, sizeof(aligned_tls));
  initialized = 100 + index;
  zeroed = index + 1;
  errno = 200 + index;
  check(pthread_mutex_lock(&mutex) == 0, "lock shared mutex");
  ++waiting;
  pthread_cond_broadcast(&condition);
  while (!released)
  {
    check(pthread_cond_wait(&condition, &mutex) == 0,
          "condition releases and reacquires mutex");
  }
  check(pthread_mutex_unlock(&mutex) == 0, "unlock shared mutex");
  for (unsigned round = 0; round < 256; ++round)
  {
    unsigned bytes = 1 + (round * 97 + index * 23) % 8192;
    auto *memory = static_cast<unsigned char *>(malloc(bytes));
    check(memory, "concurrent allocation");
    memset(memory, index + 1, bytes);
    check(pthread_mutex_lock(&mutex) == 0, "contended mutex");
    ++counter;
    if (!(round % 32))
    {
      sched_yield();
    }
    check(pthread_mutex_unlock(&mutex) == 0, "contended unlock");
    sched_yield();
    for (unsigned byte = 0; byte < bytes; ++byte)
    {
      check(memory[byte] == index + 1, "allocator gives exclusive storage");
    }
    free(memory);
    check(initialized == 100 + index && zeroed == index + 1 &&
              aligned_tls[0] == index + 1 && aligned_tls[255] == index + 1 &&
              errno == int(200 + index),
          "TLS and errno survive switches and concurrent syscalls");
  }
  if (index & 1)
  {
    pthread_exit(reinterpret_cast<void *>(index + 0x100));
  }
  return reinterpret_cast<void *>(index + 0x100);
}

void *
fork_identity_worker(void *) {
  check(initialized == 73 && !zeroed && !pthread_kill(pthread_self(), 0),
        "new pthread after untouched fork has its own TLS and TID");
  return reinterpret_cast<void *>(0x52);
}

void
untouched_fork() {
  // No pthread lifecycle API has run yet. fork must leave this registry alone,
  // while later identity, signalling and thread creation still use the child.
  int child = fork();
  check(child >= 0, "fork before pthread lifecycle");
  if (!child)
  {
    check(pthread_self() == main_thread && !pthread_kill(main_thread, 0),
          "untouched fork preserves pthread handle with new kernel TID");
    pthread_mutex_t fresh = PTHREAD_MUTEX_INITIALIZER;
    check(!pthread_mutex_lock(&fresh) && !pthread_mutex_unlock(&fresh),
          "mutex after untouched fork");
    pthread_t thread;
    void *value = nullptr;
    check(!pthread_create(&thread, nullptr, fork_identity_worker, nullptr) &&
              !pthread_join(thread, &value) && value == reinterpret_cast<void *>(0x52),
          "create and join after untouched fork");
    exit(23);
  }
  int status;
  check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
            WEXITSTATUS(status) == 23 && !pthread_kill(main_thread, 0),
        "untouched fork keeps parent registry independent");
}

void
attributes() {
  pthread_attr_t attr;
  size_t bytes;
  check(!pthread_attr_init(&attr) && !pthread_attr_getstacksize(&attr, &bytes) &&
            bytes >= PTHREAD_STACK_MIN,
        "thread attributes default stack");
  check(pthread_attr_setstacksize(&attr, 1) == EINVAL &&
            !pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN) &&
            !pthread_attr_destroy(&attr),
        "stack size validation");
  pthread_mutexattr_t lock_attr;
  pthread_mutex_t test;
  check(!pthread_mutexattr_init(&lock_attr) &&
            pthread_mutexattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED) ==
                EOPNOTSUPP &&
            !pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_ERRORCHECK) &&
            !pthread_mutex_init(&test, &lock_attr),
        "mutex attributes reject unsupported process sharing");
  check(!pthread_mutex_lock(&test) && pthread_mutex_lock(&test) == EDEADLK &&
            pthread_mutex_trylock(&test) == EBUSY &&
            pthread_mutex_destroy(&test) == EBUSY && !pthread_mutex_unlock(&test) &&
            pthread_mutex_unlock(&test) == EPERM && !pthread_mutex_destroy(&test),
        "error-checking ownership and destroy");
  check(!pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_RECURSIVE) &&
            !pthread_mutex_init(&test, &lock_attr) && !pthread_mutex_lock(&test) &&
            !pthread_mutex_lock(&test) && !pthread_mutex_unlock(&test) &&
            pthread_mutex_destroy(&test) == EBUSY && !pthread_mutex_unlock(&test) &&
            !pthread_mutex_destroy(&test),
        "recursive mutex depth");
  check(!pthread_mutexattr_destroy(&lock_attr), "destroy mutex attributes");
}

void
round() {
  waiting = released = counter = 0;
  pthread_t threads[4];
  errno = 321;
  for (unsigned i = 0; i < 4; ++i)
  {
    check(!pthread_create(&threads[i], nullptr, worker,
                          reinterpret_cast<void *>(static_cast<unsigned long>(i))),
          "create joinable pthread");
  }
  check(!pthread_mutex_lock(&mutex), "main mutex");
  while (waiting != 4)
  {
    check(!pthread_cond_wait(&condition, &mutex), "wait for all workers");
  }
  check(pthread_cond_destroy(&condition) == EBUSY,
        "condition cannot be destroyed while in use");
  released = 1;
  check(!pthread_cond_broadcast(&condition) && !pthread_mutex_unlock(&mutex),
        "release all sleepers");
  for (unsigned i = 0; i < 4; ++i)
  {
    void *result = nullptr;
    check(!pthread_join(threads[i], &result) &&
              reinterpret_cast<unsigned long>(result) == i + 0x100,
          "join returns explicit exit and normal return values");
    check(pthread_join(threads[i], nullptr) == ESRCH, "joined handle removed");
  }
  check(counter == 4 * 256 && initialized == 73 && !zeroed && errno == 321,
        "shared counter and main TLS");
}

void
first_handler(int) {
}

void
second_handler(int) {
}

void *
signal_worker(void *) {
  sigset_t blocked = 1U << SIGUSR1, previous = 0;
  check(!sigprocmask(SIG_UNBLOCK, &blocked, &previous) && previous == blocked,
        "new thread inherits mask and may change its own mask");
  struct sigaction action{second_handler, 1U << SIGUSR2, SA_RESTART}, old{};
  check(!sigaction(SIGUSR1, &action, &old) && old.sa_handler == first_handler,
        "worker sees and replaces shared disposition");
  return nullptr;
}

void
signal_actions() {
  struct sigaction initial{first_handler, 0, 0}, saved{}, current{};
  sigset_t blocked = 1U << SIGUSR1, saved_mask = 0, current_mask = 0;
  check(!sigaction(SIGUSR1, &initial, &saved) &&
            !sigprocmask(SIG_BLOCK, &blocked, &saved_mask),
        "shared disposition fixture");
  pthread_t thread;
  check(!pthread_create(&thread, nullptr, signal_worker, nullptr) &&
            !pthread_join(thread, nullptr),
        "join disposition writer");
  check(!sigaction(SIGUSR1, nullptr, &current) &&
            current.sa_handler == second_handler &&
            current.sa_mask == (1U << SIGUSR2) && current.sa_flags == SA_RESTART &&
            !sigprocmask(SIG_BLOCK, nullptr, &current_mask) &&
            current_mask == (saved_mask | blocked),
        "actions are process-wide while masks stay per-thread");
  check(!sigaction(SIGUSR1, &saved, nullptr) &&
            !sigprocmask(SIG_SETMASK, &saved_mask, nullptr),
        "restore signal fixture");
}

unsigned signal_ready, signal_gate;
pthread_t signal_observed;

void
routing_handler(int) {
  ++zeroed;
  __atomic_store_n(&signal_observed, pthread_self(), __ATOMIC_RELEASE);
}

void *
routing_worker(void *argument) {
  int signal = argument ? SIGUSR2 : SIGUSR1;
  unsigned initial = argument ? 199 : 99;
  zeroed = initial;
  __atomic_store_n(&signal_ready, 1U, __ATOMIC_RELEASE);
  while (!__atomic_load_n(&signal_gate, __ATOMIC_ACQUIRE))
  {
    sched_yield();
  }
  sigset_t blocked = 1U << signal;
  check(!sigprocmask(SIG_UNBLOCK, &blocked, nullptr),
        "unblock queued process signal in worker");
  while (!__atomic_load_n(&signal_observed, __ATOMIC_ACQUIRE))
  {
    pause(1);
  }
  check(__atomic_load_n(&signal_observed, __ATOMIC_ACQUIRE) == pthread_self() &&
            zeroed == initial + 1,
        "process handler executes in the eligible worker with its TLS");
  return reinterpret_cast<void *>(0xabc);
}

void
signal_routing() {
  signal_ready = signal_gate = 0;
  signal_observed = 0;
  struct sigaction action{routing_handler, 0, 0}, saved{};
  sigset_t blocked = 1U << SIGUSR1, saved_mask = 0, pending = 0;
  check(!sigaction(SIGUSR1, &action, &saved) &&
            !sigprocmask(SIG_BLOCK, &blocked, &saved_mask),
        "process signal routing fixture");
  pthread_t worker;
  check(!pthread_create(&worker, nullptr, routing_worker, nullptr),
        "start blocked signal worker");
  while (!__atomic_load_n(&signal_ready, __ATOMIC_ACQUIRE))
  {
    sched_yield();
  }
  check(!sigsend(getpid(), SIGUSR1) && !sigpending(&pending) && (pending & blocked),
        "blocked process signal is visible until an eligible thread exists");
  __atomic_store_n(&signal_gate, 1U, __ATOMIC_RELEASE);
  void *result;
  check(!pthread_join(worker, &result) && result == reinterpret_cast<void *>(0xabc) &&
            __atomic_load_n(&signal_observed, __ATOMIC_ACQUIRE) == worker && !zeroed,
        "queued process signal is delivered once outside the masked main thread");
  check(!sigaction(SIGUSR1, &saved, nullptr) &&
            !sigprocmask(SIG_SETMASK, &saved_mask, nullptr),
        "restore process routing fixture");
}

void
directed_signals() {
  signal_ready = signal_gate = 0;
  signal_observed = 0;
  struct sigaction action{routing_handler, 0, 0}, saved{};
  sigset_t blocked = 1U << SIGUSR2, saved_mask = 0, pending = 0;
  check(!sigaction(SIGUSR2, &action, &saved) &&
            !sigprocmask(SIG_BLOCK, &blocked, &saved_mask),
        "directed signal fixture");
  pthread_t worker;
  check(!pthread_create(&worker, nullptr, routing_worker, reinterpret_cast<void *>(1)),
        "start directed signal worker");
  while (!__atomic_load_n(&signal_ready, __ATOMIC_ACQUIRE))
  {
    sched_yield();
  }
  check(!sigprocmask(SIG_UNBLOCK, &blocked, nullptr) && !pthread_kill(worker, 0) &&
            !pthread_kill(pthread_self(), 0) && pthread_kill(worker, 31) == EINVAL &&
            pthread_kill(0, 0) == ESRCH,
        "pthread_kill validates live thread and signal");
  check(!pthread_kill(worker, SIGUSR2), "post signal to blocked worker");
  pause(1);
  check(!__atomic_load_n(&signal_observed, __ATOMIC_ACQUIRE) && !sigpending(&pending) &&
            !(pending & blocked),
        "thread-directed signal cannot migrate to unblocked main thread");
  __atomic_store_n(&signal_gate, 1U, __ATOMIC_RELEASE);
  check(!pthread_join(worker, nullptr) &&
            __atomic_load_n(&signal_observed, __ATOMIC_ACQUIRE) == worker && !zeroed,
        "directed signal is delivered after target unblocks it");
  check(!sigaction(SIGUSR2, &saved, nullptr) &&
            !sigprocmask(SIG_SETMASK, &saved_mask, nullptr),
        "restore directed signal fixture");
}

void *
finish_main(void *argument) {
  void *value = nullptr;
  check(!pthread_join(main_thread, &value) && value == reinterpret_cast<void *>(0x44),
        "join main pthread");
  auto fd = reinterpret_cast<unsigned long>(argument);
  signal_observed = 0;
  struct sigaction action{routing_handler, 0, 0}, previous{};
  check(!sigaction(SIGUSR2, &action, &previous),
        "shared disposition survives main-thread exit");
  check(!sigsend(getpid(), SIGUSR2), "send process signal after main-thread exit");
  while (!__atomic_load_n(&signal_observed, __ATOMIC_ACQUIRE))
  {
    pause(1);
  }
  check(__atomic_load_n(&signal_observed, __ATOMIC_ACQUIRE) == pthread_self(),
        "remaining worker receives process signals after main-thread exit");
  check(write(fd, "R", 1) == 1, "I/O after main thread exits");
  return nullptr;
}

int child_events[2];
pthread_t child_recipient;

void
child_handler(int signal) {
  if (signal == SIGCHLD)
  {
    __atomic_store_n(&child_recipient, pthread_self(), __ATOMIC_RELEASE);
    write(child_events[1], "C", 1);
  }
}

void *
child_waiter(void *argument) {
  sigset_t blocked = 1U << SIGCHLD;
  check(!sigprocmask(SIG_UNBLOCK, &blocked, nullptr),
        "worker accepts child signal blocked by leader");
  int ready = static_cast<int>(reinterpret_cast<unsigned long>(argument));
  check(write(ready, "R", 1) == 1, "child signal waiter ready");
  pollfd event{child_events[0], POLLIN, 0};
  int result;
  do
  {
    result = poll(&event, 1, 3000);
  } while (result < 0 && errno == EINTR);
  check(result == 1 && (event.revents & POLLIN),
        "child exit wakes eligible worker blocked on unrelated descriptor");
  char byte;
  check(read(child_events[0], &byte, 1) == 1 && byte == 'C' &&
            __atomic_load_n(&child_recipient, __ATOMIC_ACQUIRE) == pthread_self(),
        "SIGCHLD delivered in worker with leader mask preserved");
  return nullptr;
}

void
child_routing() {
  int ready[2];
  check(!pipe(child_events) && !pipe(ready), "child signal routing pipes");
  child_recipient = 0;
  struct sigaction action{child_handler, 0, 0}, saved{};
  sigset_t blocked = 1U << SIGCHLD, previous;
  check(!sigaction(SIGCHLD, &action, &saved) &&
            !sigprocmask(SIG_BLOCK, &blocked, &previous),
        "block child signal in leader");
  pthread_t thread;
  check(!pthread_create(&thread, nullptr, child_waiter,
                        reinterpret_cast<void *>(static_cast<unsigned long>(ready[1]))),
        "create eligible child signal waiter");
  char byte;
  check(read(ready[0], &byte, 1) == 1, "waiter unblocked child signal");
  int child = fork();
  check(child >= 0, "fork with child signal waiter");
  if (!child)
  {
    exit(17);
  }
  check(!pthread_join(thread, nullptr), "join notified child signal waiter");
  int status;
  check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
            WEXITSTATUS(status) == 17,
        "worker notification preserves child wait status");
  check(!sigaction(SIGCHLD, &saved, nullptr) &&
            !sigprocmask(SIG_SETMASK, &previous, nullptr),
        "restore child signal routing state");
  close(ready[0]);
  close(ready[1]);
  close(child_events[0]);
  close(child_events[1]);
}

void
main_exit() {
  int descriptors[2];
  check(!pipe(descriptors), "main exit pipe");
  int child = fork();
  check(child >= 0, "single-thread fork after pthread use");
  if (!child)
  {
    check(pthread_self() == main_thread, "fork keeps calling pthread identity");
    close(descriptors[0]);
    pthread_t thread;
    check(!pthread_create(
              &thread, nullptr, finish_main,
              reinterpret_cast<void *>(static_cast<unsigned long>(descriptors[1]))),
          "worker after fork");
    pthread_exit(reinterpret_cast<void *>(0x44));
  }
  close(descriptors[1]);
  char byte = 0;
  check(read(descriptors[0], &byte, 1) == 1 && byte == 'R', "worker keeps group alive");
  int status;
  check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
            WEXITSTATUS(status) == 0,
        "last pthread exits process normally");
  close(descriptors[0]);
}
} // namespace

extern "C" int
main() {
  main_thread = pthread_self();
  check(main_thread && initialized == 73 && !zeroed &&
            !(reinterpret_cast<unsigned long>(aligned_tls) % 128) && !aligned_tls[0] &&
            !aligned_tls[255],
        "initial thread aligned TLS");
  untouched_fork();
  check(pthread_join(main_thread, nullptr) == EDEADLK &&
            pthread_join(1, nullptr) == ESRCH,
        "join validation");
  attributes();
  for (unsigned i = 0; i < 5; ++i)
  {
    round();
  }
  signal_actions();
  signal_routing();
  directed_signals();
  child_routing();
  main_exit();
  free(nullptr);
  check(!pthread_cond_destroy(&condition) && !pthread_mutex_destroy(&mutex),
        "destroy idle synchronization objects");
  printf(
      "pthreadtest: PASS: pthread lifecycle, TLS, errno, mutex, condition and concurrent allocation\n");
  return 0;
}
