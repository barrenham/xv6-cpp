#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "file_copy.h"

namespace fileutil
{
bool destination(const char *source, const char *target, char *output,
                 unsigned capacity)
{
  unsigned size = strlen(target);
  if (size >= capacity)
  {
    return false;
  }
  strcpy(output, target);
  struct stat st;
  if (stat(target, &st) < 0 || st.type != T_DIR)
  {
    return true;
  }
  const char *base = source;
  for (const char *p = source; *p; ++p)
  {
    if (*p == '/')
    {
      base = p + 1;
    }
  }
  if (!*base || size + strlen(base) + 2 > capacity)
  {
    return false;
  }
  if (size && output[size - 1] != '/')
  {
    output[size++] = '/';
  }
  strcpy(output + size, base);
  return true;
}

int copy(const char *source, const char *target)
{
  int input = open(source, O_RDONLY);
  if (input < 0)
  {
    return -1;
  }
  struct stat src, dst;
  if (fstat(input, &src) < 0 || src.type != T_FILE ||
      (stat(target, &dst) == 0 &&
       (dst.type != T_FILE || (src.dev == dst.dev && src.ino == dst.ino))))
  {
    close(input);
    return -1;
  }
  int output = open(target, O_CREATE | O_WRONLY | O_TRUNC);
  if (output < 0)
  {
    close(input);
    return -1;
  }
  char data[512];
  int count, result = 0;
  while ((count = read(input, data, sizeof(data))) > 0)
  {
    int used = 0;
    while (used < count)
    {
      int n = write(output, data + used, count - used);
      if (n <= 0)
      {
        result = -1;
        break;
      }
      used += n;
    }
    if (result < 0)
    {
      break;
    }
  }
  if (count < 0)
  {
    result = -1;
  }
  close(input);
  close(output);
  return result;
}
} // namespace fileutil
