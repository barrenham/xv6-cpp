module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
module xv.fs;
import :devfs;
import :platform;
namespace xv::fs
{
namespace
{
class MemoryDevice final : public xv::drivers::CharacterDevice
{
public:
  constexpr explicit MemoryDevice(bool zero) : zero_(zero) {}
  const char *name() const override { return zero_ ? "zero" : "null"; }
  void initialize() override {}
  void put_sync(int) override {}
  int read(xv::drivers::IoBuffer destination, int count) override
  {
    if (count < 0) return -1;
    if (!zero_) return 0;
    const unsigned char zeros[128]{};
    int done = 0;
    while (done < count)
    {
      unsigned part = count - done;
      if (part > sizeof(zeros)) part = sizeof(zeros);
      if (detail::copy_out(destination.space == xv::drivers::AddressSpace::User,
                           destination.address + done, zeros, part) < 0)
        return done ? done : -1;
      done += part;
    }
    return done;
  }
  int write(xv::drivers::IoBuffer, int count) override { return count < 0 ? -1 : count; }
  unsigned poll(xv::sync::PollTable *) override { return POLLIN | POLLOUT; }
private:
  bool zero_;
};
constinit MemoryDevice null_device(false);
constinit MemoryDevice zero_device(true);
}
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
    if (!record.name[0]) continue;
    records_[count_++] = i;
    auto &node = nodes_[count_];
    node.dev = device;
    node.inum = count_ + 1;
    node.type = T_DEVICE;
    node.major = record.major;
    node.minor = record.minor;
    node.valid = 1;
    node.nlink = 1;
  }
  auto &directory = nodes_[0];
  directory.dev = device;
  directory.inum = 1;
  directory.type = T_DIR;
  directory.valid = 1;
  directory.nlink = 1;
  directory.size = (count_ + 3) * sizeof(dirent);
  auto &pts = nodes_[PtsRoot];
  pts.dev = device;
  pts.inum = PtsRoot + 1;
  pts.type = T_DIR;
  pts.valid = pts.nlink = 1;
  pts.size = (2 + xv::drivers::MaxNamedPtys) * sizeof(dirent);
  for (unsigned i = 0; i < xv::drivers::MaxNamedPtys; ++i)
  {
    auto &node = nodes_[PtsStart + i];
    node.dev = device; node.inum = PtsStart + i + 1;
    node.type = T_DEVICE; node.major = 5; node.minor = i;
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
  if (node->ref < 1) detail::fail("devfs: release dead reference");
  bool unpin = --node->ref == 0 && node->inum > PtsStart;
  unsigned number = node->minor;
  references_.release();
  if (unpin) xv::drivers::release_named_pty(number);
}
void DevFileSystem::lock(Inode *node) { node->lock.acquire(); }
void DevFileSystem::unlock(Inode *node) { node->lock.release(); }
void DevFileSystem::unlock_release(Inode *node) { unlock(node); release(node); }
void DevFileSystem::stat(Inode *node, Stat *result)
{
  *result = {};
  result->dev = node->dev;
  result->ino = node->inum;
  result->type = node->type;
  result->nlink = node->nlink;
  result->size = node->size;
}
namespace
{
void decimal_name(unsigned value, char *name)
{
  char digits[12]; unsigned count = 0;
  do { digits[count++] = '0' + value % 10; value /= 10; } while (value);
  for (unsigned i = 0; i < count; ++i) name[i] = digits[count - i - 1];
  name[count] = 0;
}
}
int DevFileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
{
  bool pts = node == &nodes_[PtsRoot];
  if (!pts && node != &nodes_[0]) return -1;
  unsigned numbers[xv::drivers::MaxNamedPtys];
  unsigned live = pts ? xv::drivers::named_pty_ids(numbers, xv::drivers::MaxNamedPtys) : 0;
  unsigned records = pts ? 2 + xv::drivers::MaxNamedPtys : count_ + 3;
  unsigned size = records * sizeof(dirent);
  if (offset >= size) return 0;
  if (count > size - offset) count = size - offset;
  unsigned done = 0;
  while (done < count)
  {
    unsigned index = (offset + done) / sizeof(dirent);
    unsigned inside = (offset + done) % sizeof(dirent);
    dirent entry{};
    if (index < 2)
    {
      entry.inum = pts && index == 0 ? PtsRoot + 1 : 1;
      detail::copy_fixed(entry.name, index == 0 ? "." : "..", DIRSIZ);
    }
    else if (pts)
    {
      unsigned number = index - 2;
      for (unsigned i = 0; i < live; ++i)
        if (numbers[i] == number) { entry.inum = PtsStart + number + 1; decimal_name(number, entry.name); break; }
    }
    else if (index == count_ + 2)
    { entry.inum = PtsRoot + 1; detail::copy_fixed(entry.name, "pts", DIRSIZ); }
    else
    { entry.inum = index; detail::copy_fixed(entry.name, devices().entry(records_[index - 2]).name, DIRSIZ); }
    unsigned part = sizeof(entry) - inside;
    if (part > count - done) part = count - done;
    if (detail::copy_out(user, destination + done, reinterpret_cast<const char *>(&entry) + inside, part) < 0)
      return done ? static_cast<int>(done) : -1;
    done += part;
  }
  return done;
}
Inode *DevFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset)
{
  bool pts = directory == &nodes_[PtsRoot];
  if (!pts && directory != &nodes_[0]) return nullptr;
  if (detail::compare(name, ".", DIRSIZ) == 0) return retain(directory);
  if (detail::compare(name, "..", DIRSIZ) == 0) return retain(&nodes_[0]);
  if (pts)
  {
    unsigned number = 0, length = 0;
    if (!name[0] || (name[0] == '0' && name[1])) return nullptr;
    while (length < DIRSIZ && name[length])
    {
      if (name[length] < '0' || name[length] > '9') return nullptr;
      number = number * 10 + name[length++] - '0';
      if (number >= xv::drivers::MaxNamedPtys) return nullptr;
    }
    if (!xv::drivers::retain_named_pty(number)) return nullptr;
    auto *node = &nodes_[PtsStart + number];
    references_.acquire();
    bool already_pinned = node->ref++ != 0;
    references_.release();
    if (already_pinned) xv::drivers::release_named_pty(number);
    if (offset) *offset = (number + 2) * sizeof(dirent);
    return node;
  }
  if (detail::compare(name, "pts", DIRSIZ) == 0) return retain(&nodes_[PtsRoot]);
  for (unsigned i = 0; i < count_; ++i)
    if (detail::compare(name, devices().entry(records_[i]).name, DIRSIZ) == 0)
    { if (offset) *offset = (i + 2) * sizeof(dirent); return retain(&nodes_[i + 1]); }
  return nullptr;
}
xv::drivers::CharacterDevice *DevFileSystem::open_device(Inode *node, bool nonblocking)
{
  if (node->inum > PtsStart) return xv::drivers::open_named_pty(node->minor);
  return open_registered_device(node, nonblocking);
}
Inode *DevFileSystem::create_at(Inode *directory, const char *name, short type, short, short)
{
  if (type != T_FILE) return nullptr;
  lock(directory);
  Inode *node = lookup_entry(directory, name, nullptr);
  unlock(directory);
  if (!node) return nullptr;
  lock(node);
  if (node->type == T_DEVICE) return node;
  unlock_release(node);
  return nullptr;
}
Inode *DevFileSystem::allocate(unsigned, short) { return nullptr; }
void DevFileSystem::update(Inode *) { detail::fail("devfs: immutable metadata"); }
void DevFileSystem::truncate(Inode *) { detail::fail("devfs: truncate"); }
int DevFileSystem::write(Inode *, int, unsigned long, unsigned, unsigned) { return -1; }
int DevFileSystem::link_at(Inode *, const char *, Inode *) { return -1; }
int DevFileSystem::unlink_at(Inode *, const char *) { return -1; }
}
