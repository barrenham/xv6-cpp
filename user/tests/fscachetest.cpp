#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/io/fs_control.h"
#include "kernel/fs/userfs/protocol.h"
#include "user/runtime/user.h"

namespace
{
void
check(bool condition, const char *message) {
  if (!condition)
  {
    fprintf(2, "fscachetest: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

unsigned long
requests(int file) {
  xv::fscontrol::Request query;
  strcpy(query.interface_name, "xvfs.transport");
  query.command = 1;
  check(ioctl(file, xv::fscontrol::Ioctl, reinterpret_cast<unsigned long>(&query)) ==
                0 &&
            query.output_size == sizeof(xv::userfs::TransportStatistics),
        "query statistics");
  xv::userfs::TransportStatistics value;
  memmove(&value, query.data, sizeof(value));
  return value.exchanges;
}

void
name(char *out, const char *directory, const char *leaf) {
  strcpy(out, directory);
  strcpy(out + strlen(out), "/");
  strcpy(out + strlen(out), leaf);
}
} // namespace

extern "C" int
main(int argc, char **argv) {
  const char *directory = argc > 1 ? argv[1] : "/tmp/fscache";
  check(strlen(directory) < 96 && mkdir(directory) == 0, "test directory");
  char a[108], b[108], c[108], d[108];
  name(a, directory, "a");
  name(b, directory, "b");
  name(c, directory, "c");
  name(d, directory, "d");
  int original = open(a, O_CREATE | O_RDWR);
  check(original >= 0 && write(original, "alpha", 5) == 5, "create original");
  struct stat attributes{};
  check(stat(a, &attributes) == 0 && attributes.size == 5, "warm path and attributes");
  auto before = requests(original);
  for (unsigned turn = 0; turn < 32; ++turn)
  {
    int file = open(a, O_RDONLY);
    check(file >= 0 && fstat(file, &attributes) == 0 && attributes.size == 5 &&
              stat(a, &attributes) == 0 && attributes.size == 5,
          "repeated metadata access");
    close(file);
  }
  auto after = requests(original);
  check(after == before, "cached path and attributes eliminate duplicate RPCs");
  unsigned identity = attributes.ino;
  check(lseek(original, 0, SEEK_SET) == 0 && write(original, "beta", 4) == 4 &&
            ftruncate(original, 2) == 0 && stat(a, &attributes) == 0 &&
            attributes.size == 2,
        "write and truncate keep cached size coherent");
  check(rename(a, b) == 0 && stat(a, &attributes) < 0 && stat(b, &attributes) == 0 &&
            attributes.ino == identity,
        "rename invalidates old pathname");
  int replaced = open(c, O_CREATE | O_RDWR);
  check(replaced >= 0 && write(replaced, "c", 1) == 1 &&
            fstat(replaced, &attributes) == 0 && attributes.nlink == 1,
        "warm replacement inode attributes");
  check(rename(b, c) == 0 && fstat(replaced, &attributes) == 0 &&
            attributes.nlink == 0 && stat(c, &attributes) == 0 &&
            attributes.ino == identity,
        "replacement invalidates attributes of open unlinked inode");
  check(link(c, d) == 0 && fstat(original, &attributes) == 0 && attributes.nlink == 2,
        "hard link invalidates link count");
  check(unlink(c) == 0 && stat(c, &attributes) < 0 && stat(d, &attributes) == 0 &&
            attributes.nlink == 1,
        "unlink invalidates names and link count");
  check(unlink(d) == 0 && fstat(original, &attributes) == 0 && attributes.nlink == 0,
        "open unlinked inode survives cache invalidation");
  char content[8]{};
  check(lseek(original, 0, SEEK_SET) == 0 &&
            read(original, content, sizeof(content)) == 2 && !memcmp(content, "be", 2),
        "unlinked data remains valid");
  close(original);
  close(replaced);
  int restricted = open(a, O_CREATE | O_RDWR);
  check(restricted >= 0 && chmod(a, 0644) == 0 && stat(a, &attributes) == 0 &&
            chmod(a, 0000) == 0,
        "warm then change permissions");
  int child = fork();
  check(child >= 0, "permission probe fork");
  if (!child)
  {
    check(setuid(1000) == 0 && open(a, O_RDONLY) < 0,
          "cached name cannot bypass new permissions");
    exit(0);
  }
  int status = -1;
  check(waitpid(child, &status, 0) == child && status == 0, "permission probe");
  close(restricted);
  check(unlink(a) == 0, "remove restricted file");
  for (unsigned i = 0; i < 24; ++i)
  {
    char leaf[]{'p', char('a' + i), 0};
    name(a, directory, leaf);
    int file = open(a, O_CREATE | O_RDWR);
    check(file >= 0 && write(file, leaf + 1, 1) == 1, "eviction seed");
    close(file);
  }
  for (unsigned round = 0; round < 2; ++round)
  {
    for (unsigned i = 0; i < 24; ++i)
    {
      char leaf[]{'p', char('a' + i), 0};
      name(a, directory, leaf);
      int file = open(a, O_RDONLY);
      check(file >= 0 && read(file, content, 1) == 1 && content[0] == leaf[1],
            "cache eviction preserves identity and references");
      close(file);
    }
  }
  for (unsigned i = 0; i < 24; ++i)
  {
    char leaf[]{'p', char('a' + i), 0};
    name(a, directory, leaf);
    check(unlink(a) == 0, "eviction cleanup");
  }
  check(unlink(directory) == 0, "test directory cleanup");
  printf("FSCACHE metadata_operations=96 rpc=%lu\n", after - before);
  printf(
      "fscachetest: PASS: paths, attributes, rename, replacement, links, permissions and eviction\n");
  return 0;
}
