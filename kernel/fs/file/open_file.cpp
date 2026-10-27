module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/fcntl.h"
#include "kernel/support/placement_new.hpp"

module xv.fs;
import :platform;
import :operations;
import :pipe;

namespace xv::fs
{
bool File::nonblocking() const
{
  return __atomic_load_n(&status_flags_, __ATOMIC_ACQUIRE) & O_NONBLOCK;
}

int OpenFileTable::fcntl(File *file, int command, unsigned flags)
{
  if (command == F_GETFL)
    return __atomic_load_n(&file->status_flags_, __ATOMIC_ACQUIRE);
  if (command != F_SETFL || (flags & ~(O_ACCMODE | O_NONBLOCK | O_APPEND))) return -1;
  auto old = __atomic_load_n(&file->status_flags_, __ATOMIC_ACQUIRE);
  for (;;)
  {
    unsigned value = (old & O_ACCMODE) | (flags & (O_NONBLOCK | O_APPEND));
    if (__atomic_compare_exchange_n(&file->status_flags_, &old, value, false,
                                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) return 0;
  }
}

void OpenFileTable::initialize()
{
  lock_.init("open files");
}

File *OpenFileTable::allocate()
{
  xv::sync::ScopedLock guard(lock_);
  for (auto &file : files_)
  {
    if (file.ref == 0)
    {
      file.ref = 1;
      file.backend_ = nullptr;
      __atomic_store_n(&file.ready_, false, __ATOMIC_RELAXED);
      file.identity_ = {};
      file.identity_.identity = next_identity_++;
      return &file;
    }
  }
  return nullptr;
}

File *OpenFileTable::retain(File *file)
{
  xv::sync::ScopedLock guard(lock_);
  if (file->ref < 1)
  {
    detail::fail("file: retain dead reference");
  }
  ++file->ref;
  return file;
}

void OpenFileTable::release(File *file)
{
  lock_.acquire();
  if (file->ref < 1) detail::fail("file: release dead reference");
  if (--file->ref > 0) { lock_.release(); return; }
  file->ref = -1;
  auto *backend = file->backend_;
  lock_.release();
  if (backend) backend->close();
  lock_.acquire();
  file->backend_ = nullptr;
  file->ref = 0;
  lock_.release();
}

void OpenFileTable::configure(File &file, xv::io::IoObject &object, bool readable,
                              bool writable, bool nonblocking, FileKind kind, const Inode *node)
{
  file.identity_.kind = kind;
  if (node)
  { file.identity_.type = node->kind(); file.identity_.device = node->device(); file.identity_.inode = node->number(); }
  else file.identity_.type = kind == FileKind::Socket ? T_SOCKET : kind == FileKind::Terminal ? T_DEVICE : 0;
  file.backend_ = &object;
  file.readable = readable;
  file.writable = writable;
  unsigned mode = readable ? (writable ? O_RDWR : O_RDONLY) : O_WRONLY;
  __atomic_store_n(&file.status_flags_, mode | (nonblocking ? O_NONBLOCK : 0), __ATOMIC_RELEASE);
  __atomic_store_n(&file.ready_, true, __ATOMIC_RELEASE);
}

bool OpenFileTable::bind_inode(File &file, Inode &node, bool readable, bool writable,
                               bool nonblocking, bool append)
{
  static_assert(sizeof(InodeBackend) <= File::BackendBytes && alignof(InodeBackend) <= 16);
  static_assert(sizeof(DeviceBackend) <= File::BackendBytes && alignof(DeviceBackend) <= 16);
  xv::io::IoObject *backend;
  if (node.kind() == T_DEVICE)
  {
    auto *session = node.owner().open_device(&node, nonblocking);
    if (!session) return false;
    backend = new (file.backend_storage_) DeviceBackend(node, *session);
  }
  else
  {
    PageBuffer contents;
    bool snapshot = node.owner().snapshot_reads();
    if (snapshot && (writable || !node.owner().snapshot(&node, contents))) return false;
    backend = new (file.backend_storage_) InodeBackend(node, snapshot, contents);
  }
  configure(file, *backend, readable, writable, nonblocking, FileKind::Node, &node);
  if (append) __atomic_fetch_or(&file.status_flags_, O_APPEND, __ATOMIC_RELEASE);
  return true;
}

void OpenFileTable::bind_endpoint(File &file, xv::drivers::CharacterDevice &endpoint)
{
  static_assert(sizeof(EndpointBackend) <= File::BackendBytes && alignof(EndpointBackend) <= 16);
  auto *backend = new (file.backend_storage_) EndpointBackend(endpoint);
  configure(file, *backend, true, true, false, FileKind::Terminal);
}

void OpenFileTable::bind_object(File &file, xv::io::IoObject &object, bool nonblocking)
{
  configure(file, object, true, true, nonblocking, object.capability(xv::io::CapabilityId::Socket) ? FileKind::Socket : FileKind::Unknown);
}

xv::io::IoObject *OpenFileTable::object(File *file) const
{
  return file ? file->backend_ : nullptr;
}

bool OpenFileTable::describe(File *file, FileInfo &result) const
{
  if (!file || !__atomic_load_n(&file->ready_, __ATOMIC_ACQUIRE)) return false;
  result = file->identity_;
  result.flags = __atomic_load_n(&file->status_flags_, __ATOMIC_ACQUIRE);
  return true;
}

void OpenFileTable::bind_pipe(File &file, Pipe &pipe, bool writable)
{
  static_assert(sizeof(PipeBackend) <= File::BackendBytes && alignof(PipeBackend) <= 16);
  auto *backend = new (file.backend_storage_) PipeBackend(pipe, writable);
  configure(file, *backend, !writable, writable, false, FileKind::Pipe);
}

int OpenFileTable::read(File *file, unsigned long address, int count)
{
  if (!file->readable || count < 0 || !file->backend_) return -1;
  return file->backend_->read({address, file->nonblocking()}, count);
}

int OpenFileTable::write(File *file, unsigned long address, int count)
{
  if (!file->writable || count < 0 || !file->backend_) return -1;
  return file->backend_->write({address, file->nonblocking(), bool(__atomic_load_n(&file->status_flags_, __ATOMIC_ACQUIRE) & O_APPEND)}, count);
}

int OpenFileTable::stat(File *file, unsigned long address)
{
  xv::io::Metadata value{};
  if (!file->backend_ || file->backend_->stat(value) < 0) return -1;
  return detail::copy_out(true, address, &value, sizeof(value));
}

int OpenFileTable::ioctl(File *file, unsigned request, unsigned long argument)
{
  auto *control = xv::io::query<xv::io::DeviceControl>(file->backend_);
  return control ? control->ioctl(request, {argument, file->nonblocking()}) : -1;
}

unsigned OpenFileTable::poll(File *file, xv::sync::PollTable *table)
{
  unsigned events = file->backend_ ? file->backend_->poll(table) : POLLNVAL;
  if (!file->readable) events &= ~POLLIN;
  if (!file->writable) events &= ~POLLOUT;
  return events;
}

xv::drivers::CharacterDevice *open_registered_device(Inode *node, bool nonblocking)
{
  auto *device = devices().get(node->device_major(), node->device_minor());
  return device ? device->open(nonblocking) : nullptr;
}

void DeviceRegistry::attach(unsigned major, xv::drivers::CharacterDevice &device)
{
  attach("", major, 0, device);
}

void DeviceRegistry::attach(const char *name, unsigned major, unsigned minor,
                            xv::drivers::CharacterDevice &device)
{
  unsigned length = 0;
  while (name[length] && length < 14) ++length;
  if (sealed_ || major >= NDEV || minor > 32767 || length >= 14 || count_ == NDEV ||
      (length && (detail::compare(name, ".", 14) == 0 || detail::compare(name, "..", 14) == 0)))
    detail::fail("device registry: invalid registration");
  for (unsigned i = 0; i < length; ++i)
    if (name[i] == '/') detail::fail("device registry: invalid name");
  for (unsigned i = 0; i < count_; ++i)
    if ((entries_[i].major == major && entries_[i].minor == minor) ||
        (length && detail::compare(entries_[i].name, name, 14) == 0))
      detail::fail("device registry: duplicate registration");
  auto &record = entries_[count_++];
  detail::copy_fixed(record.name, name, 14);
  record.major = major;
  record.minor = minor;
  record.device = &device;
}

xv::drivers::CharacterDevice *DeviceRegistry::get(int major, int minor) const
{
  if (major < 0 || minor < 0) return nullptr;
  for (unsigned i = 0; i < count_; ++i)
    if (entries_[i].major == static_cast<unsigned>(major) &&
        entries_[i].minor == static_cast<unsigned>(minor)) return entries_[i].device;
  return nullptr;
}

long OpenFileTable::seek(File *file, long offset, int origin)
{ return file && file->backend_ ? file->backend_->seek(offset, origin) : -1; }
int OpenFileTable::resize(File *file, unsigned size)
{ return file && file->writable && file->backend_ ? file->backend_->resize(size) : -1; }
int OpenFileTable::sync(File *file)
{ return file && file->backend_ ? file->backend_->sync() : -1; }
} // namespace xv::fs
