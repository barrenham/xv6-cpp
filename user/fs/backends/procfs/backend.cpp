module;
#include "kernel/io/fs_resources.h"
#include "user/runtime/user.h"
#include "kernel/support/types.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/stat.h"
#include "user/fs/backends/xv6/format.h"
module xv.userfs.procfs;
import xv.userfs;
import xv.userfs.resources;
import xv.userfs.snapshot;

namespace userfs
{
const char *const ProcessBackend::global_names_[GlobalCount] = {
  "fsplugins", "bcache", "meminfo", "uptime", "stat", "cpuinfo", "filesystems",
  "mounts", "devices", "partitions", "version"
};
ProcessBackend::~ProcessBackend()
{
  while (nodes_) { auto *node = nodes_; nodes_ = node->next; free(node); }
}
bool ProcessBackend::decimal(const char *text, unsigned &value, bool zero)
{
  value = 0;
  if (!*text || (*text == '0' && (!zero || text[1]))) return false;
  for (unsigned i = 0; text[i]; ++i)
  {
    if (i >= 10 || text[i] < '0' || text[i] > '9' ||
        value > (0x7fffffffU - (text[i] - '0')) / 10) return false;
    value = value * 10 + text[i] - '0';
  }
  return true;
}
void ProcessBackend::number_name(unsigned value, char *out)
{
  char digits[12]; unsigned count = 0;
  do { digits[count++] = '0' + value % 10; value /= 10; } while (value);
  while (count) *out++ = digits[--count];
  *out = 0;
}
bool ProcessBackend::directory(unsigned kind)
{
  return kind == Root || kind == ProcessDirectory || kind == FdDirectory || kind == FdInfoDirectory;
}
ProcessBackend::Node *ProcessBackend::find(unsigned id)
{
  for (auto *node = nodes_; node; node = node->next) if (node->id == id) return node;
  return nullptr;
}
bool ProcessBackend::key(unsigned id, Node &out)
{
  if (id == 1) { out = {}; out.id = id; return true; }
  if (id >= 2 && id < 2 + GlobalCount)
  { out = {}; out.id = id; out.kind = Global; out.descriptor = id - 2; return true; }
  if (auto *node = find(id)) { out = *node; return true; }
  return false;
}
bool ProcessBackend::process(unsigned pid, xv::fsservice::Process &out)
{
  return pid && resources_.query(xv::fsservice::Operation::ProcessAfter, pid - 1, out) == 1 &&
         unsigned(out.pid) == pid;
}
unsigned ProcessBackend::identity(const Node &key) const
{
  for (auto *node = nodes_; node; node = node->next)
    if (node->kind == key.kind && node->pid == key.pid && node->descriptor == key.descriptor &&
        node->uid == key.uid && node->gid == key.gid) return node->id;
  // Enumeration establishes no borrow. An already borrowed cwd still obtains
  // its exact ID; unborrowed transient records use a separate nonzero range.
  return 0x80000000U | ((key.pid * 37U + key.kind * 17U + key.descriptor) & 0x7fffffffU);
}
bool ProcessBackend::attributes(const Node &node, Attributes &out)
{
  unsigned mode = node.kind == Root || node.kind == ProcessDirectory ? 0555 :
      node.kind == FdDirectory || node.kind == FdInfoDirectory ? 0500 :
      node.kind == Fd || node.kind == FdInfo || node.kind == Command || node.kind == Environment ? 0400 : 0444;
  out = {node.id, 0, node.uid, node.gid, mode,
          directory(node.kind) ? unsigned(T_DIR) : unsigned(T_FILE), 1};
  return true;
}
bool ProcessBackend::describe(unsigned kind, unsigned pid, unsigned descriptor,
                               Attributes &out, bool borrow)
{
  xv::fsservice::Process record{};
  if (!process(pid, record)) return false;
  Node key;
  key.kind = kind; key.pid = pid; key.descriptor = descriptor;
  key.uid = record.effective_uid; key.gid = record.effective_gid;
  key.id = identity(key);
  if (borrow && (key.id & 0x80000000U))
  {
    if (next_id_ >= 0x80000000U) return false;
    auto *memory = malloc(sizeof(Node));
    if (!memory) return false;
    auto *node = new (memory) Node(key);
    node->id = next_id_++;
    node->next = nodes_; nodes_ = node;
    key.id = node->id;
  }
  return attributes(key, out);
}
bool ProcessBackend::descriptors(unsigned pid)
{
  xv::fsservice::Query request;
  request.operation = xv::fsservice::Operation::CaptureDescriptors;
  request.pid = pid; request.inspector = actor_.uid;
  return resources_.exchange(request) == 1;
}
bool ProcessBackend::descriptor(unsigned pid, unsigned fd, xv::fsservice::Descriptor &out)
{
  if (!descriptors(pid)) return false;
  for (unsigned index = 0;; ++index)
  {
    if (resources_.query(xv::fsservice::Operation::Descriptor, index, out) != 1) return false;
    if (out.number == fd) return true;
  }
}
int ProcessBackend::getattr(unsigned id, Attributes &out)
{
  Node node;
  return key(id, node) && attributes(node, out) ? 0 : -1;
}
int ProcessBackend::lookup(unsigned parent, const char *name, Attributes &out)
{
  Node node;
  if (!key(parent, node) || !directory(node.kind)) return -1;
  if (!strcmp(name, ".")) return getattr(parent, out);
  if (!strcmp(name, ".."))
  {
    if (node.kind == FdDirectory || node.kind == FdInfoDirectory)
      return describe(ProcessDirectory, node.pid, 0, out, true) ? 0 : -1;
    return getattr(1, out);
  }
  if (node.kind == Root)
  {
    for (unsigned i = 0; i < GlobalCount; ++i)
      if (!strcmp(name, global_names_[i])) return getattr(i + 2, out);
    unsigned pid;
    if (!strcmp(name, "self")) pid = actor_.pid;
    else if (!decimal(name, pid)) return -1;
    return describe(ProcessDirectory, pid, 0, out, true) ? 0 : -1;
  }
  if (node.kind == ProcessDirectory)
  {
    const char *const names[] = {"status", "fd", "fdinfo", "cmdline", "environ", "comm"};
    const unsigned kinds[] = {Status, FdDirectory, FdInfoDirectory, Command, Environment, Name};
    for (unsigned i = 0; i < 6; ++i)
      if (!strcmp(name, names[i])) return describe(kinds[i], node.pid, 0, out, true) ? 0 : -1;
    return -1;
  }
  unsigned fd;
  xv::fsservice::Descriptor record{};
  if (!decimal(name, fd, true) || !descriptor(node.pid, fd, record)) return -1;
  return describe(node.kind == FdDirectory ? Fd : FdInfo, node.pid, fd, out, true) ? 0 : -1;
}
void ProcessBackend::forget(unsigned id)
{
  auto **link = &nodes_;
  while (*link && (*link)->id != id) link = &(*link)->next;
  if (*link) { auto *node = *link; *link = node->next; free(node); }
}
int ProcessBackend::readdir(unsigned id, unsigned long offset, Entry &out)
{
  Node node;
  if (!key(id, node) || !directory(node.kind)) return -1;
  out.next = offset + 1;
  if (offset < 2)
  {
    strcpy(out.name, offset ? ".." : ".");
    if (!offset) return getattr(id, out.attributes) == 0 ? 1 : -1;
    if (node.kind == FdDirectory || node.kind == FdInfoDirectory)
      return describe(ProcessDirectory, node.pid, 0, out.attributes, false) ? 1 : -1;
    return getattr(1, out.attributes) == 0 ? 1 : -1;
  }
  if (node.kind == Root)
  {
    if (offset == 2)
    {
      strcpy(out.name, "self");
      return describe(ProcessDirectory, actor_.pid, 0, out.attributes, false) ? 1 : -1;
    }
    if (offset < 3 + GlobalCount)
    {
      strcpy(out.name, global_names_[offset - 3]);
      return getattr(offset - 1, out.attributes) == 0 ? 1 : -1;
    }
    unsigned long after = offset - (3 + GlobalCount);
    xv::fsservice::Process record{};
    if (after > 0x7fffffffUL) return 0;
    int result = resources_.query(xv::fsservice::Operation::ProcessAfter, after, record);
    if (result != 1) return result;
    number_name(record.pid, out.name);
    Node child;
    child.kind = ProcessDirectory; child.pid = record.pid;
    child.uid = record.effective_uid; child.gid = record.effective_gid;
    child.id = identity(child);
    attributes(child, out.attributes);
    out.next = 3 + GlobalCount + unsigned(record.pid);
    return 1;
  }
  if (node.kind == ProcessDirectory)
  {
    const char *const names[] = {"status", "fd", "fdinfo", "cmdline", "environ", "comm"};
    const unsigned kinds[] = {Status, FdDirectory, FdInfoDirectory, Command, Environment, Name};
    if (offset >= 8) return 0;
    strcpy(out.name, names[offset - 2]);
    return describe(kinds[offset - 2], node.pid, 0, out.attributes, false) ? 1 : -1;
  }
  if (!descriptors(node.pid)) return -1;
  xv::fsservice::Descriptor record{};
  int result = resources_.query(xv::fsservice::Operation::Descriptor, offset - 2, record);
  if (result != 1) return result;
  number_name(record.number, out.name);
  return describe(node.kind == FdDirectory ? Fd : FdInfo, node.pid,
                   record.number, out.attributes, false) ? 1 : -1;
}
bool ProcessBackend::record(const char *name, unsigned identity)
{
  dirent entry{};
  entry.inum = 1 + identity % 65535;
  for (unsigned i = 0; i < DIRSIZ && name[i]; ++i) entry.name[i] = name[i];
  return bytes_.append(&entry, sizeof(entry));
}
bool ProcessBackend::render_directory(const Node &node)
{
  if (node.kind == FdDirectory || node.kind == FdInfoDirectory)
  {
    if (!descriptors(node.pid) || !record(".", node.id) || !record("..", node.pid)) return false;
    xv::fsservice::Descriptor descriptor{};
    for (unsigned i = 0;; ++i)
    {
      int result = resources_.query(xv::fsservice::Operation::Descriptor, i, descriptor);
      if (!result) return true;
      if (result < 0) return false;
      char name[16]; number_name(descriptor.number, name);
      if (!record(name, node.id + 1 + descriptor.number)) return false;
    }
  }
  unsigned long offset = 0;
  Entry entry;
  for (;;)
  {
    int result = readdir(node.id, offset, entry);
    if (!result) return true;
    if (result < 0 || entry.next <= offset || !record(entry.name, entry.attributes.node)) return false;
    offset = entry.next;
  }
}
bool ProcessBackend::render_exec(const Node &node)
{
  xv::fsservice::Query query;
  query.operation = xv::fsservice::Operation::CaptureExec;
  query.pid = node.pid; query.index = node.kind == Environment;
  query.inspector = actor_.uid;
  bool good = resources_.exchange(query) == 1;
  unsigned long total = query.offset, offset = 0;
  while (good && offset < total)
  {
    query = {};
    query.operation = xv::fsservice::Operation::ReadExec;
    query.offset = offset;
    query.count = total - offset < xv::fsservice::PayloadSize ? total - offset : xv::fsservice::PayloadSize;
    unsigned count = query.count;
    good = resources_.exchange(query) > 0 && query.count <= count &&
           query.result == int(query.count) && bytes_.append(query.data, query.count);
    offset += good ? query.count : 0;
  }
  query = {};
  query.operation = xv::fsservice::Operation::EndExec;
  return resources_.exchange(query) == 0 && good;
}
bool ProcessBackend::render_process(const Node &node)
{
  if (node.kind == Command || node.kind == Environment) return render_exec(node);
  if (node.kind == Fd || node.kind == FdInfo)
  {
    xv::fsservice::Descriptor value{};
    if (!descriptor(node.pid, node.descriptor, value)) return false;
    if (node.kind == FdInfo)
    {
      bytes_.field("pos:\t", value.position);
      bytes_.text("flags:\t0"); bytes_.number(value.flags, 8); bytes_.text("\n");
      bytes_.field("ino:\t", value.inode);
      return bytes_.good();
    }
    const char *kind = value.kind == 1 ? value.type == T_DIR ? "directory" : value.type == T_DEVICE ? "device" : "file" :
                       value.kind == 2 ? "pipe" : value.kind == 3 ? "terminal" : value.kind == 4 ? "socket" : "unknown";
    bytes_.field("Descriptor: ", value.number); bytes_.field("Identity: ", value.identity);
    bytes_.field("Flags: ", value.flags); bytes_.text("Type: "); bytes_.text(kind); bytes_.text("\n");
    bytes_.field("Device: ", value.device); bytes_.field("Inode: ", value.inode);
    return bytes_.good();
  }
  xv::fsservice::Process value{};
  if (!process(node.pid, value)) return false;
  if (node.kind == Name) { bytes_.text(value.name); bytes_.text("\n"); return bytes_.good(); }
  bytes_.text("Name: "); bytes_.text(value.name); bytes_.text("\nState: ");
  bytes_.append(&value.state, 1); bytes_.text("\n");
  bytes_.field("Uid: ", value.uid); bytes_.field("Euid: ", value.effective_uid);
  bytes_.field("Gid: ", value.gid); bytes_.field("Egid: ", value.effective_gid);
  bytes_.field("Pid: ", value.pid); bytes_.field("PPid: ", value.parent);
  bytes_.field("Pgid: ", value.group > 0 ? value.group : 0);
  bytes_.field("Sid: ", value.session > 0 ? value.session : 0);
  bytes_.field("VmSize: ", value.bytes / 1024, " kB\n");
  return bytes_.good();
}
int ProcessBackend::snapshot(unsigned id, unsigned &size)
{
  bytes_.clear();
  Node node;
  if (!key(id, node)) return -1;
  bool okay = directory(node.kind) ? render_directory(node) :
              node.kind == Global ? render_global(node.descriptor) : render_process(node);
  if (!okay || !bytes_.good()) { bytes_.clear(); return -1; }
  size = bytes_.size();
  return 0;
}
int ProcessBackend::snapshot_read(unsigned long offset, void *data, unsigned count)
{
  return bytes_.read(offset, data, count);
}
int ProcessBackend::read(unsigned id, unsigned long offset, void *data, unsigned count)
{
  unsigned size;
  if (snapshot(id, size) < 0) return -1;
  int result = snapshot_read(offset, data, count);
  snapshot_end();
  return result;
}
} // namespace userfs
