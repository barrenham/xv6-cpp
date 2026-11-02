#include "kernel/fs/stat.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"

namespace
{
void check(bool ok, const char *text)
{
  if (!ok)
  {
    fprintf(2, "userfstest: FAIL: %s\n", text);
    exit(1);
  }
}

char data[4097], output[4097];

void reap(int pid, bool killed = false)
{
  int status;
  check(waitpid(pid, &status, 0) == pid && (killed || status == 0),
        "reap server status");
}

void fault_contract(unsigned mode)
{
  int ready[2];
  check(pipe(ready) == 0, "ready pipe");
  int server = fork();
  check(server >= 0, "fault server fork");
  if (!server)
  {
    close(ready[0]);
    int fd = fsserve("/tmp/ufs");
    if (fd < 0)
    {
      exit(2);
    }
    int duplicate = dup(fd);
    close(fd);
    fd = duplicate;
    if (write(ready[1], "!", 1) != 1)
    {
      exit(3);
    }
    close(ready[1]);
    static xv::userfs::Message m;
    if (read(fd, &m, sizeof(m) - 1) != -1 ||
        read(fd, reinterpret_cast<void *>(~0UL), sizeof(m)) != -1)
    {
      exit(4);
    }
    pollfd event{fd, POLLIN, 0};
    if (poll(&event, 1, 2000) != 1 || !(event.revents & POLLIN))
    {
      exit(5);
    }
    if (read(fd, &m, sizeof(m)) != sizeof(m))
    {
      exit(6);
    }
    if (mode == 1)
    {
      ++m.id;
      if (write(fd, &m, sizeof(m)) != -1)
      {
        exit(10);
      }
      --m.id;
      unsigned count = m.count;
      m.count = xv::userfs::DataSize + 1;
      if (write(fd, &m, sizeof(m)) != -1)
      {
        exit(11);
      }
      m.count = count;
      ++m.version;
      if (write(fd, &m, sizeof(m)) != -1)
      {
        exit(7);
      }
    }
    if (mode == 4)
    {
      if (m.operation != xv::userfs::Operation::Sync)
      {
        exit(12);
      }
      m.result = -1;
      if (write(fd, &m, sizeof(m)) != sizeof(m))
      {
        exit(13);
      }
    }
    if (mode == 3)
    {
      kill(getpid());
      pause(1);
      exit(8);
    }
    if (mode == 2)
    {
      pause(100); // Client must time out before this server exits.
    }
    close(fd);
    exit(0);
  }
  close(ready[1]);
  char byte;
  check(read(ready[0], &byte, 1) == 1, "fault server mounted");
  close(ready[0]);
  int directory = mode == 4 ? open("/tmp/ufs", O_RDONLY) : -1;
  unsigned long begin = clockticks();
  if (mode == 4)
  {
    check(directory >= 0 && fsync(directory) < 0, "server sync error propagated");
    struct stat snapshot{};
    check(fstat(directory, &snapshot) < 0, "disconnected stat error propagated");
    close(directory);
  }
  else
  {
    check(open("/tmp/ufs/missing", O_RDONLY) < 0, "fault returns error");
  }
  unsigned long elapsed = clockticks() - begin;
  check(elapsed < 80000000UL, "bounded fault wait");
  if (mode == 2)
  {
    check(elapsed >= 45000000UL, "deadline actually waited");
    kill(server);
  }
  reap(server, mode == 2 || mode == 3);
  check(umount("/tmp/ufs") == 0, "unmount disconnected service");
}
} // namespace

extern "C" int main()
{
  mkdir("/tmp/ufs");
  struct stat covered{};
  check(stat("/tmp/ufs", &covered) == 0, "covered mountpoint metadata");
  int server = fork();
  check(server >= 0, "fork server");
  if (!server)
  {
    char *arguments[] = {const_cast<char *>("memfsd"), const_cast<char *>("/tmp/ufs"),
                         nullptr};
    exec("/bin/memfsd", arguments);
    exit(1);
  }
  struct stat st{};
  bool ready = false;
  for (unsigned i = 0; i < 30; ++i)
  {
    if (stat("/tmp/ufs", &st) == 0 && st.dev >= 0x10000 && st.dev != covered.dev)
    {
      ready = true;
      break;
    }
    pause(1);
  }
  check(ready, "userspace mount");
  int fd = open("/tmp/ufs/data", O_CREATE | O_RDWR);
  check(fd >= 0, "create");
  for (unsigned i = 0; i < sizeof(data); ++i)
  {
    data[i] = i * 17;
  }
  check(write(fd, data, sizeof(data)) == sizeof(data), "multi-message write");
  check(lseek(fd, 0, SEEK_SET) == 0 &&
            read(fd, output, sizeof(output)) == sizeof(output),
        "multi-message read");
  check(!memcmp(data, output, sizeof(data)), "payload");
  check(ftruncate(fd, 9) == 0 && lseek(fd, 0, SEEK_END) == 9, "resize");
  check(umount("/tmp/ufs") < 0, "busy open file");
  check(fsync(fd) == 0 && fstat(fd, &st) == 0 && st.size == 9,
        "sync and refreshed stat");
  check(lseek(fd, 0, SEEK_SET) == 0 && read(fd, reinterpret_cast<void *>(~0UL), 1) < 0,
        "bad user pointer");
  close(fd);
  check(chdir("/tmp/ufs") == 0 && umount("/tmp/ufs") < 0 && chdir("/") == 0,
        "busy working directory");
  check(chmod("/tmp/ufs/data", 0600) == 0, "chmod server metadata");
  int child = fork();
  check(child >= 0, "permission child");
  if (!child)
  {
    if (setuid(1000) || open("/tmp/ufs/data", O_RDONLY) >= 0 || fsserve("/tmp") >= 0)
    {
      exit(1);
    }
    exit(0);
  }
  int child_status;
  check(waitpid(child, &child_status, 0) == child && !child_status,
        "kernel permissions");
  check(mkdir("/tmp/ufs/sub") == 0, "directory");
  check(rename("/tmp/ufs/data", "/tmp/ufs/sub/moved") == 0, "rename");
  fd = open("/tmp/ufs/sub/moved", O_RDONLY);
  check(fd >= 0 && read(fd, output, sizeof(output)) == 9 && !memcmp(data, output, 9),
        "renamed data");
  close(fd);
  check(link("/tmp/ufs/sub/moved", "/tmp/ufs/alias") == 0, "link");
  check(symlink("sub/moved", "/tmp/ufs/sym") == 0, "symlink");
  fd = open("/tmp/ufs/sym", O_RDONLY);
  check(fd >= 0 && read(fd, output, 9) == 9, "follow symlink");
  close(fd);
  fd = open("/tmp/ufs", O_RDONLY);
  check(fd >= 0 && getdents64(fd, output, sizeof(output)) > 0, "getdents");
  close(fd);
  check(unlink("/tmp/ufs/sub") < 0, "nonempty directory");
  check(unlink("/tmp/ufs/sub/moved") == 0 && unlink("/tmp/ufs/sub") == 0,
        "unlink tree");
  fd = open("/tmp/ufs/alias", O_RDONLY);
  check(fd >= 0 && unlink("/tmp/ufs/alias") == 0 && fstat(fd, &st) == 0 &&
            st.nlink == 0 && read(fd, output, 9) == 9,
        "open unlinked inode survives");
  close(fd);
  check(unlink("/tmp/ufs/sym") == 0, "unlink symlink");
  for (int i = 0; i < 4; ++i)
  {
    child = fork();
    check(child >= 0, "concurrent client fork");
    if (!child)
    {
      char path[] = "/tmp/ufs/worker0";
      path[15] += i;
      int file = open(path, O_CREATE | O_RDWR);
      if (file < 0)
      {
        exit(1);
      }
      for (unsigned turn = 0; turn < 16; ++turn)
      {
        if (write(file, data, sizeof(data)) != sizeof(data))
        {
          exit(2);
        }
      }
      if (lseek(file, 0, SEEK_SET) != 0 ||
          read(file, output, sizeof(output)) != sizeof(output) ||
          memcmp(data, output, sizeof(data)))
      {
        exit(3);
      }
      close(file);
      exit(unlink(path) ? 4 : 0);
    }
  }
  for (int i = 0; i < 4; ++i)
  {
    check(wait(&child_status) > 0 && !child_status, "concurrent client payload");
  }
  for (unsigned i = 0; i < 160; ++i)
  {
    int file = open("/tmp/ufs/reused", O_CREATE | O_RDWR);
    check(file >= 0 && write(file, "x", 1) == 1 && unlink("/tmp/ufs/reused") == 0,
          "create and unlink beyond object capacity");
    check(read(file, output, 1) == 0 && fstat(file, &st) == 0 && !st.nlink,
          "unlinked object held until final close");
    close(file);
  }
  check(umount("/tmp/ufs") == 0, "unmount");
  int status;
  check(waitpid(server, &status, 0) == server && !status, "server exited on unmount");
  for (unsigned i = 0; i < 10; ++i)
  {
    fault_contract(i % 5);
  }
  check(unlink("/tmp/ufs") == 0, "mountpoint cleanup");
  printf("userfstest: all tests succeeded\n");
  exit(0);
}
