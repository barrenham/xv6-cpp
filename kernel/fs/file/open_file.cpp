module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"

module xv.fs;
import :platform;
import :operations;

namespace xv::fs
{
namespace
{
constinit InodeOperations inode_operations;
constinit DeviceOperations device_operations;
constinit PipeOperations pipe_operations;
} // namespace

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
      file.type = File::Kind::None;
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
  File detached;
  lock_.acquire();
  if (file->ref < 1)
  {
    detail::fail("file: release dead reference");
  }
  if (--file->ref > 0)
  {
    lock_.release();
    return;
  }
  detached.type = file->type;
  detached.pipe = file->pipe;
  detached.node = file->node;
  detached.writable = file->writable;
  detached.major = file->major;
  file->type = File::Kind::None;
  lock_.release();
  // Final release can sleep and must happen outside the table's spinlock.
  switch (detached.type)
  {
  case File::Kind::Pipe:
    pipe_operations.close(detached);
    break;
  case File::Kind::Inode:
    inode_operations.close(detached);
    break;
  case File::Kind::Device:
    device_operations.close(detached);
    break;
  case File::Kind::None:
    break;
  }
}

void OpenFileTable::bind_inode(File &file, Inode &node, bool readable, bool writable)
{
  file.type = node.kind() == T_DEVICE ? File::Kind::Device : File::Kind::Inode;
  file.major = node.device_major();
  file.node = &node;
  file.offset = 0;
  file.readable = readable;
  file.writable = writable;
}

void OpenFileTable::bind_pipe(File &file, Pipe &pipe, bool writable)
{
  file.type = File::Kind::Pipe;
  file.pipe = &pipe;
  file.readable = !writable;
  file.writable = writable;
}

int OpenFileTable::read(File *file, unsigned long address, int count)
{
  if (!file->readable || count < 0)
  {
    return -1;
  }
  FileOperations *operations = nullptr;
  switch (file->type)
  {
  case File::Kind::Inode:
    operations = &inode_operations;
    break;
  case File::Kind::Device:
    operations = &device_operations;
    break;
  case File::Kind::Pipe:
    operations = &pipe_operations;
    break;
  default:
    detail::fail("file: invalid read kind");
  }
  return operations->read(*file, address, count);
}

int OpenFileTable::write(File *file, unsigned long address, int count)
{
  if (!file->writable || count < 0)
  {
    return -1;
  }
  FileOperations *operations = nullptr;
  switch (file->type)
  {
  case File::Kind::Inode:
    operations = &inode_operations;
    break;
  case File::Kind::Device:
    operations = &device_operations;
    break;
  case File::Kind::Pipe:
    operations = &pipe_operations;
    break;
  default:
    detail::fail("file: invalid write kind");
  }
  return operations->write(*file, address, count);
}

int OpenFileTable::stat(File *file, unsigned long address)
{
  if (file->type == File::Kind::Inode)
  {
    return inode_operations.stat(*file, address);
  }
  if (file->type == File::Kind::Device)
  {
    return device_operations.stat(*file, address);
  }
  return -1;
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
