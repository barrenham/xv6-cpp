#include "kernel/support/types.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/mount.h"
#include "user/runtime/user.h"

namespace
{
char bytes[1024];
void check(bool ok, const char *message)
{
  if (!ok) { fprintf(2, "xv6fstest: FAIL: %s\n", message); exit(1); }
}
void file_contract()
{
  int fd = open("/root/v6-data", O_CREATE | O_RDWR);
  check(fd >= 0, "create data");
  for (unsigned block = 0; block < 20; ++block)
  {
    memset(bytes, block + 1, sizeof(bytes));
    check(write(fd, bytes, sizeof(bytes)) == sizeof(bytes), "direct and indirect write");
  }
  check(lseek(fd, 0, 0) == 0, "rewind");
  for (unsigned block = 0; block < 20; ++block)
  {
    check(read(fd, bytes, sizeof(bytes)) == sizeof(bytes), "direct and indirect read");
    for (char value : bytes) check(value == block + 1, "data pattern");
  }
  constexpr long Maximum = (12 + 256) * 1024;
  check(ftruncate(fd, Maximum) == 0 && lseek(fd, Maximum - 1, 0) == Maximum - 1 &&
        write(fd, "Z", 1) == 1 && write(fd, "!", 1) < 0, "maximum sparse file");
  check(lseek(fd, Maximum - 2, 0) == Maximum - 2 && read(fd, bytes, 2) == 2 &&
        bytes[0] == 0 && bytes[1] == 'Z', "sparse gap zero");
  check(ftruncate(fd, 17) == 0 && ftruncate(fd, 1024) == 0 &&
        lseek(fd, 17, 0) == 17 && read(fd, bytes, 1007) == 1007, "shrink and regrow");
  for (unsigned i = 0; i < 1007; ++i) check(bytes[i] == 0, "truncated tail stays zero");
  check(link("/root/v6-data", "/root/v6-alias") == 0 &&
        unlink("/root/v6-data") == 0 && unlink("/root/v6-alias") == 0,
        "unlink hard links while open");
  struct stat info{};
  check(fstat(fd, &info) == 0 && info.nlink == 0 && lseek(fd, 0, 0) == 0 &&
        read(fd, bytes, 1) == 1 && bytes[0] == 1, "open orphan survives");
  close(fd);
  fd = open("/root/abcdefghijklmn", O_CREATE | O_RDWR);
  check(fd >= 0, "fourteen character name");
  close(fd);
  struct stat canonical{};
  check(stat("/root/abcdefghijklmn", &canonical) == 0, "canonical name attributes");
  fd = open("/root/abcdefghijklmno", O_CREATE | O_RDWR);
  check(fd >= 0 && fstat(fd, &info) == 0 && info.dev == canonical.dev &&
        info.ino == canonical.ino && write(fd, "alias", 5) == 5,
        "long name aliases the same fourteen character inode");
  close(fd);
  fd = open("/root/abcdefghijklmn", O_RDONLY);
  check(fd >= 0 && read(fd, bytes, 5) == 5 && !memcmp(bytes, "alias", 5),
        "canonical name sees writes through truncated alias");
  close(fd);
  check(chmod("/root/abcdefghijklmn", 0640) == 0 &&
        chown("/root/abcdefghijklmn", 123, 456) == 0 &&
        stat("/root/abcdefghijklmn", &info) == 0 && info.uid == 123 &&
        info.gid == 456 && (info.mode & 0777) == 0640, "stored ownership and mode");
  check(unlink("/root/abcdefghijklmn") == 0, "remove named fixture");
}
void directory_contract()
{
  check(mkdir("/root/v6-a") == 0 && mkdir("/root/v6-b") == 0 &&
        mkdir("/root/v6-a/c") == 0 && chdir("/root/v6-a/c") == 0,
        "directory fixture");
  check(rename("/root/v6-a/c", "/root/v6-b/c") == 0, "move current directory");
  char path[128];
  check(getcwd(path, sizeof(path)) && !strcmp(path, "/root/v6-b/c"), "cwd follows rename");
  check(rename("/root/v6-b", "/root/v6-b/c/cycle") < 0, "reject directory cycle");
  check(unlink("/root/v6-b/c") == 0, "unlink current directory");
  check(unlink("/root/v6-b") == 0, "unlink current directory parent");
  // Existing VFS policy rejects pathname traversal from an unlinked cwd.
  // The server must retain its objects until cwd release, then reclaim them.
  check(chdir("..") < 0, "unlinked cwd traversal policy");
  check(chdir("/root") == 0, "release deleted cwd and parent chain");
  check(getcwd(path, sizeof(path)) && !strcmp(path, "/root"), "recover linked cwd path");
  check(unlink("/root/v6-a") == 0, "remove original parent");
}
void concurrent_contract()
{
  for (unsigned worker = 0; worker < 4; ++worker)
  {
    int pid = fork();
    check(pid >= 0, "fork root filesystem clients");
    if (pid) continue;
    char path[] = "/root/v6-worker0";
    path[sizeof(path) - 2] += worker;
    for (unsigned round = 0; round < 20; ++round)
    {
      int fd = open(path, O_CREATE | O_RDWR);
      check(fd >= 0 && write(fd, "parallel", 8) == 8 && unlink(path) == 0,
            "concurrent create write and unlink");
      check(lseek(fd, 0, 0) == 0 && read(fd, bytes, 8) == 8 &&
            !memcmp(bytes, "parallel", 8), "concurrent orphan contents");
      close(fd);
    }
    exit(0);
  }
  for (unsigned worker = 0; worker < 4; ++worker)
  {
    int status;
    check(wait(&status) > 0 && status == 0, "concurrent root client result");
  }
}
}

extern "C" int main(int argc, char **argv)
{
  mount_info root{};
  check(mountinfo(0, &root) == 1 && !strcmp(root.type, "xv6"), "xv6 root mount");
  struct stat disk{}, ram{};
  check(stat("/", &disk) == 0 && stat("/tmp", &ram) == 0 && disk.dev != ram.dev,
        "independent memory filesystem");
  check(argc == 2, "explicit boot phase");
  if (!strcmp(argv[1], "first"))
  {
    file_contract();
    directory_contract();
    concurrent_contract();
    int fd = open("/root/v6-keep", O_CREATE | O_RDWR);
    check(fd >= 0 && write(fd, "persistent", 10) == 10 && fsync(fd) == 0, "persistent marker");
    close(fd);
    fd = open("/tmp/v6-volatile", O_CREATE | O_RDWR);
    check(fd >= 0 && write(fd, "temporary", 9) == 9, "volatile marker");
    close(fd);
    check(sync() == 0, "synchronize user services");
  }
  else
  {
    check(!strcmp(argv[1], "second"), "known boot phase");
    int fd = open("/root/v6-keep", O_RDONLY);
    check(fd >= 0 && read(fd, bytes, sizeof(bytes)) == 10 &&
          !memcmp(bytes, "persistent", 10), "recover durable marker");
    close(fd);
    check(stat("/tmp/v6-volatile", &ram) < 0, "memory filesystem resets on reboot");
    file_contract();
    directory_contract();
    concurrent_contract();
  }
  printf("xv6fstest: PASS: %s boot\n", argv[1]);
  exit(0);
}
