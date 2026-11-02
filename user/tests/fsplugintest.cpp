#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/mount.h"
#include "kernel/fs/userfs/protocol.h"
#include "user/runtime/user.h"
#include "user/runtime/proc.h"

namespace
{
char buffer[8192], payload[5000];
unsigned char block[512];

void check(bool ok, const char *text)
{
  if (!ok)
  {
    fprintf(2, "fsplugintest: FAIL: %s\n", text);
    exit(1);
  }
}

bool contains(const char *text, const char *needle)
{
  unsigned n = strlen(needle), length = strlen(text);
  for (unsigned i = 0; i + n <= length; ++i)
  {
    if (!memcmp(text + i, needle, n))
    {
      return true;
    }
  }
  return false;
}

unsigned providers(const char *type)
{
  if (proc::read("/proc/fsplugins", buffer, sizeof(buffer)) < 0)
  {
    return 0;
  }
  unsigned count = 0, n = strlen(type);
  for (char *line = buffer; line && *line;)
  {
    if (strlen(line) >= n && !memcmp(line, type, n) && line[n] == ' ')
    {
      ++count;
    }
    char *next = strchr(line, '\n');
    line = next ? next + 1 : nullptr;
  }
  return count;
}

void ready(const char *type, unsigned count)
{
  for (unsigned i = 0; i < 100; ++i)
  {
    if (providers(type) == count)
    {
      return;
    }
    pause(1);
  }
  check(false, "provider registration visible");
}

void reap(int pid)
{
  int status;
  check(waitpid(pid, &status, 0) == pid && status == 0, "service exit status");
}

int memory(const char *type)
{
  int pid = fork();
  check(pid >= 0, "fork memory service");
  if (!pid)
  {
    char *args[] = {const_cast<char *>("memfsd"), const_cast<char *>("--register"),
                    const_cast<char *>(type), const_cast<char *>("128"), nullptr};
    exec("/bin/memfsd", args);
    exit(1);
  }
  return pid;
}

int archive(const char *type, const char *image = "/plugin-fixture.tar")
{
  int pid = fork();
  check(pid >= 0, "fork archive service");
  if (!pid)
  {
    char *args[] = {const_cast<char *>("tarfsd"), const_cast<char *>(image),
                    const_cast<char *>(type), nullptr};
    exec("/bin/tarfsd", args);
    exit(1);
  }
  return pid;
}

void octal(unsigned at, unsigned width, unsigned long value)
{
  block[at + width - 1] = 0;
  for (unsigned i = width - 1; i;)
  {
    --i;
    block[at + i] = '0' + (value & 7);
    value >>= 3;
  }
}

void header(int fd, const char *name, unsigned size, char type,
            const char *target = nullptr)
{
  memset(block, 0, sizeof(block));
  strcpy(reinterpret_cast<char *>(block), name);
  octal(100, 8, type == '5' ? 0755 : 0644);
  octal(124, 12, size);
  block[156] = type;
  if (target)
  {
    strcpy(reinterpret_cast<char *>(block + 157), target);
  }
  strcpy(reinterpret_cast<char *>(block + 257), "ustar");
  block[263] = block[264] = '0';
  memset(block + 148, ' ', 8);
  unsigned sum = 0;
  for (unsigned char c : block)
  {
    sum += c;
  }
  octal(148, 7, sum);
  block[155] = ' ';
  check(write(fd, block, sizeof(block)) == sizeof(block), "archive header write");
}

void fixture()
{
  int fd = open("/plugin-fixture.tar", O_CREATE | O_RDWR | O_TRUNC);
  check(fd >= 0, "persistent archive file");
  header(fd, "dir/", 0, '5');
  header(fd, "dir/large.bin", sizeof(payload), '0');
  for (unsigned i = 0; i < sizeof(payload); ++i)
  {
    payload[i] = i * 37;
  }
  check(write(fd, payload, sizeof(payload)) == sizeof(payload), "archive file payload");
  memset(block, 0, sizeof(block));
  check(write(fd, block, 512 - sizeof(payload) % 512) == 512 - sizeof(payload) % 512,
        "archive padding");
  header(fd, "alias", 0, '2', "dir/large.bin");
  memset(block, 0, sizeof(block));
  check(write(fd, block, 512) == 512 && write(fd, block, 512) == 512 && fsync(fd) == 0,
        "archive persisted");
  close(fd);
}

void capacity_contract()
{
  xv::userfs::Registration registration;
  strcpy(registration.type, "capacity");
  int held[8], ready_pipe[2], release_pipe[2];
  for (auto &fd : held)
  {
    fd = fsregister(&registration);
    check(fd >= 0, "capacity initial providers");
  }
  check(pipe(ready_pipe) == 0 && pipe(release_pipe) == 0, "capacity barriers");
  int child = fork();
  check(child >= 0, "capacity child");
  if (!child)
  {
    for (int fd : held)
    {
      close(fd);
    }
    close(ready_pipe[0]);
    close(release_pipe[1]);
    int own[8];
    for (auto &fd : own)
    {
      if ((fd = fsregister(&registration)) < 0)
      {
        exit(1);
      }
    }
    char byte = '!';
    if (write(ready_pipe[1], &byte, 1) != 1 || read(release_pipe[0], &byte, 1) != 1)
    {
      exit(2);
    }
    for (int fd : own)
    {
      close(fd);
    }
    exit(0);
  }
  close(ready_pipe[1]);
  close(release_pipe[0]);
  char byte;
  check(read(ready_pipe[0], &byte, 1) == 1 &&
            providers("capacity") == xv::userfs::ProviderCapacity,
        "all provider slots occupied");
  check(fsregister(&registration) < 0, "registry capacity failure is clean");
  check(write(release_pipe[1], "!", 1) == 1, "release capacity child");
  close(ready_pipe[0]);
  close(release_pipe[1]);
  reap(child);
  int fd = fsregister(&registration);
  check(fd >= 0, "capacity available after provider exit");
  close(fd);
  for (int old : held)
  {
    close(old);
  }
  check(providers("capacity") == 0, "all capacity providers gone");
}

void dead_parent_contract()
{
  int parent = memory("parent-plugin");
  ready("parent-plugin", 1);
  check(mount("parent-plugin", "/tmp/plugin-a") == 0 &&
            mkdir("/tmp/plugin-a/child") == 0,
        "plugin parent mount and child directory");
  int child = memory("nested-plugin");
  ready("nested-plugin", 1);
  check(mount("nested-plugin", "/tmp/plugin-a/child") == 0, "nested plugin mount");
  check(umount("/tmp/plugin-a") < 0, "parent mount stays busy while child mounted");
  int status;
  check(kill(parent) == 0 && waitpid(parent, &status, 0) == parent,
        "kill parent service");
  check(open("/tmp/plugin-a/missing", O_RDONLY) < 0, "dead parent rejects lookup");
  check(umount("/tmp/plugin-a/child") == 0, "unmount child without dead parent RPC");
  reap(child);
  check(umount("/tmp/plugin-a") == 0, "unmount dead parent after child cleanup");
}

void mounted_archive(const char *type)
{
  int server = archive(type);
  ready(type, 1);
  check(proc::read("/proc/filesystems", buffer, sizeof(buffer)) > 0 &&
            contains(buffer, type),
        "new type registered without kernel changes");
  check(mount(type, "/tmp/plugin-a") == 0, "generic typed archive mount");
  int fd = open("/tmp/plugin-a/dir/large.bin", O_RDONLY);
  check(fd >= 0 && read(fd, buffer, sizeof(payload)) == sizeof(payload) &&
            !memcmp(buffer, payload, sizeof(payload)),
        "disk-backed plugin read");
  check(umount("/tmp/plugin-a") < 0, "archive busy mount");
  close(fd);
  fd = open("/tmp/plugin-a/alias", O_RDONLY);
  check(fd >= 0 && read(fd, buffer, sizeof(payload)) == sizeof(payload) &&
            !memcmp(buffer, payload, sizeof(payload)),
        "archive symlink");
  close(fd);
  check(open("/tmp/plugin-a/dir/large.bin", O_WRONLY) < 0 &&
            open("/tmp/plugin-a/new", O_CREATE | O_RDWR) < 0 &&
            chmod("/tmp/plugin-a/dir/large.bin", 0666) < 0,
        "kernel readonly policy");
  check(proc::read("/proc/mounts", buffer, sizeof(buffer)) > 0 &&
            contains(buffer, type) && contains(buffer, " ro 0 0"),
        "named readonly mount visible");
  check(umount("/tmp/plugin-a") == 0, "archive unmount");
  reap(server);
  check(providers(type) == 0, "type unregisters on last endpoint close");
}
} // namespace

extern "C" int main()
{
  xv::userfs::Registration r;
  strcpy(r.type, "validation");
  check(fsregister(reinterpret_cast<void *>(~0UL)) < 0,
        "registration pointer validation");
  ++r.version;
  check(fsregister(&r) < 0, "protocol version validation");
  --r.version;
  --r.size;
  check(fsregister(&r) < 0, "registration size validation");
  ++r.size;
  r.flags = 128;
  check(fsregister(&r) < 0, "unsupported flags");
  r.flags = xv::userfs::Snapshots;
  check(fsregister(&r) < 0, "snapshot capture requires a read-only backend");
  r.flags |= xv::userfs::ReadOnly;
  r.operations = xv::userfs::ReadOperations;
  check(fsregister(&r) < 0, "snapshot capture requires complete operation capabilities");
  r.flags = 0;
  r.operations = 0;
  check(fsregister(&r) < 0, "required operations");
  r.operations = xv::userfs::AllOperations;
  r.max_payload = 0;
  check(fsregister(&r) < 0, "zero payload rejected");
  r.max_payload = xv::userfs::DataSize + 1;
  check(fsregister(&r) < 0, "oversized payload rejected");
  r.max_payload = xv::userfs::DataSize;
  r.root.node = 2;
  check(fsregister(&r) < 0, "invalid root identity");
  r.root.node = 1;
  strcpy(r.type, "ext3");
  check(fsregister(&r) < 0, "builtin name collision");
  strcpy(r.type, "not/a/type");
  check(fsregister(&r) < 0, "type name validation");
  strcpy(r.type, "close-cycle");
  for (unsigned i = 0; i < xv::userfs::ProviderCapacity + 5; ++i)
  {
    int fd = fsregister(&r);
    check(fd >= 0, "dynamic provider allocation");
    check(providers(r.type) == 1, "registered idle endpoint");
    close(fd);
    check(providers(r.type) == 0, "closed provider removed");
  }
  capacity_contract();
  int child = fork();
  check(child >= 0, "permission child");
  if (!child)
  {
    if (setuid(1000) || fsregister(&r) >= 0)
    {
      exit(1);
    }
    exit(0);
  }
  reap(child);
  mkdir("/tmp/plugin-a");
  mkdir("/tmp/plugin-b");
  const char *type = "memory-plugin-long-name";
  int a = memory(type), b = memory(type);
  ready(type, 2);
  check(mount(type, "/tmp/no-such-point") < 0 && providers(type) == 2,
        "failed mount preserves registrations");
  child = fork();
  check(child >= 0, "mount utility child");
  if (!child)
  {
    char *args[] = {const_cast<char *>("mount"), const_cast<char *>("-t"),
                    const_cast<char *>(type), const_cast<char *>("/tmp/plugin-a"),
                    nullptr};
    exec("/bin/mount", args);
    exit(1);
  }
  reap(child);
  check(mount(type, "/tmp/plugin-b") == 0 && mount(type, "/tmp") < 0,
        "independent instances selected once");
  struct stat first{}, second{};
  check(stat("/tmp/plugin-a", &first) == 0 && stat("/tmp/plugin-b", &second) == 0 &&
            first.dev != second.dev,
        "instance identity isolation");
  int fd = open("/tmp/plugin-a/data", O_CREATE | O_RDWR);
  check(fd >= 0 && write(fd, "one", 3) == 3, "registered memory backend write");
  for (unsigned i = 0; i < sizeof(payload); ++i)
  {
    payload[i] = i * 37;
  }
  check(lseek(fd, 0, SEEK_SET) == 0 &&
            write(fd, payload, sizeof(payload)) == sizeof(payload) &&
            lseek(fd, 0, SEEK_SET) == 0 &&
            read(fd, buffer, sizeof(payload)) == sizeof(payload) &&
            !memcmp(buffer, payload, sizeof(payload)),
        "negotiated 128-byte request cap");
  close(fd);
  check(open("/tmp/plugin-b/data", O_RDONLY) < 0, "instance content isolation");
  check(umount("/tmp/plugin-a") == 0 && umount("/tmp/plugin-b") == 0,
        "memory plugins unmount");
  reap(a);
  reap(b);
  dead_parent_contract();
  fixture();
  mounted_archive("archive-plugin-a");
  mounted_archive("another-archive-implementation");
  int sample = archive("bundled-plugin", "/usr/share/doc/xv6/plugin.tar");
  ready("bundled-plugin", 1);
  check(mount("bundled-plugin", "/tmp/plugin-a") == 0, "bundled plugin mount");
  fd = open("/tmp/plugin-a/hello.txt", O_RDONLY);
  int count = fd < 0 ? -1 : read(fd, buffer, sizeof(buffer) - 1);
  check(count > 0, "bundled plugin contents");
  buffer[count] = 0;
  check(contains(buffer, "independently registered archive filesystem"),
        "bundled sample payload");
  close(fd);
  check(umount("/tmp/plugin-a") == 0, "bundled sample unmount");
  reap(sample);
  check(unlink("/plugin-fixture.tar") == 0 && unlink("/tmp/plugin-a") == 0 &&
            unlink("/tmp/plugin-b") == 0,
        "plugin fixture cleanup");
  printf("fsplugintest: all tests succeeded\n");
  exit(0);
}
