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
void InodeOperations::close(File &file)
{
  Transaction transaction;
  filesystem().release(file.node);
}

int InodeOperations::read(File &file, unsigned long address, int count)
{
  LockedInode guard(filesystem(), *file.node);
  int result = filesystem().read(file.node, 1, address, file.offset, count);
  if (result > 0)
  {
    file.offset += result;
  }
  return result;
}

int InodeOperations::write(File &file, unsigned long address, int count)
{
  const int maximum = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * BSIZE;
  int done = 0;
  while (done < count)
  {
    int part = count - done;
    if (part > maximum)
    {
      part = maximum;
    }
    int result;
    {
      Transaction transaction;
      LockedInode guard(filesystem(), *file.node);
      result = filesystem().write(file.node, 1, address + done, file.offset, part);
      if (result > 0)
      {
        file.offset += result;
      }
    }
    if (result != part)
    {
      break;
    }
    done += result;
  }
  return done == count ? count : -1;
}

int InodeOperations::stat(File &file, unsigned long address)
{
  Stat value;
  {
    LockedInode guard(filesystem(), *file.node);
    filesystem().stat(file.node, &value);
  }
  return detail::copy_out(true, address, &value, sizeof(value));
}

void DeviceOperations::close(File &file)
{
  Transaction transaction;
  filesystem().release(file.node);
}

int DeviceOperations::read(File &file, unsigned long address, int count)
{
  auto *device = devices().get(file.major);
  if (!device)
  {
    return -1;
  }
  return device->read({xv::drivers::AddressSpace::User, address}, count);
}

int DeviceOperations::write(File &file, unsigned long address, int count)
{
  auto *device = devices().get(file.major);
  if (!device)
  {
    return -1;
  }
  return device->write({xv::drivers::AddressSpace::User, address}, count);
}

int DeviceOperations::stat(File &file, unsigned long address)
{
  Stat value;
  {
    LockedInode guard(filesystem(), *file.node);
    filesystem().stat(file.node, &value);
  }
  return detail::copy_out(true, address, &value, sizeof(value));
}

void PipeOperations::close(File &file)
{
  file.pipe->close(file.writable);
}

int PipeOperations::read(File &file, unsigned long address, int count)
{
  return file.pipe->read(address, count);
}

int PipeOperations::write(File &file, unsigned long address, int count)
{
  return file.pipe->write(address, count);
}
} // namespace xv::fs
