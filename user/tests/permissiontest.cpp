#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"
namespace
{
void check(bool ok, const char *message) { if (!ok) { fprintf(2, "permissiontest: FAIL: %s\n", message); exit(1); } }
void reap() { int status; check(wait(&status) > 0 && status == 0, "child result"); }
void file(const char *path) { int fd = open(path, O_CREATE | O_RDWR); check(fd >= 0 && write(fd, "secret", 6) == 6, "fixture file"); close(fd); }
void drop(unsigned uid, unsigned gid)
{ check(setgroups(0, nullptr) == 0 && setgid(gid) == 0 && setuid(uid) == 0, "drop privileges"); }
}
extern "C" int main()
{
  check(getuid() == 0 && geteuid() == 0, "root test launcher");
  unsigned saved_groups[16], sampled[16];
  int saved_count = getgroups(16, saved_groups);
  auto *invalid = reinterpret_cast<unsigned *>(~0UL);
  check(saved_count >= 0 && getgroups(0, invalid) == saved_count, "group count query skips pointer");
  unsigned fixture_groups[] = {1234, 2345};
  check(setgroups(2, fixture_groups) == 0, "group boundary fixture");
  sampled[0] = 9999;
  check(getgroups(1, sampled) == -1 && sampled[0] == 9999, "short group output untouched");
  check(getgroups(2, invalid) == -1 && setgroups(2, invalid) == -1 &&
        setgroups(17, invalid) == -1 && setgroups(-1, invalid) == -1, "group copy and count bounds");
  check(getgroups(16, sampled) == 2 && sampled[0] == 1234 && sampled[1] == 2345,
        "failed group input preserves credentials");
  check(setgroups(0, invalid) == 0 && getgroups(16, invalid) == 0,
        "empty groups skip user copies");
  check(setgroups(saved_count, saved_groups) == 0, "restore original groups");
  register unsigned long unknown_number asm("a7") = 0x7fffffff;
  register unsigned long unknown_result asm("a0") = 123;
  asm volatile("ecall" : "+r"(unknown_result) : "r"(unknown_number) : "memory");
  check(unknown_result == ~0UL, "unknown syscall result");
  unsigned old_mask = umask(0027);
  file("/perm-file"); struct stat metadata{};
  check(stat("/perm-file", &metadata) == 0 && metadata.uid == 0 && metadata.mode == 0640, "default mask and ownership");
  check(chown("/perm-file", 1000, 100) == 0 && chmod("/perm-file", 0640) == 0, "owner assignment");
  check(mkdir("/perm-dir") == 0 && chown("/perm-dir", 1000, 100) == 0 && chmod("/perm-dir", 0700) == 0, "private directory");
  file("/perm-dir/data"); file("/tmp/sticky-victim");
  check(chown("/tmp/sticky-victim", 1000, 100) == 0 && chmod("/tmp/sticky-victim", 0600) == 0, "sticky fixture");
  int master = posix_openpt(O_RDWR); char slave[32]; check(master >= 0 && unlockpt(master) == 0 && ptsname_r(master, slave, sizeof(slave)) == 0, "foreign PTY fixture");
  int parent = getpid();
  check(seteuid(1001) == 0 && open("/perm-dir/data", O_RDONLY) < 0 && access("/perm-dir/data", R_OK) == 0 && seteuid(0) == 0,
        "access uses real IDs including path search");
  int child = fork(); check(child >= 0, "owner fork");
  if (!child)
  {
    unsigned groups[] = {200}; check(setgroups(1, groups) == 0 && setgid(100) == 0 && setuid(1000) == 0, "owner credentials");
    check(getuid() == 1000 && geteuid() == 1000 && getgid() == 100, "credential queries");
    check(access("/perm-file", R_OK | W_OK) == 0 && chmod("/perm-file", 0600) == 0 && chown("/perm-file", 1001, -1) < 0,
          "owner chmod but no ownership transfer");
    check(chown("/perm-file", -1, 200) == 0 && chown("/perm-file", -1, 201) < 0, "owner group membership check");
    umask(0077); file("/tmp/owned-file");
    check(stat("/tmp/owned-file", &metadata) == 0 && metadata.uid == 1000 && metadata.gid == 100 && metadata.mode == 0600, "user file credentials and umask");
    check(unlink("/tmp/owned-file") == 0 && unlink("/tmp/sticky-victim") == 0, "owner sticky delete");
    int own_master = posix_openpt(O_RDWR); char own_path[32];
    check(own_master >= 0 && unlockpt(own_master) == 0 && ptsname_r(own_master, own_path, sizeof(own_path)) == 0, "user PTY creation");
    int own_slave = open(own_path, O_RDWR);
    check(own_slave >= 0 && fstat(own_slave, &metadata) == 0 && metadata.uid == 1000 && metadata.mode == 0600, "user PTY ownership");
    close(own_slave); close(own_master);
    check(setuid(0) < 0 && seteuid(0) < 0 && setgid(0) < 0 && setgroups(0, nullptr) < 0, "no credential escalation");
    exit(0);
  }
  reap(); check(chown("/perm-file", 1000, 100) == 0 && chmod("/perm-file", 0640) == 0, "restore group fixture");
  file("/tmp/sticky-victim"); check(chown("/tmp/sticky-victim", 1000, 100) == 0, "restore sticky fixture");
  child = fork(); check(child >= 0, "foreign fork");
  if (!child)
  {
    drop(1001, 101); setenv("USER", "root", 1);
    check(stat("/perm-file", &metadata) == 0 && metadata.uid == 1000, "metadata visible without data access");
    check(open("/perm-file", O_RDONLY) < 0 && chmod("/perm-file", 0777) < 0 && chown("/perm-file", 1001, -1) < 0, "foreign file protected");
    check(stat("/perm-dir/data", &metadata) < 0 && mkdir("/forbidden") < 0 && open("/etc/passwd", O_WRONLY) < 0, "directory and account database protected");
    check(unlink("/tmp/sticky-victim") < 0, "sticky directory protected");
    check(open(slave, O_RDWR) < 0 && open("/dev/console", O_RDWR) < 0, "foreign PTY and direct console protected");
    check(kill(parent) < 0 && sigsend(parent, 0) < 0, "foreign process signal denied");
    char procpath[48]; strcpy(procpath, "/proc/1/fd"); check(open(procpath, O_RDONLY) < 0, "foreign proc descriptors protected");
    check(mknod("/tmp/evildev", 1, 0) < 0 && chmod("/tmp", 0777) < 0, "administrative operations denied");
    netctl_request request{}; request.version = NETCTL_VERSION; request.size = sizeof(request); request.ifindex = 1;
    check(netctl(NETCTL_SET_LINK, &request) < 0, "network mutation denied");
    int sock = socket(AF_INET, SOCK_DGRAM, 0); check(sock >= 0, "ordinary socket allowed");
    sockaddr_in address{}; address.sin_family = AF_INET; address.sin_port = htons(80);
    check(bind(sock, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0, "privileged port denied"); close(sock);
    exit(0);
  }
  reap();
  child = fork(); check(child >= 0, "group fork");
  if (!child)
  {
    unsigned groups[] = {100}; check(setgroups(1, groups) == 0 && setgid(101) == 0 && setuid(1002) == 0, "supplementary group");
    int fd = open("/perm-file", O_RDONLY); check(fd >= 0 && open("/perm-file", O_WRONLY) < 0, "supplementary read without write"); close(fd); exit(0);
  }
  reap();
  check(chmod("/bin/echo", 0644) == 0, "remove execute bits");
  char *arguments[] = {const_cast<char *>("echo"), nullptr}; check(exec("/bin/echo", arguments) < 0 && chmod("/bin/echo", 0755) == 0, "execute permission required even for root");
  check(chmod("/perm-file", 010000) < 0 && setuid(65535) < 0, "unsupported special bits and reserved ID rejected");
  check(sync() == 0 && stat("/perm-file", &metadata) == 0 && metadata.uid == 1000 && metadata.gid == 100 && metadata.mode == 0640, "disk metadata retained");
  close(master); unlink("/tmp/sticky-victim"); unlink("/perm-file"); unlink("/perm-dir/data"); unlink("/perm-dir");
  umask(old_mask);
  printf("permissiontest: all tests succeeded\n"); exit(0);
}
