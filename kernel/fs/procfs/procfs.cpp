module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/support/param.h"
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
  explicit Text(PageBuffer &buffer) : buffer_(buffer) {}
  void append(const char *text)
  {
    unsigned count = 0;
    while (text[count]) ++count;
    if (!buffer_.append(text, count)) failed_ = true;
  }
  void number(unsigned long value)
  {
    char digits[24]; unsigned count = 0;
    do { digits[count++] = '0' + value % 10; value /= 10; } while (value);
    while (count) { char text[2] = {digits[--count], 0}; append(text); }
  }
  void field(const char *name, unsigned long value, const char *unit = "\n")
  { append(name); number(value); append(unit); }
  bool good() const { return !failed_; }
private:
  PageBuffer &buffer_;
  bool failed_ = false;
};
int component_number(const char *name, bool zero = false)
{
  if (!*name || (*name == '0' && (!zero || name[1]))) return -1;
  unsigned value = 0;
  for (unsigned i = 0; i < DIRSIZ && name[i]; ++i)
  {
    if (name[i] < '0' || name[i] > '9' || value > (2147483647U - (name[i] - '0')) / 10) return -1;
    value = value * 10 + name[i] - '0';
  }
  return static_cast<int>(value);
}
void number_name(unsigned value, char *output)
{
  char digits[12]; unsigned count = 0;
  do { digits[count++] = '0' + value % 10; value /= 10; } while (value);
  unsigned size = count;
  for (unsigned i = 0; i < size; ++i) output[i] = digits[size - i - 1];
  output[size] = 0;
}
const char *file_kind(const FileInfo &file)
{
  switch (file.kind)
  {
    case FileKind::Node: return file.type == T_DIR ? "directory" : file.type == T_DEVICE ? "device" : "file";
    case FileKind::Pipe: return "pipe";
    case FileKind::Terminal: return "terminal";
    case FileKind::Socket: return "socket";
    default: return "unknown";
  }
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
    node.addrs[0] = i; node.mode_ = i == 0 ? 0555 : 0444;
  }
}
Inode *ProcFileSystem::get(unsigned kind, unsigned pid, unsigned descriptor)
{
  detail::ProcessRecord owner;
  if (!detail::process_after(pid - 1, owner) || static_cast<unsigned>(owner.pid) != pid) return nullptr;
  xv::sync::ScopedLock guard(references_);
  Inode *empty = nullptr;
  for (unsigned i = 3; i < NodeCount; ++i)
  {
    auto &node = nodes_[i];
    if (node.ref && node.addrs[0] == kind && node.addrs[1] == pid && node.addrs[2] == descriptor && node.uid_ == owner.effective_uid && node.gid_ == owner.effective_gid)
    { ++node.ref; return &node; }
    if (!node.ref && !empty) empty = &node;
  }
  if (!empty) return nullptr;
  empty->dev = device_; empty->inum = 16 + (pid * (NOFILE + 3) + (kind == 3 ? 0 : kind == 4 ? 1 : kind == 5 ? 2 : 3 + descriptor)) % 0xffffffe0U;
  empty->uid_ = owner.effective_uid; empty->gid_ = owner.effective_gid;
  empty->mode_ = kind == 5 ? 0500 : kind == 6 ? 0400 : kind == 3 ? 0555 : 0444;
  empty->ref = 1; empty->valid = empty->nlink = 1;
  empty->type = kind == 3 || kind == 5 ? T_DIR : T_FILE;
  empty->addrs[0] = kind; empty->addrs[1] = pid; empty->addrs[2] = descriptor;
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
  result->type = node->type; result->nlink = 1; node->security_stat(result);
}
Inode *ProcFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *)
{
  unsigned kind = directory->addrs[0], pid = directory->addrs[1];
  if (detail::compare(name, ".", DIRSIZ) == 0) return retain(directory);
  if (detail::compare(name, "..", DIRSIZ) == 0) return kind == 5 ? get(3, pid) : root();
  if (kind == 0)
  {
    if (detail::compare(name, "meminfo", DIRSIZ) == 0) return retain(&nodes_[1]);
    if (detail::compare(name, "uptime", DIRSIZ) == 0) return retain(&nodes_[2]);
    int target = detail::compare(name, "self", DIRSIZ) == 0 ? detail::current_pid() : component_number(name);
    detail::ProcessRecord record;
    if (target < 1 || !detail::process_after(target - 1, record) || record.pid != target) return nullptr;
    return get(3, target);
  }
  if (kind == 3)
  {
    detail::ProcessRecord record;
    if (!detail::process_after(pid - 1, record) || static_cast<unsigned>(record.pid) != pid) return nullptr;
    if (detail::compare(name, "status", DIRSIZ) == 0) return get(4, pid);
    if (detail::compare(name, "fd", DIRSIZ) == 0) return get(5, pid);
  }
  if (kind == 5)
  {
    int number = component_number(name, true);
    if (number < 0 || number >= NOFILE) return nullptr;
    detail::DescriptorRecord records[NOFILE];
    int count = detail::descriptor_snapshot(pid, records, NOFILE);
    for (int i = 0; i < count; ++i) if (records[i].descriptor == number) return get(6, pid, number);
  }
  return nullptr;
}
bool ProcFileSystem::snapshot(Inode *node, PageBuffer &buffer)
{
  unsigned kind = node->addrs[0];
  Text text(buffer);
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
    if (!detail::process_after(pid - 1, record) || static_cast<unsigned>(record.pid) != pid) return false;
    text.append("Name: "); text.append(record.name); text.append("\nState: ");
    char state[2] = {record.state, 0}; text.append(state); text.append("\n");
    text.field("Uid: ", record.uid); text.field("Euid: ", record.effective_uid);
    text.field("Gid: ", record.gid); text.field("Egid: ", record.effective_gid);
    text.field("Pid: ", record.pid); text.field("PPid: ", record.parent);
    text.field("Pgid: ", record.group > 0 ? record.group : 0);
    text.field("Sid: ", record.session > 0 ? record.session : 0);
    text.field("VmSize: ", record.bytes / 1024, " kB\n");
  }
  else if (kind == 6)
  {
    detail::DescriptorRecord records[NOFILE];
    int count = detail::descriptor_snapshot(node->addrs[1], records, NOFILE);
    const detail::DescriptorRecord *record = nullptr;
    for (int i = 0; i < count; ++i) if (records[i].descriptor == static_cast<int>(node->addrs[2])) record = &records[i];
    if (!record) return false;
    text.field("Descriptor: ", record->descriptor);
    text.field("Identity: ", record->file.identity);
    text.field("Flags: ", record->file.flags);
    text.append("Type: "); text.append(file_kind(record->file)); text.append("\n");
    text.field("Device: ", record->file.device);
    text.field("Inode: ", record->file.inode);
  }
  else
  {
    auto entry = [&](const char *name, unsigned number) {
      dirent value{}; value.inum = 1 + number % 65535;
      detail::copy_fixed(value.name, name, DIRSIZ);
      return buffer.append(&value, sizeof(value));
    };
    if (!entry(".", node->inum) || !entry("..", 1)) return false;
    if (kind == 3)
    {
      detail::ProcessRecord record; unsigned pid = node->addrs[1];
      if (!detail::process_after(pid - 1, record) || static_cast<unsigned>(record.pid) != pid) return false;
      if (!entry("status", node->inum + 1) || !entry("fd", node->inum + 2)) return false;
    }
    else if (kind == 5)
    {
      detail::DescriptorRecord records[NOFILE];
      int count = detail::descriptor_snapshot(node->addrs[1], records, NOFILE);
      if (count < 0) return false;
      for (int i = 0; i < count; ++i)
      {
        char name[16]; number_name(records[i].descriptor, name);
        if (!entry(name, node->inum + 1 + records[i].descriptor)) return false;
      }
    }
    else
    {
      if (!entry("self", 4) || !entry("meminfo", 2) || !entry("uptime", 3)) return false;
      int after = 0; detail::ProcessRecord record;
      while (detail::process_after(after, record))
      {
        char name[16]; number_name(record.pid, name);
        if (!entry(name, 4 + 2 * (record.pid % 32765))) return false;
        after = record.pid;
      }
    }
    return true;
  }
  return text.good();
}
int ProcFileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
{
  PageBuffer contents;
  if (!snapshot(node, contents)) return -1;
  return contents.read(offset, user, destination, count);
}
}
