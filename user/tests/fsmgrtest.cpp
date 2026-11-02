#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/mount.h"
#include "kernel/io/fs_resources.h"
#include "kernel/fs/userfs/protocol.h"
#include "user/runtime/proc.h"
#include "user/runtime/user.h"

namespace
{
void check(bool okay, const char *message)
{
  if (!okay) { fprintf(2, "fsmgrtest: FAIL: %s\n", message); exit(1); }
}
void reap(int pid = -1)
{
  int status;
  check(waitpid(pid, &status, 0) > 0 && !status, "client result");
}
int create(const char *path, const char *text)
{
  int fd = open(path, O_CREATE | O_RDWR);
  check(fd >= 0 && write(fd, text, strlen(text)) == int(strlen(text)), "write mounted file");
  return fd;
}
void content(const char *path, const char *expected)
{
  char text[32]{};
  int fd = open(path, O_RDONLY);
  check(fd >= 0 && read(fd, text, sizeof(text)) == int(strlen(expected)) &&
        !memcmp(text, expected, strlen(expected)), "mounted file contents");
  close(fd);
}
void memory_instances()
{
  constexpr char first[] = "/tmp/mgr-a", second[] = "/tmp/mgr-b";
  check(mkdir(first) == 0 && mkdir(second) == 0, "covered directories");
  close(create("/tmp/mgr-a/base", "covered"));
  check(mount("ramfs", first) == 0 && mount("ramfs", second) == 0,
        "mount creates two independent RAM services");
  struct stat a{}, b{};
  check(stat(first, &a) == 0 && stat(second, &b) == 0 && a.dev != b.dev &&
        (a.mode & 0777) == 0755 && a.uid == 0, "instance identity and root attributes");
  int fd = create("/tmp/mgr-a/one", "first");
  check(stat("/tmp/mgr-b/one", &b) < 0 && umount(first) < 0, "isolation and busy descriptor");
  close(fd);
  check(chdir(first) == 0 && umount(first) < 0 && chdir("/") == 0, "cwd blocks unmount");
  check(chmod("/tmp/mgr-a/one", 0600) == 0, "private file permissions");
  int child = fork();
  check(child >= 0, "fork unprivileged mount client");
  if (!child)
  {
    check(seteuid(1000) == 0 && mount("ramfs", "/tmp/mgr-b") < 0 &&
          open("/tmp/mgr-a/one", O_RDONLY) < 0, "mount authorization and file permissions");
    exit(0);
  }
  reap(child);
  check(mkdir("/tmp/mgr-a/nest") == 0 && mount("ramfs", "/tmp/mgr-a/nest") == 0 &&
        umount(first) < 0, "nested instance pins parent");
  check(umount("/tmp/mgr-a/nest") == 0 && umount(first) == 0 && umount(second) == 0,
        "nested instance teardown");
  content("/tmp/mgr-a/base", "covered");
  check(mount("devfs", first) == 0, "create independent device service");
  char zeros[17];
  fd = open("/tmp/mgr-a/zero", O_RDONLY);
  check(fd >= 0 && read(fd, zeros, sizeof(zeros)) == sizeof(zeros), "device proxy in dynamic instance");
  for (char value : zeros) check(!value, "zero device bytes");
  close(fd);
  check(umount(first) == 0 && mount("procfs", second) == 0, "retire devfs and create procfs service");
  fd = open("/tmp/mgr-b/self/comm", O_RDONLY);
  char name[32]{};
  check(fd >= 0 && read(fd, name, sizeof(name)) > 0 && !memcmp(name, "fsmgrtest", 9),
        "dynamic procfs resolves original client");
  close(fd);
  check(open("/tmp/mgr-b/new", O_CREATE | O_WRONLY) < 0 && umount(second) == 0,
        "dynamic procfs remains read only and can retire");
  check(mount("unknown-plugin", first) < 0 && mount("ramfs", "/tmp/no-such-dir") < 0,
        "unsupported type and bad mountpoint fail");
  for (unsigned round = 0; round < 24; ++round)
  {
    check(mount("ramfs", second) == 0 && umount(second) == 0, "repeated service instance recycling");
    pause(1); // Teardown is asynchronous; let the service close its endpoint.
  }
  for (unsigned worker = 0; worker < 4; ++worker)
  {
    char path[] = "/tmp/mgr-par0";
    path[sizeof(path) - 2] += worker;
    check(mkdir(path) == 0, "parallel covered directory");
    child = fork();
    check(child >= 0, "fork parallel mount client");
    if (!child)
    {
      check(mount("ramfs", path) == 0 && chdir(path) == 0, "parallel mount");
      close(create("file", "parallel"));
      content("file", "parallel");
      check(chdir("/") == 0 && umount(path) == 0, "parallel unmount");
      exit(0);
    }
  }
  for (unsigned worker = 0; worker < 4; ++worker) reap();
}
void disk_instance(const char *type, const char *phase)
{
  constexpr char path[] = "/tmp/mgrdisk", marker[] = "/tmp/mgrdisk/keep";
  check(mkdir(path) == 0 && mount(type, path) == 0, "create optional disk service");
  const char *other = !strcmp(type, "ext3") ? "xv6" : "ext3";
  check(mount(type, "/tmp/mgr-a") < 0 && mount(other, "/tmp/mgr-b") < 0,
        "mounted disk cannot receive a second write capability");
  if (!strcmp(phase, "first"))
  {
    int fd = create(marker, "disk-persistent");
    check(fsync(fd) == 0, "flush optional disk data");
    close(fd);
  }
  else content(marker, "disk-persistent");
  check(sync() == 0 && umount(path) == 0, "flush and retire disk instance");
  check(mount(other, path) < 0, "wrong disk format rejected");
  bool mounted = false;
  for (unsigned retry = 0; retry < 100 && !mounted; ++retry)
  {
    mounted = mount(type, path) == 0;
    if (!mounted) pause(1);
  }
  check(mounted, "disk lease returns after serving process exits");
  content(marker, "disk-persistent");
  check(umount(path) == 0, "remounted disk teardown");
}
unsigned memory_providers()
{
  static char plugins[4096];
  check(proc::read("/proc/fsplugins", plugins, sizeof(plugins)) >= 0, "provider lifecycle observation");
  unsigned count = 0;
  for (char *line = plugins; line && *line;)
  {
    if (strlen(line) >= 6 && !memcmp(line, "ramfs ", 6))
    {
      char *state = strchr(line + 6, ' ');
      if (state && memcmp(state + 1, "stopped ", 8)) ++count;
    }
    char *next = strchr(line, '\n');
    line = next ? next + 1 : nullptr;
  }
  return count;
}
void cancel_publication(int manager)
{
  unsigned baseline = memory_providers();
  check(mkdir("/tmp/mgr-stop") == 0, "publication fault mountpoint");
  int ready[2], blocked[2], release[2];
  check(pipe(ready) == 0 && pipe(blocked) == 0 && pipe(release) == 0, "publication fault channels");
  int server = fork();
  check(server >= 0, "fork blocking parent backend");
  if (!server)
  {
    close(ready[0]); close(blocked[0]); close(release[1]);
    int endpoint = fsserve("/tmp/mgr-stop");
    check(endpoint >= 0 && write(ready[1], "R", 1) == 1, "blocking parent mounted");
    static xv::userfs::Message request;
    check(read(endpoint, &request, sizeof(request)) == sizeof(request) &&
          request.operation == xv::userfs::Operation::Lookup && !strcmp(request.name, "nest"),
          "manager reached unpublished service mountpoint");
    check(write(blocked[1], "B", 1) == 1, "notify blocked publication");
    char byte;
    check(read(release[0], &byte, 1) == 1, "release blocking backend");
    close(endpoint);
    exit(0);
  }
  close(ready[1]); close(blocked[1]); close(release[0]);
  char byte;
  check(read(ready[0], &byte, 1) == 1, "blocking parent readiness");
  int requester = fork();
  check(requester >= 0, "fork canceled mount client");
  if (!requester)
  {
    close(ready[0]); close(blocked[0]); close(release[1]);
    check(mount("ramfs", "/tmp/mgr-stop/nest") < 0, "manager death wakes pending mount client");
    exit(0);
  }
  check(read(blocked[0], &byte, 1) == 1 && kill(manager) == 0,
        "terminate manager before publication finishes");
  reap(requester);
  check(write(release[1], "X", 1) == 1, "release parent service");
  close(ready[0]); close(blocked[0]); close(release[1]);
  reap(server);
  check(umount("/tmp/mgr-stop") == 0, "remove disconnected parent service");
  bool retired = false;
  for (unsigned retry = 0; retry < 100 && !retired; ++retry)
  {
    retired = memory_providers() == baseline;
    if (!retired) pause(1);
  }
  check(retired, "unpublished service retires after manager death");
}
}

extern "C" int main(int argc, char **argv)
{
  check(argc >= 2, "test mode");
  check(ioctl(4, xv::fsservice::StorageIoctl, 1) < 0, "ordinary apps inherit no resource capability");
  if (!strcmp(argv[1], "stop") || !strcmp(argv[1], "cancel"))
  {
    check(argc == 3, "manager identity");
    if (!strcmp(argv[1], "cancel")) cancel_publication(atoi(argv[2]));
    else check(kill(atoi(argv[2])) == 0 && mkdir("/tmp/mgr-stop") == 0, "stop mount manager");
    check(mount("ramfs", "/tmp/mgr-stop") < 0,
          "dead manager returns mount failure");
    int fd = open("/etc/passwd", O_RDONLY);
    check(fd >= 0, "existing root remains usable after manager death");
    close(fd);
    check(umount("/") < 0, "root stays protected");
    // One TTY write keeps the marker intact alongside PID 1's death report.
    constexpr char passed[] = "fsmgrtest: PASS: stop\n";
    check(write(1, passed, sizeof(passed) - 1) == sizeof(passed) - 1, "stop report");
    exit(0);
  }
  memory_instances();
  if (strcmp(argv[1], "basic"))
  {
    check(argc == 3 && (!strcmp(argv[1], "ext3") || !strcmp(argv[1], "xv6")), "disk mode");
    disk_instance(argv[1], argv[2]);
  }
  else check(mount("ext3", "/tmp/mgr-a") < 0 && mount("xv6", "/tmp/mgr-b") < 0,
             "absent optional disk fails cleanly");
  printf("fsmgrtest: PASS: %s\n", argv[1]);
  exit(0);
}
