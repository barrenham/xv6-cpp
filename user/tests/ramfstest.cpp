#include "kernel/fs/image_paths.h"
#include "kernel/support/types.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
namespace
{
char input[1021], output[1021];
void check(bool ok, const char *message)
{ if (!ok) { fprintf(2, "ramfstest: FAIL: %s\n", message); exit(1); } }
void filename(char *path, int number)
{
  strcpy(path, "/tmp/ramtest/f");
  int used = strlen(path); char digits[12]; unsigned count = 0;
  do { digits[count++] = '0' + number % 10; number /= 10; } while (number);
  while (count) path[used++] = digits[--count];
  path[used] = 0;
}
void reap()
{ int status; check(wait(&status) > 0 && status == 0, "child result"); }
}
extern "C" int main()
{
  struct stat disk{}, memory{};
  check(stat("/", &disk) == 0 && stat("/tmp", &memory) == 0 && memory.type == T_DIR && disk.dev != memory.dev,
        "RAM filesystem mount");
  check(mkdir("/tmp/ramtest") == 0 && chdir("/tmp/ramtest") == 0 && chdir("..") == 0 &&
        stat(".", &memory) == 0 && chdir("..") == 0 && stat(".", &disk) == 0 && memory.dev != disk.dev,
        "mount and relative parent traversal");
  int fd = open("/tmp/ramtest/data", O_CREATE | O_RDWR); check(fd >= 0, "create file");
  constexpr unsigned Size = 1048576;
  for (unsigned position = 0; position < Size;)
  {
    unsigned count = Size - position; if (count > sizeof(input)) count = sizeof(input);
    for (unsigned i = 0; i < count; ++i) input[i] = (position + i) * 17;
    check(write(fd, input, count) == static_cast<int>(count), "large RAM file write"); position += count;
  }
  check(fstat(fd, &memory) == 0 && memory.size == Size, "RAM file exceeds disk maximum");
  int reader = open("/tmp/ramtest/data", O_RDONLY); check(reader >= 0, "independent open");
  check(read(reader, reinterpret_cast<void *>(~0UL), 1) == -1, "invalid read pointer");
  for (unsigned position = 0; position < Size;)
  {
    unsigned count = Size - position; if (count > sizeof(output)) count = sizeof(output);
    check(read(reader, output, count) == static_cast<int>(count), "large RAM file read");
    for (unsigned i = 0; i < count; ++i) check(static_cast<unsigned char>(output[i]) == static_cast<unsigned char>((position + i) * 17), "RAM data contents");
    position += count;
  }
  check(read(reader, output, 1) == 0, "file EOF"); close(reader);
  check(link("/tmp/ramtest/data", "/tmp/ramtest/.") < 0 && link("/tmp/ramtest/data", "/tmp/ramtest/..") < 0, "dot hard links rejected");
  check(link("/tmp/ramtest/data", "/tmp/ramtest/alias") == 0 && unlink("/tmp/ramtest") < 0,
        "hard link and nonempty directory");
  check(unlink("/tmp/ramtest/data") == 0 && unlink("/tmp/ramtest/alias") == 0 &&
        fstat(fd, &memory) == 0 && memory.nlink == 0, "open unlinked file"); close(fd);
  fd = open("/tmp/ramtest/sparse", O_CREATE | O_RDWR); check(fd >= 0 && write(fd, input, 32) == 32, "old file position");
  int truncate = open("/tmp/ramtest/sparse", O_WRONLY | O_TRUNC); check(truncate >= 0, "truncate"); close(truncate);
  check(write(fd, "!", 1) == 1, "write beyond truncated EOF"); close(fd);
  reader = open("/tmp/ramtest/sparse", O_RDONLY);
  check(reader >= 0 && read(reader, output, 33) == 33 && output[32] == '!', "truncated gap size");
  for (int i = 0; i < 32; ++i) check(output[i] == 0, "truncated gap zero fill");
  close(reader); check(unlink("/tmp/ramtest/sparse") == 0, "remove truncated file");
  fd = open("/tmp/ramtest/shared", O_CREATE | O_RDWR); check(fd >= 0 && write(fd, "abc", 3) == 3, "shared fixture"); close(fd);
  reader = open("/tmp/ramtest/shared", O_RDONLY); check(reader >= 0 && read(reader, output, 1) == 1 && output[0] == 'a', "parent first read");
  int child = fork(); check(child >= 0, "fork shared offset");
  if (!child) { check(read(reader, output, 1) == 1 && output[0] == 'b', "child shared offset"); close(reader); exit(0); }
  reap(); check(read(reader, output, 1) == 1 && output[0] == 'c', "parent shared offset advanced"); close(reader);
  check(unlink("/tmp/ramtest/shared") == 0, "remove shared file");
  for (int i = 0; i < 220; ++i)
  { char path[64]; filename(path, i); fd = open(path, O_CREATE | O_RDWR); check(fd >= 0, "growing directory"); close(fd); }
  fd = open("/tmp/ramtest", O_RDONLY); check(fd >= 0, "directory open");
  dirent entry{}; int entries = 0;
  while (read(fd, &entry, sizeof(entry)) == sizeof(entry)) if (entry.inum) ++entries;
  close(fd); check(entries == 222, "directory entries across allocation pages");
  for (int i = 0; i < 220; ++i) { char path[64]; filename(path, i); check(unlink(path) == 0, "remove directory entries"); }
  for (int worker = 0; worker < 4; ++worker)
  {
    child = fork(); check(child >= 0, "concurrent fork");
    if (!child)
    {
      for (int round = 0; round < 20; ++round)
      {
        char path[64]; filename(path, 300 + worker);
        int file = open(path, O_CREATE | O_RDWR); check(file >= 0, "concurrent open");
        check(write(file, input, sizeof(input)) == sizeof(input) && unlink(path) == 0, "concurrent write and unlink"); close(file);
      }
      exit(0);
    }
  }
  for (int i = 0; i < 4; ++i) reap();
  check(unlink("/tmp/ramtest") == 0 && unlink("/tmp") < 0 && link(XV_README_PATH, "/tmp/cross") < 0,
        "directory cleanup and mount protection");
  check(mkdir("/tmp/parent") == 0 && mkdir("/tmp/parent/child") == 0 && chdir("/tmp/parent/child") == 0,
        "orphan directory fixture");
  check(unlink("/tmp/parent/child") == 0 && unlink("/tmp/parent") == 0 && chdir("/") == 0,
        "orphan cwd retains parent until release");
  printf("ramfstest: all tests succeeded\n"); exit(0);
}
