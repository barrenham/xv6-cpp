#include <xvfs/plugin.h>
int main(void)
{
  int console = (int)xvfs_syscall(15, (long)"/dev/console", 2, 0);
  int mounts = (int)xvfs_syscall(15, (long)"/proc/mounts", 0, 0);
  if (console < 0 || mounts < 0) return 1;
  char text[512];
  int size = xvfs_read(mounts, text, sizeof(text));
  xvfs_close(mounts);
  int found = 0;
  for (int i = 0; i + 7 <= size; ++i)
  {
    const char key[] = "modroot";
    int same = 1;
    for (int j = 0; j < 7; ++j) if (text[i + j] != key[j]) same = 0;
    found |= same;
  }
  if (!found) return 1;
  const char marker[] = "external-root: independently loaded init and procfs passed\n";
  return xvfs_write(console, marker, sizeof(marker) - 1) == sizeof(marker) - 1 ? 0 : 1;
}
