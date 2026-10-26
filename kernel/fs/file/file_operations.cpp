module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/poll.h"
module xv.fs;
import :platform;
import :operations;
namespace xv::fs
{
void InodeBackend::close()
{
  { Transaction transaction; filesystem().release(node_); }
  this->~InodeBackend();
}
int InodeBackend::read(xv::io::Buffer destination, int count)
{
  LockedInode guard(filesystem(), *node_);
  int result = filesystem().read(node_, 1, destination.address, offset_, count);
  if (result > 0) offset_ += result;
  return result;
}
int InodeBackend::write(xv::io::Buffer source, int count)
{
  const int maximum = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * BSIZE;
  int done = 0;
  while (done < count)
  {
    if (detail::killed()) return done ? done : -1;
    int part = count-done;
    if (part > maximum) part = maximum;
    int result;
    {
      Transaction transaction;
      LockedInode guard(filesystem(), *node_);
      result = filesystem().write(node_, 1, source.address+done, offset_, part);
      if (result > 0) offset_ += result;
    }
    if (result != part) break;
    done += result;
  }
  return done == count ? count : -1;
}
int InodeBackend::stat(xv::io::Metadata &value)
{
  LockedInode guard(filesystem(), *node_);
  filesystem().stat(node_, &value);
  return 0;
}
unsigned InodeBackend::poll(xv::sync::PollTable *) { return POLLIN | POLLOUT; }
void DeviceBackend::close()
{
  { Transaction transaction; filesystem().release(node_); }
  this->~DeviceBackend();
}
int DeviceBackend::read(xv::io::Buffer destination, int count)
{
  auto *device = devices().get(major_);
  return device ? device->read({xv::drivers::AddressSpace::User, destination.address, destination.nonblocking}, count) : -1;
}
int DeviceBackend::write(xv::io::Buffer source, int count)
{
  auto *device = devices().get(major_);
  return device ? device->write({xv::drivers::AddressSpace::User, source.address, source.nonblocking}, count) : -1;
}
int DeviceBackend::stat(xv::io::Metadata &value)
{
  LockedInode guard(filesystem(), *node_);
  filesystem().stat(node_, &value);
  return 0;
}
unsigned DeviceBackend::poll(xv::sync::PollTable *table)
{
  auto *device = devices().get(major_);
  return device ? device->poll(table) : POLLERR;
}
int DeviceBackend::ioctl(unsigned request, xv::io::Buffer argument)
{
  auto *device = devices().get(major_);
  return device ? device->ioctl(request, {xv::drivers::AddressSpace::User, argument.address, argument.nonblocking}) : -1;
}
void PipeBackend::close() { pipe_->close(writable_); this->~PipeBackend(); }
int PipeBackend::read(xv::io::Buffer destination, int count)
{ return pipe_->read(destination.address, count, destination.nonblocking); }
int PipeBackend::write(xv::io::Buffer source, int count)
{ return pipe_->write(source.address, count, source.nonblocking); }
unsigned PipeBackend::poll(xv::sync::PollTable *table) { return pipe_->poll(writable_, table); }
void EndpointBackend::close() { endpoint_->close(); this->~EndpointBackend(); }
int EndpointBackend::read(xv::io::Buffer destination, int count)
{ return endpoint_->read({xv::drivers::AddressSpace::User, destination.address, destination.nonblocking}, count); }
int EndpointBackend::write(xv::io::Buffer source, int count)
{ return endpoint_->write({xv::drivers::AddressSpace::User, source.address, source.nonblocking}, count); }
int EndpointBackend::stat(xv::io::Metadata &value)
{ value = {}; value.type = static_cast<short>(xv::io::MetadataKind::Device); value.nlink = 1; return 0; }
unsigned EndpointBackend::poll(xv::sync::PollTable *table) { return endpoint_->poll(table); }
int EndpointBackend::ioctl(unsigned request, xv::io::Buffer argument)
{ return endpoint_->ioctl(request, {xv::drivers::AddressSpace::User, argument.address, argument.nonblocking}); }
}
