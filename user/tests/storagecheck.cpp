#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"

static void check(bool value, const char *message)
{
  if (!value)
  {
    fprintf(2, "storagecheck: FAIL: %s\n", message);
    exit(1);
  }
}

static void one(const char *directory)
{
  check(mkdir(directory) == 0 && chdir(directory) == 0, "workspace");
  int fd = open("file", O_CREATE | O_RDWR);
  check(fd >= 0, "open");
  check(write(fd, "abcdef", 6) == 6 && lseek(fd, -3, SEEK_END) == 3, "seek end");
  int alias = dup(fd);
  char c;
  check(alias >= 0 && read(alias, &c, 1) == 1 && c == 'd' &&
            lseek(fd, 0, SEEK_CUR) == 4,
        "shared offset");
  close(alias);
  check(lseek(fd, -10, SEEK_SET) < 0 && lseek(fd, 0, 99) < 0 &&
            lseek(fd, 0, SEEK_CUR) == 4,
        "bad seek preserves offset");
  check(lseek(fd, 18000, SEEK_SET) == 18000 && write(fd, "X", 1) == 1 &&
            ftruncate(fd, 20000) == 0,
        "sparse extend");
  check(lseek(fd, 6, SEEK_SET) == 6, "gap seek");
  char data[512];
  unsigned gap = 18000 - 6;
  while (gap)
  {
    unsigned n = gap > sizeof(data) ? sizeof(data) : gap;
    check(read(fd, data, n) == static_cast<int>(n), "gap read");
    for (unsigned i = 0; i < n; ++i)
    {
      check(data[i] == 0, "zero gap");
    }
    gap -= n;
  }
  check(read(fd, &c, 1) == 1 && c == 'X', "sparse marker");
  check(ftruncate(fd, 3) == 0 && ftruncate(fd, 9000) == 0 &&
            lseek(fd, 3, SEEK_SET) == 3 && read(fd, data, sizeof(data)) == sizeof(data),
        "shrink and grow");
  for (char value : data)
  {
    check(!value, "tail does not reappear");
  }
  check(fsync(fd) == 0, "file sync");
  close(fd);
  int readonly = open("file", O_RDONLY);
  check(readonly >= 0 && ftruncate(readonly, 0) < 0, "truncate needs writable open");
  close(readonly);
  check(symlink("file", "relative") == 0 && symlink("relative", "chain") == 0,
        "relative link chain");
  struct stat st;
  check(lstat("chain", &st) == 0 && st.type == T_SYMLINK && stat("chain", &st) == 0 &&
            st.type == T_FILE,
        "stat vs lstat");
  char target[16]{};
  check(readlink("chain", target, 3) == 3 && !memcmp(target, "rel", 3),
        "readlink bounded bytes");
  check(open("relative", O_RDONLY | O_NOFOLLOW) < 0 && open("relative", O_CREATE | O_WRONLY | O_NOFOLLOW) < 0, "nofollow including create");
  check(symlink("file/", "bad-slash") == 0 && open("bad-slash", O_RDONLY) < 0, "trailing slash requires directory");
  check(symlink("missing", "dangling") == 0, "dangling link");
  fd = open("dangling", O_CREATE | O_WRONLY);
  check(fd >= 0 && write(fd, "new", 3) == 3, "create through dangling link");
  close(fd);
  check(symlink("loop2", "loop1") == 0 && symlink("loop1", "loop2") == 0 &&
            open("loop1", O_RDONLY) < 0,
        "link cycle");
  check(unlink("relative") == 0 && stat("file", &st) == 0,
        "unlink link retains target");
  check(symlink("real", "diralias") == 0 && mkdir("real") == 0 && chdir("diralias") == 0, "directory alias");
  char physical[MAXPATH]; check(getcwd(physical, sizeof(physical)) != nullptr, "physical cwd");
  char *last = physical; for (char *p = physical; *p; ++p) if (*p == '/') last = p + 1;
  check(!strcmp(last, "real") && chdir("..") == 0, "cwd skips symbolic aliases");
  int fds[2];
  check(pipe(fds) == 0 && lseek(fds[0], 0, SEEK_SET) < 0 && fsync(fds[0]) < 0,
        "unseekable pipe");
  close(fds[0]);
  close(fds[1]);
  fd = open("append", O_CREATE | O_WRONLY | O_APPEND);
  check(fd >= 0, "append fixture");
  close(fd);
  for (int worker = 0; worker < 3; ++worker)
  {
    int pid = fork();
    check(pid >= 0, "append fork");
    if (!pid)
    {
      int out = open("append", O_WRONLY | O_APPEND);
      check(out >= 0, "append open");
      for (int i = 0; i < 30; ++i)
      {
        check(write(out, "line\n", 5) == 5, "append write");
      }
      close(out);
      exit(0);
    }
  }
  for (int i = 0; i < 3; ++i)
  {
    int status;
    check(wait(&status) > 0 && status == 0, "append children");
  }
  check(stat("append", &st) == 0 && st.size == 450, "append cannot overwrite");
  fd = open("setid", O_CREATE | O_WRONLY);
  check(fd >= 0 && chmod("setid", 04755) == 0 && write(fd, "x", 1) == 1 &&
            stat("setid", &st) == 0 && !(st.mode & 06000),
        "write clears setid bits");
  close(fd);
  check(chdir("/") == 0, "restore cwd");
}

extern "C" int main()
{
  one("/tmp/io-check");
  one("/root/io-check");
  printf("storagecheck: all tests succeeded\n");
  exit(0);
}
