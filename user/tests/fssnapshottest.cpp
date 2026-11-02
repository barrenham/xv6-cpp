#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "user/runtime/userfs/server.h"

namespace
{
void check(bool okay, const char *message)
{
  if (!okay) { fprintf(2, "fssnapshottest: FAIL: %s\n", message); exit(1); }
}
class Backend final : public userfs::Backend
{
  xv::userfs::RequestContext actor_, captured_;
  unsigned generation_ = 0, active_ = 0;
public:
  unsigned flags() override { return xv::userfs::ReadOnly | xv::userfs::Snapshots; }
  void context(const xv::userfs::RequestContext &context) override { actor_ = context; }
  int getattr(unsigned node, userfs::Attributes &out) override
  {
    if (node != 1 && node != 2) return -1;
    out = {node, 0, 0, 0, node == 1 ? 0555U : 0444U,
            node == 1 ? unsigned(T_DIR) : unsigned(T_FILE), 1};
    return 0;
  }
  int lookup(unsigned node, const char *name, userfs::Attributes &out) override
  {
    if (node != 1) return -1;
    if (!strcmp(name, ".") || !strcmp(name, "..")) return getattr(1, out);
    return !strcmp(name, "data") ? getattr(2, out) : -1;
  }
  int snapshot(unsigned node, unsigned &size) override
  {
    if (node != 2 || active_) return -1;
    active_ = ++generation_;
    captured_ = actor_;
    size = 8193;
    return 0;
  }
  int snapshot_read(unsigned long offset, void *data, unsigned count) override
  {
    if (!active_) return -1;
    if (active_ == 1) return 0; // Premature EOF must fail only this open.
    if (offset >= 8193) return 0;
    if (count > 8193 - offset) count = 8193 - offset;
    unsigned prefix[] = {captured_.pid, captured_.uid, captured_.gid};
    auto *bytes = static_cast<unsigned char *>(data);
    for (unsigned i = 0; i < count; ++i)
      bytes[i] = offset + i < sizeof(prefix)
          ? reinterpret_cast<unsigned char *>(prefix)[offset + i]
          : static_cast<unsigned char>((offset + i + active_ * 17) & 127);
    return count;
  }
  void snapshot_end() override { active_ = 0; }
};
}
extern "C" int main()
{
  check(mkdir("/tmp/snapshot-test") == 0, "mount directory");
  int ready[2]; check(pipe(ready) == 0, "service gate");
  int server = fork(); check(server >= 0, "fork snapshot service");
  if (!server)
  {
    close(ready[0]);
    Backend backend;
    int endpoint = userfs::register_backend("snapshot-test", backend);
    check(endpoint >= 0 && write(ready[1], "R", 1) == 1, "register snapshot service");
    close(ready[1]);
    int result = userfs::serve(endpoint, backend);
    close(endpoint); exit(result < 0);
  }
  close(ready[1]); char signal;
  check(read(ready[0], &signal, 1) == 1, "service ready"); close(ready[0]);
  check(mount("snapshot-test", "/tmp/snapshot-test") == 0, "mount snapshot service");
  int baseline = dup(0); check(baseline >= 0, "descriptor baseline"); close(baseline);
  check(open("/tmp/snapshot-test/data", O_RDONLY) < 0, "premature snapshot EOF rejects open");
  int first = open("/tmp/snapshot-test/data", O_RDONLY);
  check(first == baseline, "failed capture cleans up descriptor and backend state");
  int second = open("/tmp/snapshot-test/data", O_RDONLY);
  check(second >= 0, "independent second capture");
  struct stat metadata{};
  check(fstat(first, &metadata) == 0 && metadata.size == 8193, "per-open snapshot size");
  unsigned identity[3]{};
  check(read(first, reinterpret_cast<void *>(~0UL), 1) < 0 &&
        read(first, identity, sizeof(identity)) == sizeof(identity) &&
        identity[0] == unsigned(getpid()) && identity[1] == unsigned(geteuid()),
        "original actor identity and copy failure preserve offset");
  char one, two;
  check(lseek(first, 5000, SEEK_SET) == 5000 && lseek(second, 5000, SEEK_SET) == 5000 &&
        read(first, &one, 1) == 1 && read(second, &two, 1) == 1 &&
        one == char((5000 + 2 * 17) & 127) && two == char((5000 + 3 * 17) & 127),
        "opened snapshots do not replace each other's pages");
  int alias = dup(first); check(alias >= 0, "snapshot duplicate");
  check(read(alias, &one, 1) == 1 && lseek(first, 0, SEEK_CUR) == 5002,
        "snapshot duplicates share offset");
  close(alias);
  int child = fork(); check(child >= 0, "actor fork");
  if (!child)
  {
    close(first); close(second);
    check(setuid(1000) == 0, "actor uid");
    int own = open("/tmp/snapshot-test/data", O_RDONLY);
    unsigned identity[3]{};
    check(own >= 0 && read(own, identity, sizeof(identity)) == sizeof(identity) &&
          identity[0] == unsigned(getpid()) && identity[1] == 1000,
          "service sees client credentials rather than its own");
    close(own); exit(0);
  }
  int status;
  check(wait(&status) == child && status == 0, "actor result");
  check(lseek(first, 0, SEEK_SET) == 0 && read(first, identity, sizeof(identity)) == sizeof(identity) &&
        identity[0] == unsigned(getpid()) && identity[1] == 0, "old snapshot keeps original actor");
  close(first); close(second);
  check(umount("/tmp/snapshot-test") == 0 && wait(&status) == server && status == 0 &&
        unlink("/tmp/snapshot-test") == 0, "snapshot service teardown");
  printf("fssnapshottest: capture errors, independent opens and actor identity passed\n");
  exit(0);
}
