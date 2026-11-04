#include "kernel/fs/stat.h"
#include "user/runtime/user.h"
#include "kernel/fs/userfs/protocol.h"
import xv.userfs;


namespace
{
using namespace xv::userfs;
constexpr unsigned Nodes = 128, Entries = 256, MaximumFile = 1024 * 1024;

struct Node
{
  Attributes attributes;
  unsigned parent = 1, capacity = 0;
  bool referenced = false;
  unsigned char *data = nullptr;
};

struct Entry
{
  unsigned directory = 0, node = 0;
  char name[NameSize]{};
};

Node nodes[Nodes];
Entry entries[Entries];
unsigned allocated = 1;
Message message;

bool valid_name(const char *name)
{
  if (!*name || !strcmp(name, ".") || !strcmp(name, ".."))
  {
    return false;
  }
  for (unsigned i = 0; i < NameSize; ++i)
  {
    if (!name[i])
    {
      return true;
    }
    if (name[i] == '/')
    {
      return false;
    }
  }
  return false;
}

Entry *find(unsigned directory, const char *name)
{
  for (auto &entry : entries)
  {
    if (entry.directory == directory && !strcmp(entry.name, name))
    {
      return &entry;
    }
  }
  return nullptr;
}

Entry *space()
{
  for (auto &entry : entries)
  {
    if (!entry.node)
    {
      return &entry;
    }
  }
  return nullptr;
}

bool empty(unsigned number)
{
  for (auto &entry : entries)
  {
    if (entry.directory == number)
    {
      return false;
    }
  }
  return true;
}

bool resize(Node &node, unsigned size)
{
  if (size > MaximumFile)
  {
    return false;
  }
  if (size > node.capacity)
  {
    unsigned capacity = (size + 4095) & ~4095U;
    auto *next = static_cast<unsigned char *>(malloc(capacity));
    if (!next)
    {
      return false;
    }
    memset(next, 0, capacity);
    if (node.attributes.size)
    {
      memmove(next, node.data, node.attributes.size);
    }
    if (node.data)
    {
      free(node.data);
    }
    node.data = next;
    node.capacity = capacity;
  }
  if (size < node.attributes.size)
  {
    memset(node.data + size, 0, node.attributes.size - size);
  }
  node.attributes.size = size;
  return true;
}

void insert(Entry &entry, unsigned directory, unsigned node, const char *name)
{
  entry.directory = directory;
  entry.node = node;
  strcpy(entry.name, name);
}

void reclaim(unsigned number)
{
  auto &node = nodes[number - 1];
  if (!node.attributes.links && !node.referenced)
  {
    if (node.data)
    {
      free(node.data);
    }
    node = {};
  }
}

void process(Message &m)
{
  m.result = -1;
  if (m.version != Version || !m.node || m.node > allocated)
  {
    return;
  }
  auto &node = nodes[m.node - 1];
  if (node.attributes.node != m.node)
  {
    return;
  }
  bool directory = node.attributes.type == T_DIR;
  switch (m.operation)
  {
  case Operation::Lookup:
  {
    if (!directory || m.name[NameSize - 1])
    {
      return;
    }
    unsigned number = 0;
    if (!strcmp(m.name, "."))
    {
      number = m.node;
    }
    else if (!strcmp(m.name, ".."))
    {
      number = node.parent;
    }
    else if (auto *entry = find(m.node, m.name))
    {
      number = entry->node;
    }
    if (!number)
    {
      return;
    }
    nodes[number - 1].referenced = true;
    m.attributes = nodes[number - 1].attributes;
    m.result = 0;
    return;
  }
  case Operation::Create:
  {
    if (!directory || !valid_name(m.name) || find(m.node, m.name))
    {
      return;
    }
    auto *entry = space();
    if (!entry || (m.attributes.type != T_DIR && m.attributes.type != T_FILE &&
                   m.attributes.type != T_SYMLINK))
    {
      return;
    }
    unsigned number = 2;
    while (number <= Nodes && nodes[number - 1].attributes.node)
    {
      ++number;
    }
    if (number > Nodes)
    {
      return;
    }
    if (number > allocated)
    {
      allocated = number;
    }
    auto &created = nodes[number - 1];
    created.referenced = true;
    created.attributes = m.attributes;
    created.attributes.node = number;
    created.attributes.size = 0;
    created.attributes.links = m.attributes.type == T_DIR ? 2 : 1;
    created.parent = m.node;
    if (created.attributes.type == T_DIR)
    {
      ++node.attributes.links;
    }
    insert(*entry, m.node, number, m.name);
    m.attributes = created.attributes;
    m.result = 0;
    return;
  }
  case Operation::Read:
  {
    if (directory || m.count > DataSize || m.offset > MaximumFile)
    {
      return;
    }
    unsigned count =
        m.offset < node.attributes.size ? node.attributes.size - m.offset : 0;
    if (count > m.count)
    {
      count = m.count;
    }
    if (count)
    {
      memmove(m.data, node.data + m.offset, count);
    }
    m.count = count;
    m.result = count;
    return;
  }
  case Operation::Write:
    if (directory || m.count > DataSize || m.offset > MaximumFile ||
        m.count > MaximumFile - m.offset)
    {
      return;
    }
    if (m.offset + m.count > node.attributes.size && !resize(node, m.offset + m.count))
    {
      return;
    }
    memmove(node.data + m.offset, m.data, m.count);
    m.result = m.count;
    return;
  case Operation::Resize:
    if (directory || !resize(node, m.attributes.size))
    {
      return;
    }
    m.result = 0;
    return;
  case Operation::Setattr:
    node.attributes.uid = m.attributes.uid;
    node.attributes.gid = m.attributes.gid;
    node.attributes.mode = m.attributes.mode & 07777;
    m.result = 0;
    return;
  case Operation::Readdir:
  {
    if (!directory)
    {
      return;
    }
    unsigned number = 0;
    if (m.offset == 0)
    {
      number = m.node;
      strcpy(m.name, ".");
      m.offset = 1;
    }
    else if (m.offset == 1)
    {
      number = node.parent;
      strcpy(m.name, "..");
      m.offset = 2;
    }
    else
    {
      unsigned i = m.offset - 2;
      for (; i < Entries; ++i)
      {
        if (entries[i].directory == m.node)
        {
          number = entries[i].node;
          strcpy(m.name, entries[i].name);
          m.offset = i + 3;
          break;
        }
      }
    }
    m.result = number ? 1 : 0;
    if (number)
    {
      m.attributes = nodes[number - 1].attributes;
    }
    return;
  }
  case Operation::Link:
  {
    if (!directory || !valid_name(m.name) || find(m.node, m.name) || !m.other ||
        m.other > allocated || nodes[m.other - 1].attributes.type == T_DIR)
    {
      return;
    }
    auto *entry = space();
    if (!entry)
    {
      return;
    }
    insert(*entry, m.node, m.other, m.name);
    ++nodes[m.other - 1].attributes.links;
    m.result = 0;
    return;
  }
  case Operation::Unlink:
  {
    if (!directory || !valid_name(m.name))
    {
      return;
    }
    auto *entry = find(m.node, m.name);
    if (!entry)
    {
      return;
    }
    auto &target = nodes[entry->node - 1];
    if (target.attributes.type == T_DIR && !empty(entry->node))
    {
      return;
    }
    if (target.attributes.type == T_DIR)
    {
      target.attributes.links = 0;
      --node.attributes.links;
    }
    else
    {
      --target.attributes.links;
    }
    m.attributes = target.attributes;
    reclaim(entry->node);
    *entry = {};
    m.result = 0;
    return;
  }
  case Operation::Rename:
  {
    if (!directory || !valid_name(m.name) || !valid_name(m.target) || !m.other ||
        m.other > allocated || nodes[m.other - 1].attributes.type != T_DIR)
    {
      return;
    }
    auto *old = find(m.node, m.name);
    if (!old)
    {
      return;
    }
    auto &source = nodes[old->node - 1];
    bool dir = source.attributes.type == T_DIR;
    if (dir)
    {
      for (unsigned at = m.other;; at = nodes[at - 1].parent)
      {
        if (at == old->node)
        {
          return;
        }
        if (at == 1)
        {
          break;
        }
      }
    }
    auto *next = find(m.other, m.target);
    if (next)
    {
      if (next->node == old->node)
      {
        m.result = 0;
        return;
      }
      auto &target = nodes[next->node - 1];
      if ((target.attributes.type == T_DIR) != dir || (dir && !empty(next->node)))
      {
        return;
      }
      if (dir)
      {
        target.attributes.links = 0;
        --nodes[m.other - 1].attributes.links;
      }
      else
      {
        --target.attributes.links;
      }
      reclaim(next->node);
      *next = {};
    }
    insert(*old, m.other, old->node, m.target);
    if (dir && source.parent != m.other)
    {
      --nodes[source.parent - 1].attributes.links;
      ++nodes[m.other - 1].attributes.links;
      source.parent = m.other;
    }
    m.result = 0;
    return;
  }
  case Operation::Forget:
    node.referenced = false;
    reclaim(m.node);
    m.result = 0;
    return;
  case Operation::Getattr:
    m.attributes = node.attributes;
    m.result = 0;
    return;
  case Operation::SnapshotBegin:
  case Operation::SnapshotRead:
  case Operation::SnapshotEnd:
    return;
  case Operation::Sync:
    m.result = 0;
    return;
  }
}

class MemBackend final : public userfs::Backend
{
  unsigned maximum_;

  Message &begin(Operation operation, unsigned node)
  {
    message = {};
    message.operation = operation;
    message.node = node;
    return message;
  }

public:

  explicit MemBackend(unsigned maximum) : maximum_(maximum)
  {
  }

  Attributes root() override
  {
    return nodes[0].attributes;
  }

  int lookup(unsigned node, const char *name, Attributes &out) override
  {
    auto &m = begin(Operation::Lookup, node);
    strcpy(m.name, name);
    process(m);
    out = m.attributes;
    return m.result;
  }

  int getattr(unsigned node, Attributes &out) override
  {
    auto &m = begin(Operation::Getattr, node);
    process(m);
    out = m.attributes;
    return m.result;
  }

  int readdir(unsigned node, unsigned long offset, userfs::Entry &out) override
  {
    auto &m = begin(Operation::Readdir, node);
    m.offset = offset;
    process(m);
    out.attributes = m.attributes;
    out.next = m.offset;
    strcpy(out.name, m.name);
    return m.result;
  }

  int read(unsigned node, unsigned long offset, void *out, unsigned count) override
  {
    if (count > maximum_)
    {
      return -1;
    }
    auto &m = begin(Operation::Read, node);
    m.offset = offset;
    m.count = count;
    process(m);
    if (m.result > 0)
    {
      memmove(out, m.data, m.result);
    }
    return m.result;
  }

  int write(unsigned node, unsigned long offset, const void *in,
            unsigned count) override
  {
    if (count > maximum_)
    {
      return -1;
    }
    auto &m = begin(Operation::Write, node);
    m.offset = offset;
    m.count = count;
    memmove(m.data, in, count);
    process(m);
    return m.result;
  }

  int create(unsigned node, const char *name, const Attributes &requested,
             Attributes &out) override
  {
    auto &m = begin(Operation::Create, node);
    strcpy(m.name, name);
    m.attributes = requested;
    process(m);
    out = m.attributes;
    return m.result;
  }

  int resize(unsigned node, unsigned size) override
  {
    auto &m = begin(Operation::Resize, node);
    m.attributes.size = size;
    process(m);
    return m.result;
  }

  int setattr(unsigned node, const Attributes &attributes) override
  {
    auto &m = begin(Operation::Setattr, node);
    m.attributes = attributes;
    process(m);
    return m.result;
  }

  int link(unsigned node, const char *name, unsigned target) override
  {
    auto &m = begin(Operation::Link, node);
    strcpy(m.name, name);
    m.other = target;
    process(m);
    return m.result;
  }

  int unlink(unsigned node, const char *name, Attributes &out) override
  {
    auto &m = begin(Operation::Unlink, node);
    strcpy(m.name, name);
    process(m);
    out = m.attributes;
    return m.result;
  }

  int rename(unsigned node, const char *name, unsigned to, const char *next) override
  {
    auto &m = begin(Operation::Rename, node);
    strcpy(m.name, name);
    strcpy(m.target, next);
    m.other = to;
    process(m);
    return m.result;
  }

  void forget(unsigned node) override
  {
    auto &m = begin(Operation::Forget, node);
    process(m);
  }
};
} // namespace


extern "C" int main(int argc, char **argv)
{
  bool registered = (argc == 3 || argc == 4) && !strcmp(argv[1], "--register");
  if (!registered && argc != 2)
  {
    fprintf(2, "usage: memfsd /mountpoint | memfsd --register type [max-payload]\n");
    exit(1);
  }
  nodes[0].attributes = {1, 0, 0, 0, 0755, T_DIR, 2};
  unsigned maximum = registered && argc == 4 ? unsigned(atoi(argv[3])) : DataSize;
  MemBackend backend(maximum);
  int fd = registered ? userfs::register_backend(argv[2], backend, maximum)
                      : fsserve(argv[1]);
  if (fd < 0)
  {
    fprintf(2, "memfsd: registration or mount failed\n");
    exit(1);
  }
  printf("memfsd: %s %s pid=%d\n", registered ? "registered" : "serving",
         registered ? argv[2] : argv[1], getpid());
  int result = userfs::serve(fd, backend);
  close(fd);
  exit(result < 0);
}
