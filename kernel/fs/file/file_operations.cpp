module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
module xv.fs;
import :inode_backend_flow;
import :platform;
import :operations;

namespace xv::fs
{
void InodeBackend::close()
{
  {
    Transaction transaction(node_->owner().persistent());
    filesystem().release(node_);
  }
  this->~InodeBackend();
}

int InodeBackend::read(xv::io::Buffer destination, int count)
{
  LockedInode guard(node_->owner(), *node_);
  ReadFlow::Context context{*this, destination, count};
  (void)ReadFlow::Graph::run<0>(context);
  return context.result;
}

int InodeBackend::write(xv::io::Buffer source, int count)
{
  WriteFlow::Context context{{*this}, source, count};
  (void)WriteFlow::Graph::run<0>(context);
  return context.done ? context.done : (count == 0 ? 0 : -1);
}

long InodeBackend::seek(long offset, int origin)
{
  LockedInode guard(node_->owner(), *node_);
  SeekFlow::Context context{*this, offset, origin};
  (void)SeekFlow::Graph::run<0>(context);
  return context.result;
}

int InodeBackend::resize(unsigned size)
{
  Transaction transaction(node_->owner().persistent());
  LockedInode guard(node_->owner(), *node_);
  ResizeFlow::Context context{{*this}, size};
  (void)ResizeFlow::Graph::run<0>(context);
  return context.result;
}

int InodeBackend::sync()
{
  SyncFlow::Context context{*this};
  (void)SyncFlow::Graph::run<0>(context);
  return context.result;
}

int InodeBackend::stat(xv::io::Metadata &value)
{
  LockedInode guard(node_->owner(), *node_);
  filesystem().stat(node_, &value);
  if (snapshot_)
  {
    value.size = contents_.size();
  }
  return 0;
}

unsigned InodeBackend::poll(xv::sync::PollTable *)
{
  return POLLIN | POLLOUT;
}

void DeviceBackend::close()
{
  session_->close();
  {
    Transaction transaction(node_->owner().persistent());
    filesystem().release(node_);
  }
  this->~DeviceBackend();
}

int DeviceBackend::read(xv::io::Buffer destination, int count)
{
  auto *device = session_;
  return device ? device->read({xv::drivers::AddressSpace::User, destination.address,
                                destination.nonblocking},
                               count)
                : -1;
}

int DeviceBackend::write(xv::io::Buffer source, int count)
{
  auto *device = session_;
  return device ? device->write({xv::drivers::AddressSpace::User, source.address,
                                 source.nonblocking},
                                count)
                : -1;
}

int DeviceBackend::stat(xv::io::Metadata &value)
{
  LockedInode guard(node_->owner(), *node_);
  filesystem().stat(node_, &value);
  return 0;
}

unsigned DeviceBackend::poll(xv::sync::PollTable *table)
{
  auto *device = session_;
  return device ? device->poll(table) : POLLERR;
}

int DeviceBackend::ioctl(unsigned request, xv::io::Buffer argument)
{
  auto *device = session_;
  return device ? device->ioctl(request, {xv::drivers::AddressSpace::User,
                                          argument.address, argument.nonblocking})
                : -1;
}

void PipeBackend::close()
{
  pipe_->close(writable_);
  this->~PipeBackend();
}

int PipeBackend::read(xv::io::Buffer destination, int count)
{
  return pipe_->read(destination.address, count, destination.nonblocking);
}

int PipeBackend::write(xv::io::Buffer source, int count)
{
  return pipe_->write(source.address, count, source.nonblocking);
}

unsigned PipeBackend::poll(xv::sync::PollTable *table)
{
  return pipe_->poll(writable_, table);
}

void EndpointBackend::close()
{
  endpoint_->close();
  this->~EndpointBackend();
}

int EndpointBackend::read(xv::io::Buffer destination, int count)
{
  return endpoint_->read(
      {xv::drivers::AddressSpace::User, destination.address, destination.nonblocking},
      count);
}

int EndpointBackend::write(xv::io::Buffer source, int count)
{
  return endpoint_->write(
      {xv::drivers::AddressSpace::User, source.address, source.nonblocking}, count);
}

int EndpointBackend::stat(xv::io::Metadata &value)
{
  value = {};
  value.type = static_cast<short>(xv::io::MetadataKind::Device);
  value.nlink = 1;
  return 0;
}

unsigned EndpointBackend::poll(xv::sync::PollTable *table)
{
  return endpoint_->poll(table);
}

int EndpointBackend::ioctl(unsigned request, xv::io::Buffer argument)
{
  return endpoint_->ioctl(request, {xv::drivers::AddressSpace::User, argument.address,
                                    argument.nonblocking});
}
} // namespace xv::fs
