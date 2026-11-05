module;
#include "kernel/net/socket/socket.h"
#include "kernel/io/posix_errno.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/stat.h"
#include "kernel/support/placement_new.hpp"
module xv.ipc;
import xv.fs;
import xv.process;
import xv.memory;
import xv.runtime;

namespace xv::ipc
{
namespace
{
constexpr unsigned Backlog = 16, PathBytes = sizeof(sockaddr_un::sun_path);
class NamedSocket;
constinit xv::sync::SpinLock names_lock;
NamedSocket *names = nullptr, *retired = nullptr;
unsigned pending_retirements = 0;
unsigned initialized = 0;

void
initialize() {
  unsigned expected = 0;
  if (__atomic_compare_exchange_n(&initialized, &expected, 1U, false, __ATOMIC_ACQ_REL,
                                  __ATOMIC_ACQUIRE))
  {
    names_lock.init("Unix pathname sockets");
    __atomic_store_n(&initialized, 2U, __ATOMIC_RELEASE);
  }
  while (__atomic_load_n(&initialized, __ATOMIC_ACQUIRE) != 2)
  {
  }
}

class NamedSocket final : public xv::io::IoObject, public LocalSocket
{
public:

  static void reclaim_retired();

  explicit NamedSocket(int type) : type_(type) {
    state_.init("Unix listener");
    operation_.init_single_waiter("Unix socket setup");
  }

  xv::io::Capability *
  capability(xv::io::CapabilityId id) override {
    return id == LocalSocket::Id ? static_cast<LocalSocket *>(this) : nullptr;
  }

  void
  attach_file(void *file) override {
    __atomic_store_n(&file_, static_cast<xv::fs::File *>(file), __ATOMIC_RELEASE);
    if (auto *socket = transport())
    {
      socket->attach_file(file);
    }
  }

  int
  type() const override {
    return type_;
  }

  bool
  connected() const override {
    return object() != nullptr;
  }

  int
  address(bool peer, char *path) override {
    xv::sync::ScopedLock guard(state_);
    if (peer && !connected())
    {
      return -ENOTCONN;
    }
    xv::runtime::copy_fixed(path, peer ? peer_path_ : local_path_, PathBytes);
    return 0;
  }

  int bind_path(const char *path) override;
  int connect_path(const char *path, bool nonblocking) override;
  int listen(int backlog) override;
  xv::io::IoObject *accept(bool nonblocking, int &error) override;

  int
  send(const iovec *vectors, unsigned count, bool user, unsigned flags,
       const Rights &rights, unsigned long deadline) override {
    auto *socket = transport();
    return socket ? socket->send(vectors, count, user, flags, rights, deadline)
                  : -ENOTCONN;
  }

  int
  receive(const iovec *vectors, unsigned count, bool user, unsigned flags,
          Rights &rights, unsigned &output, unsigned long deadline) override {
    auto *socket = transport();
    return socket
               ? socket->receive(vectors, count, user, flags, rights, output, deadline)
               : -ENOTCONN;
  }

  int
  shutdown(int how) override {
    auto *socket = transport();
    return socket ? socket->shutdown(how) : -ENOTCONN;
  }

  void
  cancel() override {
    if (auto *socket = transport())
    {
      socket->cancel();
    }
  }

  int
  read(xv::io::Buffer buffer, int count) override {
    auto *endpoint = object();
    return endpoint ? endpoint->read(buffer, count) : -ENOTCONN;
  }

  int
  write(xv::io::Buffer buffer, int count) override {
    auto *endpoint = object();
    return endpoint ? endpoint->write(buffer, count) : -ENOTCONN;
  }

  unsigned
  poll(xv::sync::PollTable *table) override {
    if (auto *endpoint = object())
    {
      return endpoint->poll(table);
    }
    xv::sync::ScopedLock guard(state_);
    if (table)
    {
      table->watch(this);
    }
    return closed_ ? POLLHUP : listening_ ? (count_ ? POLLIN : 0) : POLLHUP | POLLOUT;
  }

  int
  stat(xv::io::Metadata &value) override {
    value = {};
    value.type = T_SOCKET;
    value.mode = 0777;
    value.nlink = 1;
    return 0;
  }

  void close() override;

private:

  xv::io::IoObject *
  object() const {
    return __atomic_load_n(&inner_, __ATOMIC_ACQUIRE);
  }

  LocalSocket *
  transport() const {
    return xv::io::query<LocalSocket>(object());
  }

  int wait_locked();
  int enqueue(NamedSocket *, bool nonblocking);
  static NamedSocket *find(xv::fs::Inode *);
  static void release(NamedSocket *);

  NamedSocket *next_ = nullptr;
  unsigned references_ = 1; // Owner plus connect() leases; registry is borrowed.
  xv::fs::Inode *bound_ = nullptr;
  xv::fs::File *file_ = nullptr;
  xv::io::IoObject *inner_ = nullptr;
  xv::sync::SpinLock state_;
  xv::sync::SleepLock operation_;
  NamedSocket *pending_[Backlog]{};
  unsigned head_ = 0, count_ = 0, backlog_ = 1;
  bool listening_ = false, closed_ = false;
  const int type_;
  char local_path_[PathBytes]{}, peer_path_[PathBytes]{};
};

static_assert(sizeof(NamedSocket) <= 4096);

int
NamedSocket::wait_locked() {
  if (xv::process::current()->is_killed())
  {
    return -EINTR;
  }
  bool failed;
  {
    auto waiter = xv::process::make_poll_wait();
    waiter.watch(this);
    failed = waiter.failed();
    state_.release();
    if (!failed)
    {
      waiter.sleep();
    }
  }
  state_.acquire();
  return failed ? -ENOMEM : xv::process::current()->is_killed() ? -EINTR : 0;
}

int
NamedSocket::bind_path(const char *path) {
  xv::sync::ScopedLock operation(operation_);
  if (bound_ || connected() || listening_)
  {
    return -EINVAL;
  }
  auto &filesystem = xv::fs::filesystem();
  int error = 0;
  auto *existing = filesystem.lookup_checked(path, error, false);
  if (existing)
  {
    filesystem.release(existing);
    return -EADDRINUSE;
  }
  if (error != -ENOENT)
  {
    return error;
  }
  char component[256];
  auto *parent =
      filesystem.lookup_parent_checked(path, component, sizeof(component), error);
  if (!parent)
  {
    return error;
  }
  filesystem.lock(parent);
  bool allowed = filesystem.allowed(parent, 3);
  bool readonly = parent->owner().read_only();
  filesystem.unlock_release(parent);
  if (readonly)
  {
    return -EROFS;
  }
  if (!allowed)
  {
    return -EACCES;
  }
  auto *node = filesystem.create(const_cast<char *>(path), T_SOCKET, 0, 0, false);
  if (!node)
  {
    // A competing bind may have won while the backend serialized creation.
    existing = filesystem.lookup_checked(path, error, false);
    if (existing)
    {
      filesystem.release(existing);
      return -EADDRINUSE;
    }
    return -EIO;
  }
  filesystem.unlock(node);
  names_lock.acquire();
  state_.acquire();
  bound_ = node;
  xv::runtime::copy_fixed(local_path_, path, sizeof(local_path_));
  next_ = names;
  names = this;
  state_.release();
  names_lock.release();
  return 0;
}

NamedSocket *
NamedSocket::find(xv::fs::Inode *node) {
  xv::sync::ScopedLock guard(names_lock);
  for (auto *socket = names; socket; socket = socket->next_)
  {
    auto *bound = socket->bound_;
    if (&bound->owner() == &node->owner() && bound->number() == node->number())
    {
      ++socket->references_;
      return socket;
    }
  }
  return nullptr;
}

void
NamedSocket::release(NamedSocket *socket) {
  names_lock.acquire();
  bool last = --socket->references_ == 0;
  names_lock.release();
  if (last)
  {
    auto *node = socket->bound_;
    if (node && !xv::fs::can_reclaim_file_objects())
    {
      {
        xv::sync::ScopedLock guard(names_lock);
        socket->next_ = retired;
        retired = socket;
        __atomic_store_n(&pending_retirements, 1U, __ATOMIC_RELEASE);
      }
      request_socket_reclamation();
      return;
    }
    socket->~NamedSocket();
    xv::memory::allocator().deallocate(socket);
    if (node)
    {
      xv::fs::filesystem().release(node);
    }
  }
}

void
NamedSocket::reclaim_retired() {
  if (!__atomic_load_n(&pending_retirements, __ATOMIC_ACQUIRE) ||
      !xv::fs::can_reclaim_file_objects())
  {
    return;
  }
  names_lock.acquire();
  auto *list = retired;
  retired = nullptr;
  __atomic_store_n(&pending_retirements, 0U, __ATOMIC_RELEASE);
  names_lock.release();
  while (list)
  {
    auto *next = list->next_;
    auto *node = list->bound_;
    list->~NamedSocket();
    xv::memory::allocator().deallocate(list);
    xv::fs::filesystem().release(node);
    list = next;
  }
}

int
NamedSocket::enqueue(NamedSocket *child, bool nonblocking) {
  state_.acquire();
  int error = 0;
  for (;;)
  {
    if (closed_ || !listening_)
    {
      error = -ECONNREFUSED;
      break;
    }
    if (count_ < backlog_)
    {
      break;
    }
    error = nonblocking ? -EAGAIN : wait_locked();
    if (error)
    {
      break;
    }
  }
  if (!error)
  {
    xv::runtime::copy_fixed(child->local_path_, local_path_, PathBytes);
    pending_[(head_ + count_) % Backlog] = child;
    ++count_;
    xv::process::wakeup(this);
  }
  state_.release();
  return error;
}

int
NamedSocket::connect_path(const char *path, bool nonblocking) {
  xv::sync::ScopedLock operation(operation_);
  if (connected())
  {
    return -EISCONN;
  }
  if (listening_)
  {
    return -EINVAL;
  }
  auto &filesystem = xv::fs::filesystem();
  int error = 0;
  auto *node = filesystem.lookup_checked(path, error);
  if (!node)
  {
    return error;
  }
  filesystem.lock(node);
  error = node->kind() != T_SOCKET       ? -ECONNREFUSED
          : !filesystem.allowed(node, 2) ? -EACCES
                                         : 0;
  filesystem.unlock(node);
  auto *listener = error ? nullptr : find(node);
  filesystem.release(node);
  if (error)
  {
    return error;
  }
  if (!listener)
  {
    return -ECONNREFUSED;
  }
  if (listener->type_ != type_)
  {
    release(listener);
    return -EPROTOTYPE;
  }
  auto *child = static_cast<NamedSocket *>(local_socket(type_));
  xv::io::IoObject *endpoints[2]{};
  if (!child || !socket_pair(type_, endpoints))
  {
    if (child)
    {
      child->close();
    }
    release(listener);
    return -ENOMEM;
  }
  child->inner_ = endpoints[1];
  xv::runtime::copy_fixed(child->peer_path_, local_path_, PathBytes);
  // Pending transports are reachable through the listener File. The existing
  // SCM_RIGHTS collector follows that edge, including across other listeners;
  // a pending connection is not an unconditional GC root.
  child->attach_file(__atomic_load_n(&listener->file_, __ATOMIC_ACQUIRE));
  xv::io::query<LocalSocket>(endpoints[0])
      ->attach_file(__atomic_load_n(&file_, __ATOMIC_ACQUIRE));
  error = listener->enqueue(child, nonblocking);
  if (!error)
  {
    state_.acquire();
    xv::runtime::copy_fixed(peer_path_, listener->local_path_, PathBytes);
    __atomic_store_n(&inner_, endpoints[0], __ATOMIC_RELEASE);
    state_.release();
    xv::process::wakeup(this);
  }
  else
  {
    child->close();
    endpoints[0]->close();
  }
  release(listener);
  return error;
}

int
NamedSocket::listen(int backlog) {
  xv::sync::ScopedLock operation(operation_);
  if (!bound_ || connected())
  {
    return -EINVAL;
  }
  xv::sync::ScopedLock guard(state_);
  backlog_ = backlog <= 0                  ? 1
             : unsigned(backlog) > Backlog ? Backlog
                                           : unsigned(backlog);
  listening_ = true;
  xv::process::wakeup(this);
  return 0;
}

xv::io::IoObject *
NamedSocket::accept(bool nonblocking, int &error) {
  state_.acquire();
  error = 0;
  NamedSocket *child = nullptr;
  for (;;)
  {
    if (closed_ || !listening_)
    {
      error = -EINVAL;
      break;
    }
    if (count_)
    {
      child = pending_[head_];
      pending_[head_] = nullptr;
      head_ = (head_ + 1) % Backlog;
      --count_;
      xv::process::wakeup(this);
      break;
    }
    error = nonblocking ? -EAGAIN : wait_locked();
    if (error)
    {
      break;
    }
  }
  state_.release();
  return child;
}

void
NamedSocket::close() {
  NamedSocket *pending[Backlog]{};
  names_lock.acquire();
  state_.acquire();
  closed_ = true;
  auto **link = &names;
  while (*link && *link != this)
  {
    link = &(*link)->next_;
  }
  if (*link)
  {
    *link = next_;
  }
  for (unsigned i = 0; i < Backlog; ++i)
  {
    pending[i] = pending_[i];
    pending_[i] = nullptr;
  }
  count_ = 0;
  xv::process::wakeup(this);
  state_.release();
  names_lock.release();
  for (auto *child : pending)
  {
    if (child)
    {
      child->close();
    }
  }
  if (auto *endpoint = object())
  {
    endpoint->close();
  }
  release(this);
}
} // namespace

bool
reclaim_named_sockets() {
  NamedSocket::reclaim_retired();
  return __atomic_load_n(&pending_retirements, __ATOMIC_ACQUIRE) != 0;
}

xv::io::IoObject *
local_socket(int type) {
  if (type != SOCK_STREAM && type != SOCK_SEQPACKET)
  {
    return nullptr;
  }
  initialize();
  void *memory = xv::memory::allocator().allocate();
  return memory ? new (memory) NamedSocket(type) : nullptr;
}
} // namespace xv::ipc
