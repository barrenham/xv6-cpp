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
  if (command != F_SETFL || (flags & ~(O_ACCMODE | O_NONBLOCK))) return -1;
  auto old = __atomic_load_n(&file->status_flags_, __ATOMIC_ACQUIRE);
  for (;;)
  {
    unsigned value = (old & O_ACCMODE) | (flags & O_NONBLOCK);
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
                              bool writable, bool nonblocking)
{
  file.backend_ = &object;
  file.readable = readable;
  file.writable = writable;
  unsigned mode = readable ? (writable ? O_RDWR : O_RDONLY) : O_WRONLY;
  __atomic_store_n(&file.status_flags_, mode | (nonblocking ? O_NONBLOCK : 0), __ATOMIC_RELEASE);
}

void OpenFileTable::bind_inode(File &file, Inode &node, bool readable, bool writable,
                               bool nonblocking)
{
  static_assert(sizeof(InodeBackend) <= File::BackendBytes && alignof(InodeBackend) <= 16);
  static_assert(sizeof(DeviceBackend) <= File::BackendBytes && alignof(DeviceBackend) <= 16);
  xv::io::IoObject *backend;
  if (node.kind() == T_DEVICE) backend = new (file.backend_storage_) DeviceBackend(node);
  else backend = new (file.backend_storage_) InodeBackend(node);
  configure(file, *backend, readable, writable, nonblocking);
}

void OpenFileTable::bind_endpoint(File &file, xv::drivers::CharacterDevice &endpoint)
{
  static_assert(sizeof(EndpointBackend) <= File::BackendBytes && alignof(EndpointBackend) <= 16);
  auto *backend = new (file.backend_storage_) EndpointBackend(endpoint);
  configure(file, *backend, true, true, false);
}

void OpenFileTable::bind_object(File &file, xv::io::IoObject &object, bool nonblocking)
{
  configure(file, object, true, true, nonblocking);
}

xv::io::IoObject *OpenFileTable::object(File *file) const
{
  return file ? file->backend_ : nullptr;
}

void OpenFileTable::bind_pipe(File &file, Pipe &pipe, bool writable)
{
  static_assert(sizeof(PipeBackend) <= File::BackendBytes && alignof(PipeBackend) <= 16);
  auto *backend = new (file.backend_storage_) PipeBackend(pipe, writable);
  configure(file, *backend, !writable, writable, false);
}

int OpenFileTable::read(File *file, unsigned long address, int count)
{
  if (!file->readable || count < 0 || !file->backend_) return -1;
  return file->backend_->read({address, file->nonblocking()}, count);
}

int OpenFileTable::write(File *file, unsigned long address, int count)
{
  if (!file->writable || count < 0 || !file->backend_) return -1;
  return file->backend_->write({address, file->nonblocking()}, count);
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

void DeviceRegistry::attach(unsigned major, xv::drivers::CharacterDevice &device)
{
  if (major >= NDEV)
  {
    detail::fail("device registry: invalid major");
  }
  devices_[major] = &device;
}

xv::drivers::CharacterDevice *DeviceRegistry::get(int major) const
{
  return major >= 0 && major < NDEV ? devices_[major] : nullptr;
}
} // namespace xv::fs
