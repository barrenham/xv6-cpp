#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "kernel/fs/userfs/protocol.h"
import xv.userfs;


namespace
{
constexpr unsigned Capacity = 128;

struct Node
{
  userfs::Attributes attributes;
  unsigned parent = 1;
  unsigned long offset = 0;
  char name[256]{}, link[101]{};
};

Node nodes[Capacity];
unsigned used = 1;
unsigned char header[512];

bool number(const unsigned char *text, unsigned length, unsigned long &value)
{
  value = 0;
  bool ended = false, started = false;
  for (unsigned i = 0; i < length; ++i)
  {
    unsigned char c = text[i];
    if (c == 0 || c == ' ')
    {
      if (started)
      {
        ended = true;
      }
      continue;
    }
    if (ended || c < '0' || c > '7' || value > (~0UL >> 3))
    {
      return false;
    }
    started = true;
    value = value * 8 + c - '0';
  }
  return true;
}

unsigned child(unsigned parent, const char *name)
{
  for (unsigned i = 1; i < used; ++i)
  {
    if (nodes[i].parent == parent && !strcmp(nodes[i].name, name))
    {
      return i + 1;
    }
  }
  return 0;
}

unsigned path(char *name, unsigned type)
{
  while (name[0] == '.' && name[1] == '/')
  {
    name += 2;
  }
  if (!*name)
  {
    return type == T_DIR ? 1 : 0;
  }
  if (*name == '/')
  {
    return 0;
  }
  unsigned parent = 1;
  char component[256];
  while (*name)
  {
    unsigned length = 0;
    while (*name && *name != '/')
    {
      component[length++] = *name++;
    }
    component[length] = 0;
    if (!length || !strcmp(component, ".") || !strcmp(component, ".."))
    {
      return 0;
    }
    if (*name == '/')
    {
      ++name;
    }
    unsigned wanted = *name ? T_DIR : type;
    unsigned found = child(parent, component);
    if (!found)
    {
      if (used == Capacity)
      {
        return 0;
      }
      found = ++used;
      auto &node = nodes[found - 1];
      node.parent = parent;
      node.attributes = {found,
                         0,
                         0,
                         0,
                         wanted == T_DIR ? 0555U : 0444U,
                         wanted,
                         wanted == T_DIR ? 2U : 1U};
      strcpy(node.name, component);
    }
    if (nodes[found - 1].attributes.type != wanted)
    {
      return 0;
    }
    parent = found;
  }
  return parent;
}

class TarBackend final : public userfs::Backend
{
  int image_;

public:

  explicit TarBackend(int image) : image_(image)
  {
  }

  unsigned flags() override
  {
    return xv::userfs::ReadOnly;
  }

  unsigned long operations() override
  {
    return xv::userfs::ReadOperations;
  }

  userfs::Attributes root() override
  {
    return nodes[0].attributes;
  }

  int getattr(unsigned node, userfs::Attributes &out) override
  {
    if (!node || node > used)
    {
      return -1;
    }
    out = nodes[node - 1].attributes;
    return 0;
  }

  int lookup(unsigned node, const char *name, userfs::Attributes &out) override
  {
    if (!node || node > used || nodes[node - 1].attributes.type != T_DIR)
    {
      return -1;
    }
    unsigned found = !strcmp(name, ".")    ? node
                     : !strcmp(name, "..") ? nodes[node - 1].parent
                                           : child(node, name);
    return found ? getattr(found, out) : -1;
  }

  int readdir(unsigned node, unsigned long offset, userfs::Entry &entry) override
  {
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

  int read(unsigned number, unsigned long offset, void *output,
           unsigned maximum) override
  {
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

bool load(int image)
{
  struct stat st{};
  if (fstat(image, &st) || st.type != T_FILE)
  {
    return false;
  }
  nodes[0].attributes = {1, 0, 0, 0, 0555, T_DIR, 2};
  for (unsigned long at = 0; at + 512 <= st.size;)
  {
    if (lseek(image, at, SEEK_SET) < 0 || ::read(image, header, 512) != 512)
    {
      return false;
    }
    bool zero = true;
    for (unsigned char c : header)
    {
      if (c)
      {
        zero = false;
      }
    }
    if (zero)
    {
      return true;
    }
    unsigned long stored, size, mode;
    if (!number(header + 148, 8, stored) || !number(header + 124, 12, size) ||
        !number(header + 100, 8, mode) || size > 0xffffffffUL || at + 512 > st.size ||
        size > st.size - at - 512)
    {
      return false;
    }
    unsigned checksum = 0;
    for (unsigned i = 0; i < 512; ++i)
    {
      checksum += i >= 148 && i < 156 ? ' ' : header[i];
    }
    if (checksum != stored)
    {
      return false;
    }
    if (memcmp(header + 257, "ustar", 5))
    {
      return false;
    }
    char name[256]{};
    unsigned n = 0;
    while (n < 155 && header[345 + n])
    {
      name[n] = header[345 + n];
      ++n;
    }
    if (n)
    {
      name[n++] = '/';
    }
    for (unsigned i = 0; i < 100 && header[i]; ++i)
    {
      if (n == 255)
      {
        return false;
      }
      name[n++] = header[i];
    }
    unsigned type = header[156] == '5'                   ? T_DIR
                    : header[156] == '2'                 ? T_SYMLINK
                    : header[156] == '0' || !header[156] ? T_FILE
                                                         : 0;
    if (!type)
    {
      return false;
    }
    unsigned found = path(name, type);
    if (!found)
    {
      return false;
    }
    auto &node = nodes[found - 1];
    node.offset = at + 512;
    node.attributes.mode = mode & 0555;
    node.attributes.size = type == T_FILE ? size : 0;
    if (type == T_SYMLINK)
    {
      unsigned i = 0;
      for (; i < 100 && header[157 + i]; ++i)
      {
        node.link[i] = header[157 + i];
      }
      node.link[i] = 0;
      node.attributes.size = i;
    }
    at += 512 + ((size + 511) & ~511UL);
  }
  return false;
}
} // namespace


extern "C" int main(int argc, char **argv)
{
  if (argc != 3)
  {
    fprintf(2, "usage: tarfsd archive.tar registration-type\n");
    exit(1);
  }
  int image = open(argv[1], O_RDONLY);
  if (image < 0 || !load(image))
  {
    fprintf(2, "tarfsd: invalid or unsupported ustar archive\n");
    exit(1);
  }
  TarBackend backend(image);
  int service = userfs::register_backend(argv[2], backend);
  if (service < 0)
  {
    fprintf(2, "tarfsd: registration failed\n");
    exit(1);
  }
  printf("tarfsd: registered %s source=%s pid=%d\n", argv[2], argv[1], getpid());
  int result = userfs::serve(service, backend);
  close(service);
  close(image);
  exit(result < 0);
}
