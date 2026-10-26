module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
module xv.fs;
import :procfs;
import :platform;
namespace xv::fs
{
namespace
{
class Text final
{
public:
  Text(char *buffer, unsigned capacity) : buffer_(buffer), capacity_(capacity) {}
  void append(const char *text)
  {
    while (*text) { if (size_ == capacity_) { failed_ = true; return; } buffer_[size_++] = *text++; }
  }
  void number(unsigned long value)
  {
    char digits[24]; unsigned count = 0;
    do { digits[count++] = '0' + value % 10; value /= 10; } while (value);
    while (count) { char text[2] = {digits[--count], 0}; append(text); }
  }
  void field(const char *name, unsigned long value, const char *unit = "\n")
  { append(name); number(value); append(unit); }
  int size() const { return failed_ ? -1 : static_cast<int>(size_); }
private:
  char *buffer_;
  unsigned capacity_;
  unsigned size_ = 0;
  bool failed_ = false;
};
int pid_name(const char *name)
{
  if (!*name || *name == '0') return -1;
  unsigned value = 0;
  for (unsigned i = 0; i < DIRSIZ && name[i]; ++i)
  {
    if (name[i] < '0' || name[i] > '9' || value > (2147483647U - (name[i] - '0')) / 10) return -1;
    value = value * 10 + name[i] - '0';
  }
  return static_cast<int>(value);
}
}
void ProcFileSystem::initialize_inodes()
{
  references_.init("procfs refs");
  for (auto &node : nodes_)
  { node.owner_ = this; node.lock.init("procfs inode"); }
}
void ProcFileSystem::initialize(int device)
{
  device_ = device;
  for (unsigned i = 0; i < 3; ++i)
  {
    auto &node = nodes_[i];
    node.dev = device_; node.inum = i + 1;
    node.valid = node.nlink = 1; node.type = i == 0 ? T_DIR : T_FILE;
    node.addrs[0] = i;
  }
}
Inode *ProcFileSystem::get(unsigned kind, unsigned pid)
{
  xv::sync::ScopedLock guard(references_);
  Inode *empty = nullptr;
  for (unsigned i = 3; i < NodeCount; ++i)
  {
    auto &node = nodes_[i];
    if (node.ref && node.addrs[0] == kind && node.addrs[1] == pid)
    { ++node.ref; return &node; }
    if (!node.ref && !empty) empty = &node;
  }
  if (!empty) return nullptr;
  empty->dev = device_; empty->inum = 4 + 2 * pid + (kind == 4);
  empty->ref = 1; empty->valid = empty->nlink = 1;
  empty->type = kind == 3 ? T_DIR : T_FILE;
  empty->addrs[0] = kind; empty->addrs[1] = pid;
  return empty;
}
Inode *ProcFileSystem::retain(Inode *node)
{ xv::sync::ScopedLock guard(references_); ++node->ref; return node; }
void ProcFileSystem::release(Inode *node)
{
  xv::sync::ScopedLock guard(references_);
  if (node->ref < 1) detail::fail("procfs: release dead reference");
  --node->ref;
}
void ProcFileSystem::lock(Inode *node) { node->lock.acquire(); }
void ProcFileSystem::unlock(Inode *node) { node->lock.release(); }
void ProcFileSystem::unlock_release(Inode *node) { unlock(node); release(node); }
void ProcFileSystem::update(Inode *) { detail::fail("procfs: immutable metadata"); }
void ProcFileSystem::truncate(Inode *) { detail::fail("procfs: truncate"); }
void ProcFileSystem::stat(Inode *node, Stat *result)
{
  *result = {}; result->dev = node->dev; result->ino = node->inum;
  result->type = node->type; result->nlink = 1;
}
Inode *ProcFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *)
{
  if (detail::compare(name, ".", DIRSIZ) == 0) return retain(directory);
  if (detail::compare(name, "..", DIRSIZ) == 0) return root();
  if (directory->addrs[0] == 0)
  {
    if (detail::compare(name, "meminfo", DIRSIZ) == 0) return retain(&nodes_[1]);
    if (detail::compare(name, "uptime", DIRSIZ) == 0) return retain(&nodes_[2]);
    int pid = pid_name(name); detail::ProcessRecord record;
    if (pid < 1 || !detail::process_after(pid - 1, record) || record.pid != pid) return nullptr;
    return get(3, pid);
  }
  if (directory->addrs[0] == 3 && detail::compare(name, "status", DIRSIZ) == 0)
  {
    unsigned pid = directory->addrs[1]; detail::ProcessRecord record;
    if (!detail::process_after(pid - 1, record) || static_cast<unsigned>(record.pid) != pid) return nullptr;
    return get(4, pid);
  }
  return nullptr;
}
int ProcFileSystem::snapshot(Inode *node, char *buffer, unsigned capacity)
{
  unsigned kind = node->addrs[0];
  Text text(buffer, capacity);
  if (kind == 1)
  {
    unsigned long total, free; detail::memory_info(total, free);
    if (free > total) free = total;
    text.field("MemTotal: ", total, " kB\n");
    text.field("MemFree: ", free, " kB\n");
    text.field("MemAvailable: ", free, " kB\n");
    text.field("MemUsed: ", total - free, " kB\n");
  }
  else if (kind == 2)
  {
    unsigned long ticks = detail::uptime_ticks();
    text.number(ticks / 10); text.append("."); text.number(ticks % 10); text.append("0\n");
  }
  else if (kind == 4)
  {
    detail::ProcessRecord record; unsigned pid = node->addrs[1];
    if (!detail::process_after(pid - 1, record) || static_cast<unsigned>(record.pid) != pid) return -1;
    text.append("Name: "); text.append(record.name); text.append("\nState: ");
    char state[2] = {record.state, 0}; text.append(state); text.append("\n");
    text.field("Pid: ", record.pid); text.field("PPid: ", record.parent);
    text.field("Pgid: ", record.group > 0 ? record.group : 0);
    text.field("Sid: ", record.session > 0 ? record.session : 0);
    text.field("VmSize: ", record.bytes / 1024, " kB\n");
  }
  else
  {
    unsigned size = 0;
    auto entry = [&](const char *name, unsigned number) {
      if (capacity - size < sizeof(dirent)) return false;
      dirent value{}; value.inum = number;
      detail::copy_fixed(value.name, name, DIRSIZ);
      detail::move(buffer + size, &value, sizeof(value)); size += sizeof(value);
      return true;
    };
    if (!entry(".", node->inum) || !entry("..", 1)) return -1;
    if (kind == 3)
    {
      detail::ProcessRecord record; unsigned pid = node->addrs[1];
      if (!detail::process_after(pid - 1, record) || static_cast<unsigned>(record.pid) != pid) return -1;
      if (!entry("status", node->inum + 1)) return -1;
    }
    else
    {
      if (!entry("meminfo", 2) || !entry("uptime", 3)) return -1;
      int after = 0; detail::ProcessRecord record;
      while (detail::process_after(after, record))
      {
        char name[16]{}; Text number(name, sizeof(name) - 1); number.number(record.pid);
        if (!entry(name, 4 + 2 * (record.pid % 32765))) return -1;
        after = record.pid;
      }
    }
    return size;
  }
  return text.size();
}
int ProcFileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
{
  auto *page = static_cast<char *>(detail::allocate_page());
  if (!page) return -1;
  int length = snapshot(node, page, 4096);
  int result = -1;
  if (length >= 0)
  {
    unsigned part = offset >= static_cast<unsigned>(length) ? 0 : length - offset;
    if (part > count) part = count;
    result = part && detail::copy_out(user, destination, page + offset, part) < 0 ? -1 : static_cast<int>(part);
  }
  detail::free_page(page);
  return result;
}
}
