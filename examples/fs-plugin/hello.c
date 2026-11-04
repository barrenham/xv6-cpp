#include <xvfs/plugin.h>

static int equal(const char *a, const char *b)
{
  while (*a && *a == *b) { ++a; ++b; }
  return *a == *b;
}
static unsigned length(const char *text)
{
  unsigned n = 0;
  while (text[n]) ++n;
  return n;
}
static void text(char *out, const char *in)
{
  while ((*out++ = *in++)) {}
}
static int writable;
static struct xvfs_attributes attributes(unsigned node, unsigned size)
{
  struct xvfs_attributes out = {node, node == 1 ? 0 : size, 0, 0,
    node == 1 ? 0555u : writable ? 0644u : 0444u, node == 1 ? 1u : 2u, node == 1 ? 2u : 1u, 0, 0};
  return out;
}
int main(int argc, char **argv)
{
  if (argc != 6 || !equal(argv[1], "--fs-plugin") || !equal(argv[2], "1")) return 2;
  if (equal(argv[5], "slowstart")) (void)xvfs_syscall(49, 0, 0, 35000);
  unsigned name_size = length(argv[3]);
  if (!name_size || name_size >= 32) return 2;
  xvfs_close(XVFS_IMAGE_FD);
  xvfs_close(XVFS_RESOURCES_FD);
  const char *content = *argv[5] ? argv[5] : "hello from an independently built ELF\n";
  long source_size = xvfs_seek(XVFS_SOURCE_FD, 0, 2);
  if (source_size < 0) xvfs_close(XVFS_SOURCE_FD);
  if (source_size > UINT32_MAX) return 1;
  writable = source_size >= 0 && equal(argv[5], "rw");
  unsigned size = source_size >= 0 ? (unsigned)source_size : length(content);
  struct xvfs_registration registration = {0};
  registration.size = sizeof(registration);
  registration.version = XVFS_WIRE_VERSION;
  registration.flags = writable ? 0 : XVFS_READ_ONLY;
  registration.max_payload = XVFS_DATA_SIZE;
  registration.operations = XVFS_READ_OPERATIONS | (writable ? XVFS_OP_BIT(XVFS_WRITE) : 0);
  registration.root = attributes(1, 0);
  text(registration.type, argv[3]);
  int service = xvfs_register(&registration);
  if (service < 0 || xvfs_write(XVFS_READY_FD, "R", 1) != 1) return 1;
  xvfs_close(XVFS_READY_FD);
  char approved = 0;
  if (xvfs_read(XVFS_ACTIVATION_FD, &approved, 1) != 1 || approved != 'G') return 1;
  xvfs_close(XVFS_ACTIVATION_FD);
  struct xvfs_message message;
  int received;
  while ((received = xvfs_read(service, &message, sizeof(message))) == sizeof(message))
  {
    message.result = -1;
    switch (message.operation)
    {
    case XVFS_LOOKUP:
      if (message.node == 1)
      {
        unsigned node = equal(message.name, "hello") ? 2 :
                        equal(message.name, ".") || equal(message.name, "..") ? 1 : 0;
        if (node) { message.attributes = attributes(node, size); message.result = 0; }
      }
      break;
    case XVFS_GETATTR:
      if (message.node == 1 || message.node == 2)
      { message.attributes = attributes(message.node, size); message.result = 0; }
      break;
    case XVFS_READDIR:
      if (message.node == 1 && message.offset <= 3)
      {
        message.result = 0;
        if (message.offset < 3)
        {
          unsigned node = message.offset == 2 ? 2 : 1;
          text(message.name, message.offset == 0 ? "." : message.offset == 1 ? ".." : "hello");
          message.attributes = attributes(node, size);
          ++message.offset;
          message.result = 1;
        }
      }
      break;
    case XVFS_READ:
      if (message.node == 2 && message.offset <= size && message.count <= XVFS_DATA_SIZE)
      {
        unsigned take = message.count;
        if (take > size - message.offset) take = size - message.offset;
        if (source_size >= 0)
        {
          if (xvfs_seek(XVFS_SOURCE_FD, message.offset, 0) < 0) break;
          message.result = xvfs_read(XVFS_SOURCE_FD, message.data, take);
        }
        else
        {
          for (unsigned i = 0; i < take; ++i) message.data[i] = content[message.offset + i];
          message.result = take;
        }
        message.count = message.result < 0 ? 0 : (unsigned)message.result;
      }
      break;
    case XVFS_WRITE:
      if (writable && message.node == 2 && message.count <= XVFS_DATA_SIZE &&
          message.offset <= UINT32_MAX && message.count <= UINT32_MAX - message.offset)
      {
        if (xvfs_seek(XVFS_SOURCE_FD, message.offset, 0) < 0) break;
        message.result = xvfs_write(XVFS_SOURCE_FD, message.data, message.count);
        if (message.result > 0 && message.offset + message.result > size)
          size = (unsigned)(message.offset + message.result);
      }
      break;
    case XVFS_FORGET: message.result = 0; break;
    case XVFS_SYNC:
      message.result = source_size >= 0 ? (int)xvfs_syscall(90, XVFS_SOURCE_FD, 0, 0) : 0;
      break;
    }
    if (xvfs_write(service, &message, sizeof(message)) != sizeof(message)) return 1;
  }
  xvfs_close(service);
  return received != 0;
}
