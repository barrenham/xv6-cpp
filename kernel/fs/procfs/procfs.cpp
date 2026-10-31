module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/support/param.h"
#include "kernel/support/flow.h"
module xv.fs;
import :procfs;
import :platform;
import :snapshot_text;

namespace xv::fs
{
namespace
{
using Text=SnapshotText;

int component_number(const char *name, bool zero = false)
{
  if (!*name || (*name == '0' && (!zero || name[1])))
  {
    return -1;
  }
  unsigned value = 0;
  for (unsigned i = 0; i < DIRSIZ && name[i]; ++i)
  {
    if (name[i] < '0' || name[i] > '9' || value > (2147483647U - (name[i] - '0')) / 10)
    {
      return -1;
    }
    value = value * 10 + name[i] - '0';
  }
  return static_cast<int>(value);
}

void number_name(unsigned value, char *output)
{
  char digits[12];
  unsigned count = 0;
  do
  {
    digits[count++] = '0' + value % 10;
    value /= 10;
  } while (value);
  unsigned size = count;
  for (unsigned i = 0; i < size; ++i)
  {
    output[i] = digits[size - i - 1];
  }
  output[size] = 0;
}

const char *file_kind(const FileInfo &file)
{
  switch (file.kind)
  {
  case FileKind::Node:
    return file.type == T_DIR ? "directory" : file.type == T_DEVICE ? "device" : "file";
  case FileKind::Pipe:
    return "pipe";
  case FileKind::Terminal:
    return "terminal";
  case FileKind::Socket:
    return "socket";
  default:
    return "unknown";
  }
}
} // namespace
namespace flow = xv::flow;

struct ProcFileSystem::GetFlow
{
  struct Context
  {
    ProcFileSystem &owner;
    unsigned kind, pid, descriptor;
    detail::ProcessRecord &process;
    Inode *empty = nullptr, *result = nullptr;
  };

  static bool owner_exists(Context &c) noexcept
  {
    return detail::process_after(c.pid - 1, c.process) &&
           static_cast<unsigned>(c.process.pid) == c.pid;
  }

  static void find(Context &c) noexcept
  {
    for (unsigned i = RootSlots; i < NodeCount; ++i)
    {
      auto &node = c.owner.nodes_[i];
      if (node.ref && node.addrs[0] == c.kind && node.addrs[1] == c.pid &&
          node.addrs[2] == c.descriptor && node.uid_ == c.process.effective_uid &&
          node.gid_ == c.process.effective_gid)
      {
        ++node.ref;
        c.result = &node;
        break;
      }
      if (!node.ref && !c.empty)
      {
        c.empty = &node;
      }
    }
  }

  static bool found(Context &c) noexcept
  {
    return c.result != nullptr;
  }

  static bool available(Context &c) noexcept
  {
    return c.empty != nullptr && c.owner.next_inode_!=0;
  }

  static void initialize(Context &c) noexcept
  {
    auto *node = c.empty;
    node->dev = c.owner.device_;
    node->inum = c.owner.next_inode_++;
    node->uid_ = c.process.effective_uid;
    node->gid_ = c.process.effective_gid;
    node->mode_ = c.kind == 5 || c.kind==FdInfoDirectory ? 0500 : c.kind == 6 || c.kind==FdInfo ? 0400 : c.kind == 3 ? 0555 : 0444;
    if(c.kind==ProcessFile)node->mode_=c.owner.process_providers_.at(c.descriptor).mode;
    node->ref = 1;
    node->valid = node->nlink = 1;
    node->type = c.kind == 3 || c.kind == 5 || c.kind==FdInfoDirectory ? T_DIR : T_FILE;
    node->addrs[0] = c.kind;
    node->addrs[1] = c.pid;
    node->addrs[2] = c.descriptor;
    c.result = node;
  }
  struct Find;
  struct Available;
  struct Initialize;
  using Graph = flow::graph<
      Find, flow::types<flow::node<Find, flow::action<&find>,
                                   flow::branch<flow::condition<&found>, flow::stop,
                                                flow::go<Available>>>,
                        flow::node<Available, flow::noop,
                                   flow::branch<flow::condition<&available>,
                                                flow::go<Initialize>, flow::stop>>,
                        flow::node<Initialize, flow::action<&initialize>, flow::stop>>>;
};

struct ProcFileSystem::LookupFlow
{
  struct Context
  {
    ProcFileSystem &owner;
    Inode *directory;
    const char *name;
    unsigned kind, pid;
    int target = -1, number = -1;
    Inode *result = nullptr;
  };

  static bool self(Context &c) noexcept
  {
    return detail::compare(c.name, ".", DIRSIZ) == 0;
  }

  static void retain_self(Context &c) noexcept
  {
    c.result = c.owner.retain(c.directory);
  }

  static bool parent(Context &c) noexcept
  {
    return detail::compare(c.name, "..", DIRSIZ) == 0;
  }

  static void retain_parent(Context &c) noexcept
  {
    c.result = c.kind == 5 || c.kind==FdInfoDirectory ? c.owner.get(3, c.pid) : c.owner.root();
  }

  static bool root(Context &c) noexcept
  {
    return c.kind == 0;
  }

  static bool provider(Context &c) noexcept
  {
    c.number=c.owner.providers_.find(c.name);return c.number>=0;
  }

  static void retain_provider(Context &c) noexcept
  {
    c.result=c.owner.retain(&c.owner.nodes_[1+unsigned(c.number)]);
  }

  static void target(Context &c) noexcept
  {
    c.target = detail::compare(c.name, "self", DIRSIZ) == 0 ? detail::current_pid()
                                                            : component_number(c.name);
  }

  static bool target_exists(Context &c) noexcept
  {
    detail::ProcessRecord record;
    return c.target >= 1 && detail::process_after(c.target - 1, record) &&
           record.pid == c.target;
  }

  static void retain_process(Context &c) noexcept
  {
    c.result = c.owner.get(3, c.target);
  }

  static bool process(Context &c) noexcept
  {
    return c.kind == 3;
  }

  static bool process_exists(Context &c) noexcept
  {
    detail::ProcessRecord record;
    return detail::process_after(c.pid - 1, record) &&
           static_cast<unsigned>(record.pid) == c.pid;
  }

  static bool status(Context &c) noexcept
  {
    return detail::compare(c.name, "status", DIRSIZ) == 0;
  }

  static void retain_status(Context &c) noexcept
  {
    c.result = c.owner.get(4, c.pid);
  }

  static bool descriptor_name(Context &c) noexcept
  {
    return detail::compare(c.name,"fd",DIRSIZ)==0 || detail::compare(c.name,"fdinfo",DIRSIZ)==0;
  }

  static void retain_descriptors(Context &c) noexcept
  {
    c.result = c.owner.get(detail::compare(c.name,"fdinfo",DIRSIZ)==0 ? FdInfoDirectory:5,c.pid);
  }
  static void process_provider(Context &c) noexcept
  {
    int index=c.owner.process_providers_.find(c.name);
    if(index>=0)c.result=c.owner.get(ProcessFile,c.pid,unsigned(index));
  }

  static bool descriptors(Context &c) noexcept
  {
    return c.kind == 5 || c.kind==FdInfoDirectory;
  }

  static void number(Context &c) noexcept
  {
    c.number = component_number(c.name, true);
  }

  static bool in_range(Context &c) noexcept
  {
    return c.number >= 0 && c.number < NOFILE;
  }

  static void retain_descriptor(Context &c) noexcept
  {
    detail::DescriptorRecord records[NOFILE];
    int count = detail::descriptor_snapshot(c.pid, records, NOFILE);
    for (int i = 0; i < count; ++i)
    {
      if (records[i].descriptor == c.number)
      {
        c.result = c.owner.get(c.kind==FdInfoDirectory ? FdInfo:6,c.pid,c.number);
        break;
      }
    }
  }
  struct Self;
  struct RetainSelf;
  struct Parent;
  struct RetainParent;
  struct Root;
  struct Provider;
  struct RetainProvider;
  struct Target;
  struct RetainProcess;
  struct Process;
  struct ProcessExists;
  struct Status;
  struct RetainStatus;
  struct DescriptorName;
  struct RetainDescriptors;
  struct ProcessProvider;
  struct Descriptors;
  struct Number;
  struct RetainDescriptor;
  using Graph = flow::graph<
      Self,
      flow::types<
          flow::node<Self, flow::noop,
                     flow::branch<flow::condition<&self>, flow::go<RetainSelf>,
                                  flow::go<Parent>>>,
          flow::node<RetainSelf, flow::action<&retain_self>, flow::stop>,
          flow::node<Parent, flow::noop,
                     flow::branch<flow::condition<&parent>, flow::go<RetainParent>,
                                  flow::go<Root>>>,
          flow::node<RetainParent, flow::action<&retain_parent>, flow::stop>,
          flow::node<Root, flow::noop,
                     flow::branch<flow::condition<&root>, flow::go<Provider>,
                                  flow::go<Process>>>,
          flow::node<Provider, flow::noop,
                     flow::branch<flow::condition<&provider>,flow::go<RetainProvider>,flow::go<Target>>>,
          flow::node<RetainProvider,flow::action<&retain_provider>,flow::stop>,
          flow::node<Target, flow::action<&target>,
                     flow::branch<flow::condition<&target_exists>,
                                  flow::go<RetainProcess>, flow::stop>>,
          flow::node<RetainProcess, flow::action<&retain_process>, flow::stop>,
          flow::node<Process, flow::noop,
                     flow::branch<flow::condition<&process>, flow::go<ProcessExists>,
                                  flow::go<Descriptors>>>,
          flow::node<ProcessExists, flow::noop,
                     flow::branch<flow::condition<&process_exists>, flow::go<Status>,
                                  flow::stop>>,
          flow::node<Status, flow::noop,
                     flow::branch<flow::condition<&status>, flow::go<RetainStatus>,
                                  flow::go<DescriptorName>>>,
          flow::node<RetainStatus, flow::action<&retain_status>, flow::stop>,
          flow::node<DescriptorName, flow::noop,
                     flow::branch<flow::condition<&descriptor_name>,
                                  flow::go<RetainDescriptors>, flow::go<ProcessProvider>>>,
          flow::node<ProcessProvider,flow::action<&process_provider>,flow::stop>,
          flow::node<RetainDescriptors, flow::action<&retain_descriptors>, flow::stop>,
          flow::node<Descriptors, flow::noop,
                     flow::branch<flow::condition<&descriptors>, flow::go<Number>,
                                  flow::stop>>,
          flow::node<Number, flow::action<&number>,
                     flow::branch<flow::condition<&in_range>,
                                  flow::go<RetainDescriptor>, flow::stop>>,
          flow::node<RetainDescriptor, flow::action<&retain_descriptor>, flow::stop>>>;
};

struct ProcFileSystem::SnapshotFlow
{
  struct Context
  {
    ProcFileSystem &owner;
    Inode *node;
    PageBuffer &buffer;
    Text &text;
    unsigned kind;
    bool ready = false, result = false;
  };

  static bool provider(Context &c) noexcept {return c.kind==RootFile;}
  static void provider_text(Context &c) noexcept
  {
    c.result=c.owner.providers_.at(c.node->addrs[1]).render(c.buffer);
  }
  static bool process_provider(Context &c) noexcept {return c.kind==ProcessFile;}
  static void process_text(Context &c) noexcept
  {c.result=c.owner.process_providers_.at(c.node->addrs[2]).render(c.node->addrs[1],c.buffer);}

  static bool status(Context &c) noexcept
  {
    return c.kind == 4;
  }

  static void status_text(Context &c) noexcept
  {
    detail::ProcessRecord record;
    unsigned pid = c.node->addrs[1];
    if (!detail::process_after(pid - 1, record) ||
        static_cast<unsigned>(record.pid) != pid)
    {
      return;
    }
    c.text.append("Name: ");
    c.text.append(record.name);
    c.text.append("\nState: ");
    char state[2] = {record.state, 0};
    c.text.append(state);
    c.text.append("\n");
    c.text.field("Uid: ", record.uid);
    c.text.field("Euid: ", record.effective_uid);
    c.text.field("Gid: ", record.gid);
    c.text.field("Egid: ", record.effective_gid);
    c.text.field("Pid: ", record.pid);
    c.text.field("PPid: ", record.parent);
    c.text.field("Pgid: ", record.group > 0 ? record.group : 0);
    c.text.field("Sid: ", record.session > 0 ? record.session : 0);
    c.text.field("VmSize: ", record.bytes / 1024, " kB\n");
    c.ready = true;
  }

  static bool descriptor(Context &c) noexcept
  {
    return c.kind == 6 || c.kind==FdInfo;
  }

  static void descriptor_text(Context &c) noexcept
  {
    detail::DescriptorRecord records[NOFILE];
    int count = detail::descriptor_snapshot(c.node->addrs[1], records, NOFILE);
    const detail::DescriptorRecord *record = nullptr;
    for (int i = 0; i < count; ++i)
    {
      if (records[i].descriptor == static_cast<int>(c.node->addrs[2]))
      {
        record = &records[i];
      }
    }
    if (!record)
    {
      return;
    }
    if(c.kind==FdInfo)
    {
      c.text.field("pos:\t",record->file.position);c.text.append("flags:\t0");c.text.number(record->file.flags,8);
      c.text.append("\n");c.text.field("ino:\t",record->file.inode);c.ready=true;return;
    }
    c.text.field("Descriptor: ", record->descriptor);
    c.text.field("Identity: ", record->file.identity);
    c.text.field("Flags: ", record->file.flags);
    c.text.append("Type: ");
    c.text.append(file_kind(record->file));
    c.text.append("\n");
    c.text.field("Device: ", record->file.device);
    c.text.field("Inode: ", record->file.inode);
    c.ready = true;
  }

  static bool ready(Context &c) noexcept
  {
    return c.ready;
  }

  static void finish_text(Context &c) noexcept
  {
    c.result = c.text.good();
  }

  static bool entry(Context &c, const char *name, unsigned number) noexcept
  {
    dirent value{};
    value.inum = 1 + number % 65535;
    detail::copy_fixed(value.name, name, DIRSIZ);
    return c.buffer.append(&value, sizeof(value));
  }

  static bool self_entry(Context &c) noexcept
  {
    return entry(c, ".", c.node->inum);
  }

  static bool parent_entry(Context &c) noexcept
  {
    return entry(c, "..", 1);
  }

  static bool process_directory(Context &c) noexcept
  {
    return c.kind == 3;
  }

  static bool process_entries(Context &c) noexcept
  {
    detail::ProcessRecord record;
    unsigned pid = c.node->addrs[1];
    if (!detail::process_after(pid - 1, record) ||
        static_cast<unsigned>(record.pid) != pid)
    {
      return false;
    }
    if(!entry(c,"status",c.node->inum+1) || !entry(c,"fd",c.node->inum+2) || !entry(c,"fdinfo",c.node->inum+3))return false;
    for(unsigned i=0;i<c.owner.process_providers_.count();++i)
      if(!entry(c,c.owner.process_providers_.at(i).name,c.node->inum+4+i))return false;
    return true;
  }

  static bool descriptors(Context &c) noexcept
  {
    return c.kind == 5 || c.kind==FdInfoDirectory;
  }

  static bool descriptor_entries(Context &c) noexcept
  {
    detail::DescriptorRecord records[NOFILE];
    int count = detail::descriptor_snapshot(c.node->addrs[1], records, NOFILE);
    if (count < 0)
    {
      return false;
    }
    for (int i = 0; i < count; ++i)
    {
      char name[16];
      number_name(records[i].descriptor, name);
      if (!entry(c, name, c.node->inum + 1 + records[i].descriptor))
      {
        return false;
      }
    }
    return true;
  }

  static bool root_entries(Context &c) noexcept
  {
    if(!entry(c,"self",64))return false;
    for(unsigned i=0;i<c.owner.providers_.count();++i)
      if(!entry(c,c.owner.providers_.at(i).name,i+2))return false;
    int after = 0;
    detail::ProcessRecord record;
    while (detail::process_after(after, record))
    {
      char name[16];
      number_name(record.pid, name);
      if (!entry(c, name, 4 + 2 * (record.pid % 32765)))
      {
        return false;
      }
      after = record.pid;
    }
    return true;
  }

  static void success(Context &c) noexcept
  {
    c.result = true;
  }
  struct Provider;
  struct ProviderText;
  struct ProcessProvider;
  struct ProcessText;
  struct Status;
  struct StatusText;
  struct Descriptor;
  struct DescriptorText;
  struct TextDone;
  struct SelfEntry;
  struct ParentEntry;
  struct ProcessDirectory;
  struct ProcessEntries;
  struct Descriptors;
  struct DescriptorEntries;
  struct RootEntries;
  struct Success;
  using Graph = flow::graph<
      Provider,
      flow::types<
          flow::node<Provider,flow::noop,
                     flow::branch<flow::condition<&provider>,flow::go<ProviderText>,flow::go<ProcessProvider>>>,
          flow::node<ProviderText,flow::action<&provider_text>,flow::stop>,
          flow::node<ProcessProvider,flow::noop,flow::branch<flow::condition<&process_provider>,flow::go<ProcessText>,flow::go<Status>>>,
          flow::node<ProcessText,flow::action<&process_text>,flow::stop>,
          flow::node<Status, flow::noop,
                     flow::branch<flow::condition<&status>, flow::go<StatusText>,
                                  flow::go<Descriptor>>>,
          flow::node<
              StatusText, flow::action<&status_text>,
              flow::branch<flow::condition<&ready>, flow::go<TextDone>, flow::stop>>,
          flow::node<Descriptor, flow::noop,
                     flow::branch<flow::condition<&descriptor>,
                                  flow::go<DescriptorText>, flow::go<SelfEntry>>>,
          flow::node<
              DescriptorText, flow::action<&descriptor_text>,
              flow::branch<flow::condition<&ready>, flow::go<TextDone>, flow::stop>>,
          flow::node<TextDone, flow::action<&finish_text>, flow::stop>,
          flow::node<SelfEntry, flow::noop,
                     flow::branch<flow::condition<&self_entry>, flow::go<ParentEntry>,
                                  flow::stop>>,
          flow::node<ParentEntry, flow::noop,
                     flow::branch<flow::condition<&parent_entry>,
                                  flow::go<ProcessDirectory>, flow::stop>>,
          flow::node<ProcessDirectory, flow::noop,
                     flow::branch<flow::condition<&process_directory>,
                                  flow::go<ProcessEntries>, flow::go<Descriptors>>>,
          flow::node<ProcessEntries, flow::noop,
                     flow::branch<flow::condition<&process_entries>, flow::go<Success>,
                                  flow::stop>>,
          flow::node<Descriptors, flow::noop,
                     flow::branch<flow::condition<&descriptors>,
                                  flow::go<DescriptorEntries>, flow::go<RootEntries>>>,
          flow::node<DescriptorEntries, flow::noop,
                     flow::branch<flow::condition<&descriptor_entries>,
                                  flow::go<Success>, flow::stop>>,
          flow::node<RootEntries, flow::noop,
                     flow::branch<flow::condition<&root_entries>, flow::go<Success>,
                                  flow::stop>>,
          flow::node<Success, flow::action<&success>, flow::stop>>>;
};

void ProcFileSystem::initialize_inodes()
{
  references_.init("procfs refs");
  for (auto &node : nodes_)
  {
    node.owner_ = this;
    node.lock.init("procfs inode");
  }
}

void ProcFileSystem::initialize(int device)
{
  device_ = device;
  install_proc_providers(*this);providers_.seal();process_providers_.seal();
  for (unsigned i = 0; i <= providers_.count(); ++i)
  {
    auto &node = nodes_[i];
    node.dev = device_;
    node.inum = i + 1;
    node.valid = node.nlink = 1;
    node.type = i == 0 ? T_DIR : T_FILE;
    node.addrs[0] = i ? RootFile:0;
    node.addrs[1] = i ? i-1:0;
    node.mode_ = i == 0 ? 0555 : providers_.at(i-1).mode;
  }
}

Inode *ProcFileSystem::get(unsigned kind, unsigned pid, unsigned descriptor)
{
  detail::ProcessRecord process;
  GetFlow::Context context{*this, kind, pid, descriptor, process};
  if (!GetFlow::owner_exists(context))
  {
    return nullptr;
  }
  xv::sync::ScopedLock guard(references_);
  (void)GetFlow::Graph::run<0>(context);
  return context.result;
}

Inode *ProcFileSystem::retain(Inode *node)
{
  xv::sync::ScopedLock guard(references_);
  ++node->ref;
  return node;
}

void ProcFileSystem::release(Inode *node)
{
  xv::sync::ScopedLock guard(references_);
  if (node->ref < 1)
  {
    detail::fail("procfs: release dead reference");
  }
  --node->ref;
}

void ProcFileSystem::lock(Inode *node)
{
  node->lock.acquire();
}

void ProcFileSystem::unlock(Inode *node)
{
  node->lock.release();
}

void ProcFileSystem::unlock_release(Inode *node)
{
  unlock(node);
  release(node);
}

void ProcFileSystem::update(Inode *)
{
  detail::fail("procfs: immutable metadata");
}

void ProcFileSystem::truncate(Inode *)
{
  detail::fail("procfs: truncate");
}

void ProcFileSystem::stat(Inode *node, Stat *result)
{
  *result = {};
  result->dev = node->dev;
  result->ino = node->inum;
  result->type = node->type;
  result->nlink = 1;
  node->security_stat(result);
}

Inode *ProcFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *)
{
  LookupFlow::Context context{*this, directory, name, directory->addrs[0],
                              directory->addrs[1]};
  (void)LookupFlow::Graph::run<0>(context);
  return context.result;
}

bool ProcFileSystem::snapshot(Inode *node, PageBuffer &buffer)
{
  Text text(buffer);
  SnapshotFlow::Context context{*this, node, buffer, text, node->addrs[0]};
  (void)SnapshotFlow::Graph::run<0>(context);
  return context.result;
}

int ProcFileSystem::read(Inode *node, int user, unsigned long destination,
                         unsigned offset, unsigned count)
{
  PageBuffer contents;
  if (!snapshot(node, contents))
  {
    return -1;
  }
  return contents.read(offset, user, destination, count);
}
} // namespace xv::fs
