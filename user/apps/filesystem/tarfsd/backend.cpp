#include "application.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "kernel/fs/userfs/protocol.h"
import xv.userfs;

#include "archive.h"

namespace tarfs
{
class TarBackend final : public userfs::Backend
{
  int image_;

public:

  explicit TarBackend(int image) : image_(image) {
  }

  unsigned
  flags() override {
    return xv::userfs::ReadOnly;
  }

  unsigned long
  operations() override {
    return xv::userfs::ReadOperations;
  }

  userfs::Attributes
  root() override {
    return nodes[0].attributes;
  }

  int
  getattr(unsigned node, userfs::Attributes &out) override {
    if (!node || node > used)
    {
      return -1;
    }
    out = nodes[node - 1].attributes;
    return 0;
  }

  int
  lookup(unsigned node, const char *name, userfs::Attributes &out) override {
    if (!node || node > used || nodes[node - 1].attributes.type != T_DIR)
    {
      return -1;
    }
    unsigned found = !strcmp(name, ".")    ? node
                     : !strcmp(name, "..") ? nodes[node - 1].parent
                                           : child(node, name);
    return found ? getattr(found, out) : -1;
  }

  int
  readdir(unsigned node, unsigned long offset, userfs::Entry &entry) override {
    if (!node || node > used || nodes[node - 1].attributes.type != T_DIR)
    {
      return -1;
    }
    unsigned found = 0;
    if (!offset)
    {
      found = node;
      strcpy(entry.name, ".");
      entry.next = 1;
    }
    else if (offset == 1)
    {
      found = nodes[node - 1].parent;
      strcpy(entry.name, "..");
      entry.next = 2;
    }
    else
    {
      if (offset - 2 > used)
      {
        return 0;
      }
      for (unsigned i = offset - 2; i < used; ++i)
      {
        if (i && nodes[i].parent == node)
        {
          found = i + 1;
          strcpy(entry.name, nodes[i].name);
          entry.next = i + 3;
          break;
        }
      }
    }
    if (!found)
    {
      return 0;
    }
    entry.attributes = nodes[found - 1].attributes;
    return 1;
  }

  int
  read(unsigned number, unsigned long offset, void *output, unsigned maximum) override {
    if (!number || number > used)
    {
      return -1;
    }
    auto &node = nodes[number - 1];
    if (node.attributes.type == T_DIR)
    {
      return -1;
    }
    if (offset >= node.attributes.size)
    {
      return 0;
    }
    unsigned count = node.attributes.size - offset;
    if (count > maximum)
    {
      count = maximum;
    }
    if (node.attributes.type == T_SYMLINK)
    {
      memmove(output, node.link + offset, count);
      return count;
    }
    if (lseek(image_, node.offset + offset, SEEK_SET) < 0)
    {
      return -1;
    }
    return ::read(image_, output, count);
  }
};

int
serve(int image, const char *type) {
  TarBackend backend(image);
  int service = userfs::register_backend(type, backend);
  if (service < 0)
  {
    return 1;
  }
  printf("tarfsd: registered %s pid=%d\n", type, getpid());
  int result = userfs::serve(service, backend);
  close(service);
  return result < 0;
}
} // namespace tarfs
