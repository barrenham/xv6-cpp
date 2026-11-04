#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "user/fs/sdk/include/xvfs/control.h"

namespace
{
bool number(const char *text, unsigned &out)
{
  if (!*text) return false;
  unsigned long value = 0;
  for (unsigned i = 0; text[i]; ++i)
  {
    if (i >= 10 || text[i] < '0' || text[i] > '9') return false;
    value = value * 10 + text[i] - '0';
    if (value > 0xffffffffUL) return false;
  }
  out = value;
  return true;
}
int digit(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}
}
extern "C" int main(int argc, char **argv)
{
  bool writable = argc > 1 && !strcmp(argv[1], "-w");
  int first = writable ? 2 : 1;
  int remaining = argc - first;
  if (remaining != 1 && remaining != 4 && remaining != 5)
  {
    fprintf(2, "usage: fsctl [-w] path [interface version command [hex-input]]\n");
    return 1;
  }
  xvfs_control request{};
  request.size = sizeof(request); request.version = XVFS_CONTROL_VERSION;
  request.driver_version = 1; request.capacity = sizeof(request.data);
  const char *interface_name = remaining == 1 ? XVFS_DRIVER_INTERFACE : argv[first + 1];
  if (!*interface_name || strlen(interface_name) >= sizeof(request.interface_name)) return 1;
  strcpy(request.interface_name, interface_name);
  if (remaining >= 4 && (!number(argv[first + 2], request.driver_version) ||
      !request.driver_version || !number(argv[first + 3], request.command))) return 1;
  if (remaining == 5)
  {
    const char *input = argv[first + 4];
    unsigned length = strlen(input);
    if ((length & 1) || length > sizeof(request.data) * 2) return 1;
    for (unsigned i = 0; i < length; i += 2)
    {
      int high = digit(input[i]), low = digit(input[i + 1]);
      if (high < 0 || low < 0) return 1;
      request.data[request.input_size++] = (high << 4) | low;
    }
  }
  int fd = open(argv[first], writable ? O_RDWR : O_RDONLY);
  if (fd < 0) { fprintf(2, "fsctl: cannot open path\n"); return 1; }
  int result = xvfs_driver_control(fd, &request);
  close(fd);
  if (result < 0) { fprintf(2, "fsctl: driver command failed\n"); return 1; }
  if (remaining == 1 && request.output_size == sizeof(xvfs_driver_info))
  {
    xvfs_driver_info info;
    memmove(&info, request.data, sizeof(info));
    if (info.size != sizeof(info) || info.version != 1 || info.interface_name[31]) return 1;
    printf("driver: wire=%u interface=%s version=%u operations=%lx\n",
           info.wire_version, info.interface_name, info.interface_version, info.operations);
  }
  else
  {
    printf("driver: %u bytes:", request.output_size);
    const char *hex = "0123456789abcdef";
    for (unsigned i = 0; i < request.output_size; ++i)
    {
      char pair[3] = {hex[request.data[i] >> 4], hex[request.data[i] & 15], 0};
      printf(" %s", pair);
    }
    printf("\n");
  }
  return 0;
}
