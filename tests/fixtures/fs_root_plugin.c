#include <xvfs/plugin.h>
#include "init_image.h"
struct entry { unsigned parent, node; const char *name; };
static const struct entry children[] = {
  {1, 2, "sbin"}, {1, 4, "dev"}, {1, 5, "proc"}, {1, 6, "tmp"}, {2, 3, "init"}
};
static int equal(const char *a, const char *b)
{ while (*a && *a == *b) { ++a; ++b; } return *a == *b; }
static void copy(char *out, const char *in) { while ((*out++ = *in++)) {} }
static int directory(unsigned node) { return node >= 1 && node <= 6 && node != 3; }
static int entry(unsigned directory_node, unsigned index, unsigned *node, const char **name)
{
  if (!directory(directory_node)) return 0;
  if (index < 2) { *node = index ? 1 : directory_node; *name = index ? ".." : "."; return 1; }
  index -= 2;
  for (unsigned i = 0; i < sizeof(children) / sizeof(children[0]); ++i)
    if (children[i].parent == directory_node && !index--)
    { *node = children[i].node; *name = children[i].name; return 1; }
  return 0;
}
static unsigned count(unsigned node)
{
  unsigned n = 0, child;
  const char *name;
  while (entry(node, n, &child, &name)) ++n;
  return n;
}
static struct xvfs_attributes attributes(unsigned node)
{
  struct xvfs_attributes out = {node, node == 3 ? sizeof(init_image) : count(node) * 16,
    0, 0, 0555, node == 3 ? 2 : 1, node == 3 ? 1 : node == 1 ? 6 : 2, 0, 0};
  return out;
}
int main(int argc, char **argv)
{
  if (argc != 6 || !equal(argv[1], "--fs-plugin") || !equal(argv[2], "1")) return 2;
  xvfs_close(XVFS_IMAGE_FD); xvfs_close(XVFS_SOURCE_FD); xvfs_close(XVFS_RESOURCES_FD);
  struct xvfs_registration registration = {0};
  registration.size = sizeof(registration); registration.version = XVFS_WIRE_VERSION;
  registration.flags = XVFS_READ_ONLY; registration.max_payload = XVFS_DATA_SIZE;
  registration.operations = XVFS_READ_OPERATIONS; registration.root = attributes(1);
  copy(registration.type, argv[3]);
  int service = xvfs_register(&registration);
  if (service < 0 || xvfs_write(XVFS_READY_FD, "R", 1) != 1) return 1;
  xvfs_close(XVFS_READY_FD);
  char approved;
  if (xvfs_read(XVFS_ACTIVATION_FD, &approved, 1) != 1 || approved != 'G') return 1;
  xvfs_close(XVFS_ACTIVATION_FD);
  struct xvfs_message message;
  while (xvfs_read(service, &message, sizeof(message)) == sizeof(message))
  {
    message.result = -1;
    switch (message.operation)
    {
    case XVFS_LOOKUP:
      for (unsigned i = 0, node;; ++i)
      {
        const char *name;
        if (!entry(message.node, i, &node, &name)) break;
        if (equal(name, message.name)) { message.attributes = attributes(node); message.result = 0; break; }
      }
      break;
    case XVFS_GETATTR:
      if (message.node >= 1 && message.node <= 6)
      { message.attributes = attributes(message.node); message.result = 0; }
      break;
    case XVFS_READDIR:
      if (directory(message.node) && message.offset <= count(message.node))
      {
        unsigned node;
        const char *name;
        message.result = 0;
        if (entry(message.node, message.offset, &node, &name))
        { copy(message.name, name); message.attributes = attributes(node); ++message.offset; message.result = 1; }
      }
      break;
    case XVFS_READ:
      if (message.count > XVFS_DATA_SIZE || !message.node || message.node > 6) break;
      {
        unsigned size = message.node == 3 ? sizeof(init_image) : count(message.node) * 16;
        if (message.offset > size) break;
        unsigned take = message.count;
        if (take > size - message.offset) take = size - message.offset;
        for (unsigned i = 0; i < take; ++i)
        {
          unsigned position = message.offset + i;
          if (message.node == 3) message.data[i] = init_image[position];
          else
          {
            unsigned node;
            const char *name;
            if (!entry(message.node, position / 16, &node, &name)) return 1;
            unsigned part = position % 16;
            if (part < 2) message.data[i] = (node >> (part * 8)) & 255;
            else
            {
              unsigned index = 0;
              while (name[index] && index < part - 2) ++index;
              message.data[i] = index == part - 2 ? name[index] : 0;
            }
          }
        }
        message.count = take; message.result = take;
      }
      break;
    case XVFS_FORGET:
    case XVFS_SYNC: message.result = 0; break;
    }
    if (xvfs_write(service, &message, sizeof(message)) != sizeof(message)) return 1;
  }
  return 0;
}
