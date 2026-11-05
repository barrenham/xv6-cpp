module;
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/io/posix_errno.h"
#include "kernel/io/mqueue.h"
#include "kernel/support/placement_new.hpp"
module xv.ipc;
import xv.arch;
import xv.memory;
import xv.process;
import xv.runtime;

namespace xv::ipc
{
namespace
{
constexpr unsigned Names = 32, MaximumMessages = 64, MessageBytes = 3072;

struct Message
{
  Message *next = nullptr;
  unsigned size = 0, priority = 0;
  char data[MessageBytes];
};

struct Queue
{
  xv::sync::SpinLock lock;
  Message *head = nullptr;
  unsigned maximum = 16, size = MessageBytes, count = 0;
  unsigned references = 0, uid = 0, gid = 0, mode = 0;
  char name[256]{};
};

static_assert(sizeof(Queue) <= 4096 && sizeof(Message) <= 4096);
constinit xv::sync::SpinLock names_lock;
Queue *names[Names]{};
unsigned initialized = 0;

void
initialize() {
  unsigned expected = 0;
  if (__atomic_compare_exchange_n(&initialized, &expected, 1U, false, __ATOMIC_ACQ_REL,
                                  __ATOMIC_ACQUIRE))
  {
    names_lock.init("message queue names");
    __atomic_store_n(&initialized, 2U, __ATOMIC_RELEASE);
  }
  while (__atomic_load_n(&initialized, __ATOMIC_ACQUIRE) != 2)
  {
  }
}

bool
valid_name(const char *name) {
  if (name[0] != '/' || !name[1])
  {
    return false;
  }
  for (unsigned i = 1; name[i]; ++i)
  {
    if (name[i] == '/')
    {
      return false;
    }
  }
  return true;
}

void
release(Queue *queue) {
  names_lock.acquire();
  bool last = --queue->references == 0;
  names_lock.release();
  if (!last)
  {
    return;
  }
  while (queue->head)
  {
    auto *message = queue->head;
    queue->head = message->next;
    xv::memory::allocator().deallocate(message);
  }
  queue->~Queue();
  xv::memory::allocator().deallocate(queue);
}

// The caller holds the condition lock. Register before dropping it so a
// receiver freeing space (or a sender posting data) cannot be missed.
int
wait(Queue &queue, unsigned long deadline) {
  if (xv::process::current()->is_killed())
  {
    return -EINTR;
  }
  if (deadline && xv::arch::r_time() / 10000 >= deadline)
  {
    return -ETIMEDOUT;
  }
  bool failed;
  {
    auto waiter = xv::process::make_poll_wait();
    waiter.watch(&queue);
    if (deadline)
    {
      waiter.set_deadline(deadline);
    }
    failed = waiter.failed();
    queue.lock.release();
    if (!failed)
    {
      waiter.sleep();
    }
  }
  queue.lock.acquire();
  return failed ? -ENOMEM : xv::process::current()->is_killed() ? -EINTR : 0;
}

class QueueView final : public xv::io::IoObject, public MessageQueue
{
public:

  QueueView(Queue &queue, unsigned access) : queue_(queue), access_(access) {
  }

  xv::io::Capability *
  capability(xv::io::CapabilityId id) override {
    return id == MessageQueue::Id ? static_cast<MessageQueue *>(this) : nullptr;
  }

  int
  send(unsigned long address, unsigned long bytes, unsigned priority, bool nonblocking,
       unsigned long deadline) override {
    if (access_ == O_RDONLY)
    {
      return -EBADF;
    }
    if (bytes > queue_.size)
    {
      return -EMSGSIZE;
    }
    if (priority >= MQ_PRIO_MAX)
    {
      return -EINVAL;
    }
    auto *storage = xv::memory::allocator().allocate();
    if (!storage)
    {
      return -ENOMEM;
    }
    auto *message = new (storage) Message;
    message->size = bytes;
    message->priority = priority;
    if (xv::process::current()->memory().copy_in(message->data, address, bytes) < 0)
    {
      xv::memory::allocator().deallocate(message);
      return -EFAULT;
    }
    queue_.lock.acquire();
    int error = 0;
    while (queue_.count == queue_.maximum)
    {
      error = nonblocking ? -EAGAIN : wait(queue_, deadline);
      if (error)
      {
        break;
      }
    }
    if (!error)
    {
      auto **at = &queue_.head;
      // Insert after equal priorities to preserve FIFO for each priority.
      while (*at && (*at)->priority >= priority)
      {
        at = &(*at)->next;
      }
      message->next = *at;
      *at = message;
      ++queue_.count;
      xv::process::wakeup(&queue_);
    }
    queue_.lock.release();
    if (error)
    {
      xv::memory::allocator().deallocate(message);
    }
    return error;
  }

  int
  receive(unsigned long address, unsigned long capacity, unsigned long priority,
          bool nonblocking, unsigned long deadline) override {
    if (access_ == O_WRONLY)
    {
      return -EBADF;
    }
    if (capacity < queue_.size)
    {
      return -EMSGSIZE;
    }
    queue_.lock.acquire();
    int error = 0;
    while (!queue_.head)
    {
      error = nonblocking ? -EAGAIN : wait(queue_, deadline);
      if (error)
      {
        break;
      }
    }
    Message *message = nullptr;
    if (!error)
    {
      message = queue_.head;
      auto &memory = xv::process::current()->memory();
      if (memory.copy_out(address, message->data, message->size) < 0 ||
          (priority &&
           memory.copy_out(priority, &message->priority, sizeof(unsigned)) < 0))
      {
        error = -EFAULT;
        message = nullptr; // Invalid output must not consume the message.
      }
      else
      {
        queue_.head = message->next;
        --queue_.count;
        xv::process::wakeup(&queue_);
      }
    }
    queue_.lock.release();
    if (message)
    {
      int size = message->size;
      xv::memory::allocator().deallocate(message);
      return size;
    }
    return error;
  }

  void
  attributes(mq_attr &value) override {
    xv::sync::ScopedLock guard(queue_.lock);
    value = {};
    value.mq_maxmsg = queue_.maximum;
    value.mq_msgsize = queue_.size;
    value.mq_curmsgs = queue_.count;
  }

  unsigned
  poll(xv::sync::PollTable *table) override {
    xv::sync::ScopedLock guard(queue_.lock);
    if (table)
    {
      table->watch(&queue_);
    }
    return (access_ != O_WRONLY && queue_.count ? POLLIN : 0) |
           (access_ != O_RDONLY && queue_.count < queue_.maximum ? POLLOUT : 0);
  }

  int
  read(xv::io::Buffer, int) override {
    return -EINVAL;
  }

  int
  write(xv::io::Buffer, int) override {
    return -EINVAL;
  }

  void
  close() override {
    auto *queue = &queue_;
    this->~QueueView();
    xv::memory::allocator().deallocate(this);
    release(queue);
  }

private:

  Queue &queue_;
  unsigned access_;
};

static_assert(sizeof(QueueView) <= 4096);
} // namespace

xv::io::IoObject *
queue_open(const char *name, unsigned flags, unsigned mode, const mq_attr *attributes,
           int &error) {
  error = -EINVAL;
  unsigned access = flags & O_ACCMODE;
  if (!valid_name(name) || access > O_RDWR ||
      (flags & ~(O_ACCMODE | O_CREAT | O_EXCL | O_NONBLOCK | O_CLOEXEC)))
  {
    return nullptr;
  }
  initialize();
  auto credentials = xv::process::current()->credentials().snapshot();
  names_lock.acquire();
  Queue *queue = nullptr;
  unsigned slot = Names;
  for (unsigned i = 0; i < Names; ++i)
  {
    if (names[i] && !xv::runtime::compare_text(names[i]->name, name, 256))
    {
      queue = names[i];
    }
    if (!names[i] && slot == Names)
    {
      slot = i;
    }
  }
  if (queue)
  {
    if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
    {
      error = -EEXIST;
    }
    else
    {
      bool group = queue->gid == credentials.effective_gid;
      for (unsigned i = 0; i < credentials.group_count; ++i)
      {
        group |= queue->gid == credentials.groups[i];
      }
      unsigned shift = queue->uid == credentials.effective_uid ? 6 : group ? 3 : 0;
      unsigned requested = access == O_RDONLY ? 4 : access == O_WRONLY ? 2 : 6;
      error =
          credentials.effective_uid && ((queue->mode >> shift) & requested) != requested
              ? -EACCES
              : 0;
      if (!error)
      {
        ++queue->references;
      }
    }
    if (error)
    {
      queue = nullptr;
    }
  }
  else if (!(flags & O_CREAT))
  {
    error = -ENOENT;
  }
  else if (slot == Names)
  {
    error = -ENOSPC;
  }
  else if (attributes &&
           (attributes->mq_maxmsg <= 0 || attributes->mq_maxmsg > MaximumMessages ||
            attributes->mq_msgsize <= 0 || attributes->mq_msgsize > MessageBytes))
  {
    error = -EINVAL;
  }
  else
  {
    auto *memory = xv::memory::allocator().allocate();
    if (!memory)
    {
      error = -ENOMEM;
    }
    else
    {
      queue = new (memory) Queue;
      queue->lock.init("POSIX message queue");
      queue->references = 2; // Name plus this open description.
      queue->uid = credentials.effective_uid;
      queue->gid = credentials.effective_gid;
      queue->mode = mode & 0777 & ~credentials.mask;
      if (attributes)
      {
        queue->maximum = attributes->mq_maxmsg;
        queue->size = attributes->mq_msgsize;
      }
      xv::runtime::copy_fixed(queue->name, name, sizeof(queue->name));
      names[slot] = queue;
      error = 0;
    }
  }
  names_lock.release();
  if (!queue)
  {
    return nullptr;
  }
  auto *memory = xv::memory::allocator().allocate();
  if (!memory)
  {
    release(queue);
    error = -ENOMEM;
    return nullptr;
  }
  return new (memory) QueueView(*queue, access);
}

int
queue_unlink(const char *name) {
  if (!valid_name(name))
  {
    return -EINVAL;
  }
  initialize();
  auto uid = xv::process::current()->credentials().snapshot().effective_uid;
  names_lock.acquire();
  Queue *queue = nullptr;
  for (unsigned i = 0; i < Names; ++i)
  {
    if (names[i] && !xv::runtime::compare_text(names[i]->name, name, 256))
    {
      queue = names[i];
      if (uid && uid != queue->uid)
      {
        names_lock.release();
        return -EACCES;
      }
      names[i] = nullptr;
      break;
    }
  }
  names_lock.release();
  if (!queue)
  {
    return -ENOENT;
  }
  release(queue);
  return 0;
}
} // namespace xv::ipc
