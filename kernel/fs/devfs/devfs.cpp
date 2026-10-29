module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
module xv.fs;
import :devfs_flow;
import :devfs;
import :platform;

namespace xv::fs
{
namespace
{
class MemoryDevice final : public xv::drivers::CharacterDevice
{
public:

  constexpr explicit MemoryDevice(bool zero) : zero_(zero)
  {
  }

  const char *name() const override
  {
    return zero_ ? "zero" : "null";
  }

  void initialize() override
  {
  }

  void put_sync(int) override
  {
  }

  int read(xv::drivers::IoBuffer destination, int count) override
  {
    if (count < 0)
    {
      return -1;
    }
    if (!zero_)
    {
      return 0;
    }
    const unsigned char zeros[128]{};
    int done = 0;
    while (done < count)
    {
      unsigned part = count - done;
      if (part > sizeof(zeros))
      {
        part = sizeof(zeros);
      }
      if (detail::copy_out(destination.space == xv::drivers::AddressSpace::User,
                           destination.address + done, zeros, part) < 0)
      {
        return done ? done : -1;
      }
      done += part;
    }
    return done;
  }

  int write(xv::drivers::IoBuffer, int count) override
  {
    return count < 0 ? -1 : count;
  }

  unsigned poll(xv::sync::PollTable *) override
  {
    return POLLIN | POLLOUT;
  }

private:

  bool zero_;
};

constinit MemoryDevice null_device(false);
constinit MemoryDevice zero_device(true);
} // namespace

void DevFileSystem::initialize_inodes()
{
  references_.init("devfs refs");
  for (auto &node : nodes_)
  {
    node.owner_ = this;
    node.lock.init("devfs inode");
  }
}

void DevFileSystem::initialize(int device)
{
  devices().attach("null", 2, 0, null_device);
  devices().attach("zero", 3, 0, zero_device);
  devices().seal();
  count_ = 0;
  for (unsigned i = 0; i < devices().count(); ++i)
  {
    const auto &record = devices().entry(i);
    if (!record.name[0])
    {
      continue;
    }
    records_[count_++] = i;
    auto &node = nodes_[count_];
    node.dev = device;
    node.inum = count_ + 1;
    node.type = T_DEVICE;
    node.uid_ = node.gid_ = 0;
    node.mode_ = detail::compare(record.name, "console", DIRSIZ) == 0 ? 0600 : 0666;
    node.major = record.major;
    node.minor = record.minor;
    node.valid = 1;
    node.nlink = 1;
  }
  auto &directory = nodes_[0];
  directory.dev = device;
  directory.inum = 1;
  directory.type = T_DIR;
  directory.mode_ = 0555;
  directory.valid = 1;
  directory.nlink = 1;
  directory.size = (count_ + 3) * sizeof(dirent);
  auto &pts = nodes_[PtsRoot];
  pts.dev = device;
  pts.inum = PtsRoot + 1;
  pts.type = T_DIR;
  pts.mode_ = 0555;
  pts.valid = pts.nlink = 1;
  pts.size = (2 + xv::drivers::MaxNamedPtys) * sizeof(dirent);
  for (unsigned i = 0; i < xv::drivers::MaxNamedPtys; ++i)
  {
    auto &node = nodes_[PtsStart + i];
    node.dev = device;
    node.inum = PtsStart + i + 1;
    node.type = T_DEVICE;
    node.uid_ = node.gid_ = 0;
    node.mode_ = 0600;
    node.major = 5;
    node.minor = i;
    node.valid = node.nlink = 1;
  }
}

Inode *DevFileSystem::retain(Inode *node)
{
  xv::sync::ScopedLock guard(references_);
  ++node->ref;
  return node;
}

void DevFileSystem::release(Inode *node)
{
  references_.acquire();
  ReleaseFlow::Context context{*this, node};
  (void)ReleaseFlow::Graph::run<0>(context);
}

void DevFileSystem::lock(Inode *node)
{
  node->lock.acquire();
}

void DevFileSystem::unlock(Inode *node)
{
  node->lock.release();
}

void DevFileSystem::unlock_release(Inode *node)
{
  unlock(node);
  release(node);
}

void DevFileSystem::stat(Inode *node, Stat *result)
{
  *result = {};
  result->dev = node->dev;
  result->ino = node->inum;
  result->type = node->type;
  result->nlink = node->nlink;
  result->size = node->size;
  node->security_stat(result);
}

int DevFileSystem::read(Inode *node, int user, unsigned long destination,
                        unsigned offset, unsigned count)
{
  unsigned numbers[xv::drivers::MaxNamedPtys];
  ReadFlow::Context context{*this,  node,  user,    destination,
                            offset, count, numbers, node == &nodes_[PtsRoot]};
  (void)ReadFlow::Graph::run<0>(context);
  return context.result;
}

Inode *DevFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset)
{
  LookupFlow::Context context{*this, directory, name, offset,
                              directory == &nodes_[PtsRoot]};
  (void)LookupFlow::Graph::run<0>(context);
  return context.result;
}

xv::drivers::CharacterDevice *DevFileSystem::open_device(Inode *node, bool nonblocking)
{
  if (node->inum > PtsStart)
  {
    return xv::drivers::open_named_pty(node->minor);
  }
  return open_registered_device(node, nonblocking);
}

Inode *DevFileSystem::create_at(Inode *directory, const char *name, short type, short,
                                short)
{
  CreateFlow::Context context{*this, directory, name, type};
  (void)CreateFlow::Graph::run<0>(context);
  return context.result;
}

Inode *DevFileSystem::allocate(unsigned, short)
{
  return nullptr;
}

void DevFileSystem::update(Inode *)
{
  detail::fail("devfs: immutable metadata");
}

void DevFileSystem::truncate(Inode *)
{
  detail::fail("devfs: truncate");
}

int DevFileSystem::write(Inode *, int, unsigned long, unsigned, unsigned)
{
  return -1;
}

int DevFileSystem::link_at(Inode *, const char *, Inode *)
{
  return -1;
}

int DevFileSystem::unlink_at(Inode *, const char *)
{
  return -1;
}
} // namespace xv::fs
