#include "user/runtime/user.h"
#include "user/include/pthread.h"
#include "kernel/process/wait/wait.h"
#include "kernel/fs/fcntl.h"
#include "user/include/mqueue.h"

namespace
{
constexpr unsigned Page = 4096, Workers = 7;

struct Context
{
  char *page;
  int descriptor;
  unsigned index;
} contexts[Workers];

struct Coordination
{
  unsigned parent_done, child_ok;
} *coordination;

unsigned ready, start;

void
check(bool okay, const char *message) {
  if (!okay)
  {
    fprintf(2, "threadio: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

void *
reader(void *argument) {
  auto &context = *static_cast<Context *>(argument);
  __atomic_add_fetch(&ready, 1U, __ATOMIC_RELEASE);
  while (!__atomic_load_n(&start, __ATOMIC_ACQUIRE))
  {
    sched_yield();
  }
  // This address was populated before fork and has never been written since.
  // The read itself must resolve its COW fault while other threads are active.
  long count = context.index == 2
                   ? mq_receive(context.descriptor, context.page + 128, 1, nullptr)
                   : read(context.descriptor, context.page + 128, 1);
  check(
      count == 1 &&
          (context.index == 6 || context.page[128] == char('A' + context.index)),
      "pipe/socket/message/shm/PTY/TTY/random read resolves COW without an outer spin lock");
  return reinterpret_cast<void *>(static_cast<unsigned long>(context.index + 1));
}

struct ForkContext
{
  int descriptor, group;
};

void *
worker_fork(void *argument) {
  auto &context = *static_cast<ForkContext *>(argument);
  auto identity = pthread_self();
  int child = fork();
  check(child >= 0, "fork from a non-initial thread");
  if (!child)
  {
    check(
        getpid() != context.group && pthread_self() == identity &&
            !pthread_kill(identity, 0),
        "worker fork child keeps only its calling thread with a new process identity");
    check(write(context.descriptor, "W", 1) == 1,
          "worker fork child inherits shared descriptors");
    char *arguments[] = {const_cast<char *>("echo"),
                         const_cast<char *>("threadio-worker-child"), nullptr};
    exec("/bin/echo", arguments);
    exit(1);
  }
  int status;
  check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
            !WEXITSTATUS(status),
        "worker waits its process's child after successful exec");
  return reinterpret_cast<void *>(0xf00);
}

void
fork_from_worker() {
  int descriptors[2];
  check(!pipe(descriptors), "worker fork pipe");
  ForkContext context{descriptors[1], getpid()};
  pthread_t thread;
  void *value;
  check(!pthread_create(&thread, nullptr, worker_fork, &context) &&
            !pthread_join(thread, &value) && value == reinterpret_cast<void *>(0xf00),
        "join the forking worker");
  char byte;
  check(read(descriptors[0], &byte, 1) == 1 && byte == 'W' && !close(descriptors[0]) &&
            !close(descriptors[1]),
        "worker fork descriptor lifetime");
}

void *
change_mappings(void *argument) {
  unsigned char value = reinterpret_cast<unsigned long>(argument);
  for (unsigned i = 0; i < 32; ++i)
  {
    auto *page = static_cast<volatile unsigned char *>(mmap(
        nullptr, Page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    check(page != MAP_FAILED, "concurrent mapping allocation");
    page[0] = value;
    page[Page - 1] = value + 1;
    void *address = const_cast<unsigned char *>(page);
    check(!mprotect(address, Page, PROT_READ) && page[0] == value &&
              page[Page - 1] == static_cast<unsigned char>(value + 1),
          "cross-hart permission downgrade preserves data");
    sched_yield();
    check(!mprotect(address, Page, PROT_READ | PROT_WRITE),
          "concurrent permission upgrade");
    page[0] = value + 2;
    check(!munmap(address, Page), "cross-hart unmap and page reclamation");
  }
  return argument;
}

void
mapping_churn() {
  pthread_t threads[4];
  for (unsigned long i = 0; i < 4; ++i)
  {
    check(!pthread_create(&threads[i], nullptr, change_mappings,
                          reinterpret_cast<void *>(i + 1)),
          "create mapping worker");
  }
  for (unsigned long i = 0; i < 4; ++i)
  {
    void *value;
    check(!pthread_join(threads[i], &value) && value == reinterpret_cast<void *>(i + 1),
          "join mapping worker after all page releases");
  }
}

void
round(char *pages) {
  ready = start = 0;
  coordination->parent_done = coordination->child_ok = 0;
  memset(pages, 'Z', Workers * Page);
  pthread_t threads[Workers];
  int descriptors[Workers][2];
  for (unsigned i = 0; i < Workers; ++i)
  {
    if (i == 0)
    {
      check(!pipe(descriptors[i]), "create worker pipe");
    }
    else if (i == 1)
    {
      check(!socketpair(AF_UNIX, SOCK_STREAM, 0, descriptors[i]),
            "create local socket pair");
    }
    else if (i == 2)
    {
      mq_attr attributes{};
      attributes.mq_maxmsg = 4;
      attributes.mq_msgsize = 1;
      descriptors[i][0] =
          mq_open("/threadio-queue", O_CREAT | O_EXCL | O_RDWR, 0600, &attributes);
      check(descriptors[i][0] >= 0 && !mq_unlink("/threadio-queue"),
            "create unlinked worker message queue");
      descriptors[i][1] = descriptors[i][0];
    }
    else if (i == 3)
    {
      descriptors[i][0] = shm_open("/threadio-memory", O_CREAT | O_EXCL | O_RDWR, 0600);
      check(descriptors[i][0] >= 0 && !shm_unlink("/threadio-memory") &&
                !ftruncate(descriptors[i][0], Page),
            "create unlinked shared memory object");
      char byte = 'A' + i;
      check(write(descriptors[i][0], &byte, 1) == 1 &&
                lseek(descriptors[i][0], 0, SEEK_SET) == 0,
            "prepare shared memory file offset");
      descriptors[i][1] = descriptors[i][0];
    }
    else if (i == 6)
    {
      descriptors[i][0] = open("/dev/urandom", O_RDONLY);
      check(descriptors[i][0] >= 0, "open entropy for a COW destination");
      descriptors[i][1] = descriptors[i][0];
    }
    else
    {
      check(!openpty(descriptors[i]), "create PTY worker pair");
      if (i == 5)
      {
        int master = descriptors[i][0];
        descriptors[i][0] = descriptors[i][1];
        descriptors[i][1] = master;
      }
    }
    contexts[i] = {pages + i * Page, descriptors[i][0], i};
    check(!pthread_create(&threads[i], nullptr, reader, &contexts[i]),
          "create pipe reader");
  }
  while (__atomic_load_n(&ready, __ATOMIC_ACQUIRE) != Workers)
  {
    sched_yield();
  }
  int child = fork();
  check(child >= 0, "fork shared multithreaded address space");
  if (!child)
  {
    while (!__atomic_load_n(&coordination->parent_done, __ATOMIC_ACQUIRE))
    {
      sched_yield();
    }
    for (unsigned i = 0; i < Workers; ++i)
    {
      check(pages[i * Page + 128] == 'Z',
            "kernel copyout preserves the child's private snapshot");
    }
    __atomic_store_n(&coordination->child_ok, 1U, __ATOMIC_RELEASE);
    exit(0);
  }
  __atomic_store_n(&start, 1U, __ATOMIC_RELEASE);
  for (unsigned i = 0; i < Workers; ++i)
  {
    char bytes[2] = {char('A' + i), '\n'};
    if (i == 2)
    {
      check(!mq_send(descriptors[i][1], bytes, 1, 3),
            "send message during concurrent COW faults");
    }
    else if (i != 3 && i != 6)
    {
      unsigned count = i == 5 ? 2 : 1;
      check(write(descriptors[i][1], bytes, count) == int(count),
            "write IPC/terminal input while concurrent readers fault");
    }
  }
  for (unsigned i = 0; i < Workers; ++i)
  {
    void *value;
    check(!pthread_join(threads[i], &value) &&
              reinterpret_cast<unsigned long>(value) == i + 1 &&
              (i == 6 || pages[i * Page + 128] == char('A' + i)),
          "joined reader publishes exactly its own data");
  }
  for (unsigned i = 0; i < Workers; ++i)
  {
    check(!close(descriptors[i][0]), "close worker input descriptor");
    if (descriptors[i][1] != descriptors[i][0])
    {
      check(!close(descriptors[i][1]), "close worker peer descriptor");
    }
  }
  __atomic_store_n(&coordination->parent_done, 1U, __ATOMIC_RELEASE);
  int status;
  check(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
            !WEXITSTATUS(status) &&
            __atomic_load_n(&coordination->child_ok, __ATOMIC_ACQUIRE),
        "reap single-threaded fork child with unchanged private data");
}
} // namespace

extern "C" int
main() {
  auto *pages =
      static_cast<char *>(mmap(nullptr, Workers * Page, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  coordination = static_cast<Coordination *>(
      mmap(nullptr, Page, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
  check(pages != MAP_FAILED && coordination != MAP_FAILED,
        "map private destinations and shared coordination");
  for (unsigned i = 0; i < 3; ++i)
  {
    round(pages);
  }
  fork_from_worker();
  mapping_churn();
  check(!munmap(pages, Workers * Page) && !munmap(coordination, Page),
        "release thread I/O fixtures");
  printf(
      "threadio: PASS: concurrent pipe/socket/MQ/shm/PTY/TTY/random COW, private isolation, worker fork/exec and concurrent VM\n");
  return 0;
}
