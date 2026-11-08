module;
#if __STDC_HOSTED__
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <sys/mman.h>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#define XVFS_ERROR(...) std::fprintf(stderr, __VA_ARGS__)
#else
#include "user/runtime/user.h"
#include "user/include/pthread.h"
#include "kernel/fs/poll.h"
#define XVFS_ERROR(...) fprintf(2, __VA_ARGS__)
#endif
#include "kernel/fs/userfs/protocol.h"
#include "kernel/support/placement_new.hpp"
module xv.userfs;

namespace userfs
{
namespace
{
constexpr unsigned Lanes = xv::userfs::TransportLanes;

[[noreturn]]
void
fail(const char *reason) {
  XVFS_ERROR("fs-thread-pool: %s errno=%d\n", reason, errno);
  exit(1);
}

struct Pool
{
  struct Worker
  {
    Pool *pool = nullptr;
    pthread_t thread = 0;
    pthread_cond_t changed = PTHREAD_COND_INITIALIZER;
    xv::userfs::Message message;
    unsigned index = 0, lane = 0;
    bool busy = false, assigned = false, complete = false, maintenance = false;
    int result = 0;
  } workers[2];

  Backend &backend;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_rwlock_t namespace_lock = PTHREAD_RWLOCK_INITIALIZER;
  pthread_rwlock_t inodes[64]{};
  int channels[Lanes]{-1, -1, -1}, completion[2]{-1, -1};
  bool lane_busy[Lanes]{}, stopping = false, owned = false;
  unsigned lanes = 1, count = 0, payload = DataSize, mapping_bytes = 0;
  unsigned maintenance_pending = 0, maintenance_delay = 0;
  void *mapping = nullptr;

  explicit Pool(Backend &value) : backend(value) {
    for (auto &inode : inodes)
    {
      pthread_rwlock_init(&inode, nullptr);
    }
    maintenance_pending = backend.maintenance_pending();
    maintenance_delay = backend.maintenance_delay_ms();
  }

  ~Pool() {
    pthread_mutex_lock(&mutex);
    stopping = true;
    for (auto &worker : workers)
    {
      pthread_cond_signal(&worker.changed);
    }
    pthread_mutex_unlock(&mutex);
    for (auto &worker : workers)
    {
      if (worker.thread)
      {
        pthread_join(worker.thread, nullptr);
      }
    }
    for (int fd : completion)
    {
      if (fd >= 0)
      {
        close(fd);
      }
    }
    if (owned)
    {
      for (int fd : channels)
      {
        if (fd >= 0)
        {
          close(fd);
        }
      }
    }
    if (mapping)
    {
      munmap(mapping, mapping_bytes);
    }
  }

  // The dispatcher alone owns busy/lane_busy. The mutex publishes jobs and
  // completions; a worker never changes another worker's request or bulk slice.
  bool
  busy() const {
    for (unsigned i = 0; i < count; ++i)
    {
      if (workers[i].busy)
      {
        return true;
      }
    }
    return false;
  }

  void
  healthy() {
    for (unsigned i = 0; i < count; ++i)
    {
      if (workers[i].busy && pthread_kill(workers[i].thread, 0))
      {
        fail("worker exited while a callback may own locks");
      }
    }
  }

  bool
  complete() {
    char index;
    if (read(completion[0], &index, 1) != 1 || unsigned(index) >= count)
    {
      return false;
    }
    pthread_mutex_lock(&mutex);
    auto &worker = workers[unsigned(index)];
    bool okay = worker.busy && worker.complete && !worker.result;
    if (!okay)
    {
      XVFS_ERROR(
          "fs-thread-pool: completion worker=%u busy=%d done=%d result=%d maintenance=%d operation=%u id=%lu\n",
          unsigned(index), int(worker.busy), int(worker.complete), worker.result,
          int(worker.maintenance), unsigned(worker.message.operation),
          worker.message.id);
    }
    if (okay)
    {
      worker.busy = worker.complete = false;
      if (!worker.maintenance)
      {
        lane_busy[worker.lane] = false;
      }
    }
    pthread_mutex_unlock(&mutex);
    return okay;
  }

  void
  wait() {
    while (busy())
    {
      pollfd pending{completion[0], POLLIN, 0};
      int ready = poll(&pending, 1, 100);
      if (ready < 0 && (errno == EINTR || errno == ENOMEM || errno == EAGAIN))
      {
        if (errno != EINTR)
        {
          poll(nullptr, 0, 1);
        }
        continue;
      }
      if (ready < 0 || (ready && !complete()))
      {
        fail("completion wait");
      }
      healthy();
    }
  }

  void
  assign(unsigned index, unsigned lane, const xv::userfs::Message *message) {
    pthread_mutex_lock(&mutex);
    auto &worker = workers[index];
    worker.maintenance = message == nullptr;
    worker.lane = lane;
    if (message)
    {
      worker.message = *message;
      lane_busy[lane] = true;
    }
    worker.busy = worker.assigned = true;
    worker.complete = false;
    worker.result = 0;
    pthread_cond_signal(&worker.changed);
    pthread_mutex_unlock(&mutex);
  }

  void
  maintain() {
    if (!__atomic_load_n(&maintenance_pending, __ATOMIC_ACQUIRE))
    {
      return;
    }
    wait();
    assign(0, 0, nullptr);
    wait();
  }
};

void *
worker_entry(void *argument) {
  auto &worker = *static_cast<Pool::Worker *>(argument);
  auto &pool = *worker.pool;
  for (;;)
  {
    pthread_mutex_lock(&pool.mutex);
    while (!worker.assigned && !pool.stopping)
    {
      pthread_cond_wait(&worker.changed, &pool.mutex);
    }
    if (!worker.assigned && pool.stopping)
    {
      pthread_mutex_unlock(&pool.mutex);
      return nullptr;
    }
    worker.assigned = false;
    pthread_mutex_unlock(&pool.mutex);
    auto operation = worker.message.operation;
    bool writing = operation == xv::userfs::Operation::Write;
    bool data =
        !worker.maintenance && (writing || operation == xv::userfs::Operation::Read);
    bool independent = data && pool.backend.concurrent_data();
    auto &inode = pool.inodes[worker.message.node % 64];
    if (independent)
    {
      pthread_rwlock_rdlock(&pool.namespace_lock);
      if (writing)
      {
        pthread_rwlock_wrlock(&inode);
      }
      else
      {
        pthread_rwlock_rdlock(&inode);
      }
    }
    else
    {
      pthread_rwlock_wrlock(&pool.namespace_lock);
    }
    int result = 0;
    if (worker.maintenance)
    {
      result = pool.backend.maintain() ? 0 : -1;
    }
    else
    {
      void *bulk = pool.mapping
                       ? static_cast<char *>(pool.mapping) + worker.lane * pool.payload
                       : nullptr;
      dispatch(worker.message, pool.backend, bulk, pool.payload);
    }
    __atomic_store_n(&pool.maintenance_delay, pool.backend.maintenance_delay_ms(),
                     __ATOMIC_RELEASE);
    __atomic_store_n(&pool.maintenance_pending,
                     unsigned(pool.backend.maintenance_pending()), __ATOMIC_RELEASE);
    if (independent)
    {
      pthread_rwlock_unlock(&inode);
    }
    pthread_rwlock_unlock(&pool.namespace_lock);
    if (!worker.maintenance && write(pool.channels[worker.lane], &worker.message,
                                     sizeof(worker.message)) != sizeof(worker.message))
    {
      result = -1;
    }
    pthread_mutex_lock(&pool.mutex);
    worker.result = result;
    worker.complete = true;
    pthread_mutex_unlock(&pool.mutex);
    char token = worker.index;
    if (write(pool.completion[1], &token, 1) != 1)
    {
      fail("completion notification");
    }
  }
}
} // namespace

int
serve_thread_pool(int descriptor, Backend &backend) {
  auto *memory = malloc(sizeof(Pool));
  if (!memory)
  {
    return -1;
  }
  auto *pool = new (memory) Pool(backend);

  struct Cleanup
  {
    Pool *pool;

    ~Cleanup() {
      pool->~Pool();
      free(pool);
    }
  } cleanup{pool};

  pool->owned = backend.transport_ >= 0;
  pool->lanes = pool->owned ? backend.transport_lanes_ : 1;
  pool->channels[0] = pool->owned ? backend.transport_ : descriptor;
  for (unsigned i = 1; i < pool->lanes; ++i)
  {
    pool->channels[i] = backend.extra_transports_[i - 1];
  }
  pool->mapping = backend.bulk_;
  pool->payload = pool->owned ? backend.bulk_bytes_ : DataSize;
  pool->mapping_bytes = backend.bulk_mapping_bytes_;
  backend.transport_ = -1;
  backend.bulk_ = nullptr;
  if (pipe(pool->completion) < 0)
  {
    return -1;
  }
  for (unsigned i = 0; i < backend.worker_threads(); ++i)
  {
    auto &worker = pool->workers[i];
    worker.pool = pool;
    worker.index = i;
    if (pthread_create(&worker.thread, nullptr, worker_entry, &worker))
    {
      return -1;
    }
    ++pool->count;
  }
  unsigned cursor = 0;
  // One receive buffer belongs to the dispatcher; each worker owns its message.
  auto *message =
      static_cast<xv::userfs::Message *>(malloc(sizeof(xv::userfs::Message)));
  if (!message)
  {
    return -1;
  }

  struct MessageCleanup
  {
    void *memory;

    ~MessageCleanup() {
      free(memory);
    }
  } messages{message};

  for (;;)
  {
    pool->healthy();
    pool->maintain();
    unsigned available = pool->count;
    for (unsigned i = 0; i < pool->count; ++i)
    {
      if (!pool->workers[i].busy)
      {
        available = i;
        break;
      }
    }
    pollfd pending[Lanes + 1]{};
    pending[0] = {pool->completion[0], POLLIN, 0};
    for (unsigned lane = 0; lane < pool->lanes; ++lane)
    {
      pending[lane + 1] = {
          available == pool->count || pool->lane_busy[lane] ? -1 : pool->channels[lane],
          POLLIN, 0};
    }
    int delay = pool->busy() ? 100 : -1;
    if (__atomic_load_n(&pool->maintenance_pending, __ATOMIC_ACQUIRE))
    {
      unsigned requested = __atomic_load_n(&pool->maintenance_delay, __ATOMIC_ACQUIRE);
      delay = requested > 100 ? 100 : int(requested);
    }
    int ready = poll(pending, pool->lanes + 1, delay);
    if (ready < 0 && (errno == EINTR || errno == ENOMEM || errno == EAGAIN))
    {
      if (errno != EINTR)
      {
        poll(nullptr, 0, 1);
      }
      continue;
    }
    if (ready < 0)
    {
      fail("transport poll");
    }
    if (pending[0].revents)
    {
      if (!pool->complete())
      {
        fail("transport completion");
      }
      continue;
    }
    unsigned lane = pool->lanes;
    for (unsigned n = 0; n < pool->lanes; ++n)
    {
      unsigned candidate = (cursor + n) % pool->lanes;
      if (pending[candidate + 1].revents)
      {
        lane = candidate;
        break;
      }
    }
    if (lane == pool->lanes)
    {
      continue;
    }
    cursor = (lane + 1) % pool->lanes;
    int received = read(pool->channels[lane], message, sizeof(*message));
    if (!received)
    {
      pool->wait();
      while (__atomic_load_n(&pool->maintenance_pending, __ATOMIC_ACQUIRE))
      {
        pool->maintain();
        unsigned delay = __atomic_load_n(&pool->maintenance_delay, __ATOMIC_ACQUIRE);
        if (delay)
        {
          poll(nullptr, 0, delay > 1000 ? 1000 : delay);
        }
      }
      return 0;
    }
    if (received != sizeof(*message))
    {
      fail("transport receive");
    }
    auto operation = message->operation;
    bool data = operation == xv::userfs::Operation::Read ||
                operation == xv::userfs::Operation::Write;
    if (!data)
    {
      pool->wait(); // Namespace, Forget, snapshots and transactions are barriers.
    }
    pool->assign(available, lane, message);
    if (!data)
    {
      pool->wait();
    }
  }
}
} // namespace userfs
