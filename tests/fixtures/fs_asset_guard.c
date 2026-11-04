#include <xvfs/resources.h>
struct asset_request {
  uint32_t size, version, index, flags;
  uint64_t bytes;
  char name[64];
};
static int open_asset(struct asset_request *request)
{ return xvfs_ioctl(4, 0x58560006u, (uintptr_t)request); }
static int fail(void)
{
  const char text[] = "asset-guard: FAIL: immutable execution contract\n";
  xvfs_write(2, text, sizeof(text) - 1);
  return 1;
}
int main(int argc, char **argv)
{
  if (argc == 2 && argv[1][0] == 'd')
  {
    if (xvfs_syscall(68, 0, 0, 0) != 1000) return fail();
    struct asset_request request = { .size = sizeof(request), .version = 1, .name = "data" };
    if (open_asset(&request) >= 0) return fail();
    const char text[] = "asset-guard: PASS: bounds, failed exec, immutable descriptors and delegated identity\n";
    xvfs_write(1, text, sizeof(text) - 1);
    for (;;) xvfs_syscall(13, 1, 0, 0);
  }
  struct xvfs_resource_query query = { .size = sizeof(query),
    .version = XVFS_RESOURCE_VERSION, .operation = XVFS_SYSTEM, .result = -1 };
  if (xvfs_ioctl(4, XVFS_RESOURCE_QUERY, (uintptr_t)&query) < 0 ||
      query.result != 1 || query.count != sizeof(struct xvfs_system)) return fail();
  struct xvfs_system *system = (struct xvfs_system *)query.data;
  if (!system->total_kib || system->free_kib > system->total_kib) return fail();
  struct asset_request request = { .size = sizeof(request), .version = 99 };
  if (open_asset(&request) >= 0) return fail();
  request.version = 1;
  request.index = 999;
  if (open_asset(&request) >= 0) return fail();
  request.index = 0;
  for (unsigned i = 0; i < 64; ++i) request.name[i] = 'a';
  if (open_asset(&request) >= 0) return fail();
  struct asset_request data = { .size = sizeof(data), .version = 1, .name = "data" };
  int fd = open_asset(&data);
  if (fd < 0 || data.bytes != 4 || data.flags) return fail();
  char bytes[4];
  if (xvfs_write(fd, "oops", 4) >= 0 || xvfs_seek(fd, 5, 0) >= 0 ||
      xvfs_read(fd, bytes, 4) != 4 || bytes[0] != 'D' || xvfs_read(fd, bytes, 1) != 0) return fail();
  char *arguments[] = { "asset-guard", "delegated", 0 };
  if (xvfs_syscall(96, fd, (long)arguments, 0) >= 0) return fail();
  xvfs_close(fd);
  struct asset_request bad = { .size = sizeof(bad), .version = 1, .name = "bad" };
  fd = open_asset(&bad);
  if (fd < 0 || xvfs_syscall(96, fd, (long)arguments, 0) >= 0) return fail();
  xvfs_close(fd);
  struct asset_request self = { .size = sizeof(self), .version = 1, .name = "self" };
  fd = open_asset(&self);
  if (fd < 0 || xvfs_syscall(72, 1000, 0, 0) < 0) return fail();
  if (open_asset(&data) >= 0) return fail();
  xvfs_syscall(96, fd, (long)arguments, 0);
  return fail();
}
