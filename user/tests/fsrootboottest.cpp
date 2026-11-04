#include "kernel/support/types.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"
#include "kernel/fs/userfs/protocol.h"
import xv.userfs;
import xv.userfs.block;
import xv.userfs.ext3;
import xv.userfs.devfs;





namespace
{
[[noreturn]] void failed(const char *message)
{
  printf("fsrootboottest: FAIL: %s\n", message);
  if (getpid() != 1) exit(1);
  for (;;) pause(1);
}

void require(bool okay, const char *message)
{
  if (!okay) failed(message);
}

void worker(int ready)
{
  close(4);
  userfs::BlockIo io(3);
  auto *storage = malloc(sizeof(userfs::Ext3Backend));
  require(storage != nullptr, "allocate volume");
  auto *backend = new (storage) userfs::Ext3Backend(io);
  if (!backend->initialize())
  {
    printf("fsrootboottest: ext3 error=%d\n", int(backend->error()));
    failed("mount volume in userspace");
  }
  int endpoint = userfs::register_backend("ext3", *backend);
  require(endpoint >= 0, "register ext3 backend");
  require(write(ready, "R", 1) == 1, "announce backend");
  close(ready);
  int result = userfs::serve(endpoint, *backend);
  printf("fsrootboottest: service stopped result=%d\n", result);
  exit(result ? 1 : 0);
}
void device_worker(int ready)
{
  userfs::DeviceBackend backend(4);
  close(4);
  require(backend.ready(), "clone device resources");
  int endpoint = userfs::register_backend("devfs", backend);
  require(endpoint >= 0, "register device backend");
  require(write(ready, "D", 1) == 1, "announce device backend");
  close(ready);
  int result = userfs::serve(endpoint, backend);
  exit(result ? 1 : 0);
}
} // namespace


extern "C" int main(int, char **)
{
  require(getpid() == 1, "initial process");
  int channel[2];
  require(pipe(channel) == 0, "bootstrap pipe");
  int server = fork();
  require(server >= 0, "fork root backend");
  if (!server)
  {
    close(channel[0]);
    worker(channel[1]);
  }
  close(channel[1]);
  close(3); // Only the serving process retains the writable block capability.
  char ready = 0;
  require(read(channel[0], &ready, 1) == 1 && ready == 'R', "backend ready");
  close(channel[0]);
  require(mount("ext3", "/") == 0, "publish root backend");
  require(chdir("/") == 0, "initial cwd after root mount");
  printf("fsrootboottest: userspace ext3 root mounted server=%d\n", server);

  require(pipe(channel) == 0, "device bootstrap pipe");
  int device_server = fork();
  require(device_server >= 0, "fork device backend");
  if (!device_server)
  {
    close(channel[0]);
    device_worker(channel[1]);
  }
  close(channel[1]);
  close(4);
  require(read(channel[0], &ready, 1) == 1 && ready == 'D', "device backend ready");
  close(channel[0]);
  require(mount("devfs", "/dev") == 0, "mount user device backend");
  int zero = open("/dev/zero", O_RDWR), null = open("/dev/null", O_RDWR);
  char zeros[33];
  require(zero >= 0 && null >= 0 && read(zero, zeros, sizeof(zeros)) == sizeof(zeros),
          "open and read user device nodes");
  for (auto value : zeros) require(value == 0, "zero device content");
  require(write(null, zeros, sizeof(zeros)) == sizeof(zeros) && read(null, zeros, 1) == 0,
          "null device I/O");
  close(zero);
  close(null);
  int master = posix_openpt(O_RDWR | O_NONBLOCK);
  require(master >= 0 && grantpt(master) == 0 && unlockpt(master) == 0,
          "create named PTY through user devfs");
  char slave_name[32];
  require(ptsname_r(master, slave_name, sizeof(slave_name)) == 0, "named PTY path");
  int slave = open(slave_name, O_RDWR | O_NONBLOCK);
  require(slave >= 0, "open user devfs PTY slave");
  struct termios tty{};
  require(tcgetattr(slave, &tty) == 0, "PTY termios");
  cfmakeraw(&tty);
  require(tcsetattr(slave, TCSANOW, &tty) == 0, "PTY raw mode");
  require(write(master, "pty", 3) == 3 && read(slave, zeros, sizeof(zeros)) == 3 &&
          !memcmp(zeros, "pty", 3), "PTY input through user devfs");
  close(slave);
  close(master);
  require(open(slave_name, O_RDWR) < 0, "released PTY disappears");
  printf("fsrootboottest: user devfs and named PTY passed\n");

  constexpr char marker[] = "userspace-root-persistence\n";
  constexpr char path[] = "/tmp/fsroot-persistent";
  int fd = open(path, O_RDONLY);
  char data[sizeof(marker)]{};
  if (fd >= 0)
  {
    require(read(fd, data, sizeof(data)) == sizeof(marker) - 1 &&
            !strcmp(data, marker), "persistent marker content");
    close(fd);
    printf("fsrootboottest: persistent marker recovered\n");
  }
  else
  {
    fd = open(path, O_CREATE | O_WRONLY | O_TRUNC);
    require(fd >= 0, "create persistent marker");
    require(write(fd, marker, sizeof(marker) - 1) == sizeof(marker) - 1,
            "write persistent marker");
    require(fsync(fd) == 0, "sync persistent marker");
    close(fd);
    printf("fsrootboottest: persistent marker created\n");
  }

  fd = open("/tmp/fsroot-data", O_CREATE | O_RDWR | O_TRUNC);
  require(fd >= 0, "create data file");
  auto *pattern = static_cast<char *>(malloc(9000));
  auto *copy = static_cast<char *>(malloc(9000));
  require(pattern && copy, "allocate test buffers");
  for (unsigned i = 0; i < 9000; ++i) pattern[i] = char(i * 17);
  require(write(fd, pattern, 9000) == 9000, "write across pages and frames");
  require(lseek(fd, 0, SEEK_SET) == 0 && read(fd, copy, 9000) == 9000 &&
          !memcmp(pattern, copy, 9000), "read across pages and frames");
  require(unlink("/tmp/fsroot-data") == 0, "unlink open file");
  require(lseek(fd, 5000, SEEK_SET) == 5000 && read(fd, copy, 4000) == 4000 &&
          !memcmp(pattern + 5000, copy, 4000), "open orphan remains readable");
  close(fd);
  free(copy);
  free(pattern);

  require(mkdir("/tmp/fsroot-directory") == 0, "mkdir");
  require(rename("/tmp/fsroot-directory", "/tmp/fsroot-renamed") == 0,
          "rename directory");
  require(unlink("/tmp/fsroot-renamed") == 0, "remove directory");
  require(link(path, "/tmp/fsroot-linked") == 0, "hard link");
  require(unlink("/tmp/fsroot-linked") == 0, "unlink hard link");
  require(symlink(path, "/tmp/fsroot-symlink") == 0, "symlink");
  fd = open("/tmp/fsroot-symlink", O_RDONLY);
  require(fd >= 0 && read(fd, data, sizeof(data)) == sizeof(marker) - 1,
          "follow symlink");
  close(fd);
  require(unlink("/tmp/fsroot-symlink") == 0, "unlink symlink");

  fd = open("/tmp/fsroot-private", O_CREATE | O_RDWR | O_TRUNC);
  require(fd >= 0 && chmod("/tmp/fsroot-private", 0600) == 0, "private file mode");
  close(fd);
  int actor = fork();
  require(actor >= 0, "fork permission actor");
  if (!actor)
  {
    require(setuid(123) == 0, "drop actor uid");
    require(open("/tmp/fsroot-private", O_RDONLY) < 0, "deny private read");
    require(unlink("/tmp/fsroot-private") < 0, "sticky directory ownership");
    require(mount("ext3", "/") < 0, "deny non-root mount");
    int owned = open("/tmp/fsroot-actor", O_CREATE | O_RDWR | O_TRUNC);
    require(owned >= 0, "non-root create");
    close(owned);
    exit(0);
  }
  int actor_status = -1;
  require(wait(&actor_status) == actor && actor_status == 0, "permission actor");
  struct stat actor_file{};
  require(stat("/tmp/fsroot-actor", &actor_file) == 0 && actor_file.uid == 123,
          "persist creator identity");
  require(unlink("/tmp/fsroot-private") == 0 && unlink("/tmp/fsroot-actor") == 0,
          "remove permission fixtures");
  printf("fsrootboottest: permissions and ownership passed\n");

  int child = fork();
  require(child >= 0, "fork executable reader");
  if (!child)
  {
    char *args[] = {const_cast<char *>("/bin/echo"),
                    const_cast<char *>("fsrootboottest: executable read through service"),
                    nullptr};
    exec(args[0], args);
    failed("exec from root service");
  }
  int status = -1;
  require(wait(&status) == child && status == 0, "wait executable reader");
  int pty_checks = fork();
  require(pty_checks >= 0, "fork full named PTY regression");
  if (!pty_checks)
  {
    char *args[] = {const_cast<char *>("/usr/tests/namedptytest"), nullptr};
    exec(args[0], args);
    failed("exec named PTY regression");
  }
  require(wait(&status) == pty_checks && status == 0, "full named PTY regression");
  require(sync() == 0, "global service sync");
  require(kill(device_server) == 0 && wait(&status) == device_server,
          "stop user device backend");
  require(open("/dev/null", O_RDWR) < 0, "disconnected devfs returns failure");
  require(kill(server) == 0, "kill root backend");
  require(wait(&status) == server, "reap root backend");
  require(open("/bin/echo", O_RDONLY) < 0, "disconnected root returns failure");
  require(getpid() == 1, "kernel remains responsive after root backend failure");
  printf("fsrootboottest: backend failure contained\n");
  printf("fsrootboottest: PASS: persistent root served entirely by a user process\n");
  for (;;) pause(1);
}
