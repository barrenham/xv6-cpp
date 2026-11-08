#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/userfs/protocol.h"
import xv.userfs;
extern "C" int __xv_gettid();

namespace
{
void
check(bool okay, const char *why) {
  if (!okay)
  {
    fprintf(2, "fsworkertest: FAIL: %s errno=%d\n", why, errno);
    exit(1);
  }
}

struct Probe
{
  unsigned entered = 0, active = 0, peak = 0, gate = 1, writes = 0, violations = 0;
  int readers[2]{}, actors[2]{};
};

class Backend final : public userfs::Backend
{
  Probe &probe_;
  int owner_ = getpid();
  char *values_ = nullptr;

public:

  explicit Backend(Probe &probe) : probe_(probe) {
    values_ = static_cast<char *>(malloc(16384));
    check(values_ != nullptr, "backend contents");
    memset(values_, 'A', 8192);
    memset(values_ + 8192, 'B', 8192);
  }

  ~Backend() {
    free(values_);
  }

  unsigned
  cache_flags() const override {
    return xv::userfs::CacheMetadata;
  }

  bool
  concurrent_data() const override {
    return true;
  }

  userfs::Attributes
  root() override {
    return {1, 0, 0, 0, 0777, T_DIR, 2};
  }

  int
  getattr(unsigned node, userfs::Attributes &out) override {
    if (__atomic_load_n(&probe_.active, __ATOMIC_ACQUIRE))
    {
      ++probe_.violations;
    }
    if (node < 1 || node > 3)
    {
      return -1;
    }
    out = node == 1 ? root() : userfs::Attributes{node, 8192, 0, 0, 0777, T_FILE, 1};
    return 0;
  }

  int
  lookup(unsigned directory, const char *name, userfs::Attributes &out) override {
    return directory != 1 ? -1
                          : getattr(!strcmp(name, "a")   ? 2
                                    : !strcmp(name, "b") ? 3
                                                         : 0,
                                    out);
  }

  int
  read(unsigned node, unsigned long offset, void *out, unsigned count) override {
    if (node < 2 || node > 3 || offset > 8192 || count > 8192 - offset)
    {
      return -1;
    }
    if (getpid() != owner_ || __xv_gettid() == owner_)
    {
      ++probe_.violations;
    }
    unsigned active = __atomic_add_fetch(&probe_.active, 1U, __ATOMIC_ACQ_REL);
    unsigned peak = __atomic_load_n(&probe_.peak, __ATOMIC_RELAXED);
    while (active > peak &&
           !__atomic_compare_exchange_n(&probe_.peak, &peak, active, false,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED))
    {
    }
    auto actor = userfs::current_context().pid;
    unsigned slot = __atomic_fetch_add(&probe_.entered, 1U, __ATOMIC_ACQ_REL);
    if (slot < 2)
    {
      probe_.readers[slot] = __xv_gettid();
      probe_.actors[slot] = actor;
    }
    auto deadline = clockticks() + 20000000;
    while (__atomic_load_n(&probe_.gate, __ATOMIC_ACQUIRE) &&
           __atomic_load_n(&probe_.entered, __ATOMIC_ACQUIRE) < 2)
    {
      check(clockticks() < deadline,
            "both SDK read handlers must execute before either completes");
      poll(nullptr, 0, 1);
    }
    check(userfs::current_context().pid == actor,
          "request identity remains thread local across blocking calls");
    memmove(out, values_ + (node - 2) * 8192 + offset, count);
    __atomic_sub_fetch(&probe_.active, 1U, __ATOMIC_ACQ_REL);
    return count;
  }

  int
  write(unsigned node, unsigned long offset, const void *data,
        unsigned count) override {
    if (getpid() != owner_ || __atomic_load_n(&probe_.active, __ATOMIC_ACQUIRE))
    {
      ++probe_.violations;
    }
    if (node < 2 || node > 3 || !count || offset > 8192 || count > 8192 - offset)
    {
      return -1;
    }
    memmove(values_ + (node - 2) * 8192 + offset, data, count);
    ++probe_.writes;
    return count;
  }
};
} // namespace

extern "C" int
main() {
  int memory = shm_open("/fsworker-test", O_CREAT | O_EXCL | O_RDWR, 0600);
  check(memory >= 0 && shm_unlink("/fsworker-test") == 0 &&
            ftruncate(memory, 4096) == 0,
        "shared probe");
  auto *probe = static_cast<Probe *>(
      mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, memory, 0));
  check(probe != MAP_FAILED, "map probe");
  close(memory);
  memset(probe, 0, sizeof(*probe));
  probe->gate = 1;
  int readiness[2];
  check(pipe(readiness) == 0, "readiness pipe");
  int provider = fork();
  check(provider >= 0, "provider fork");
  if (!provider)
  {
    close(readiness[0]);
    Backend backend(*probe);
    int endpoint = userfs::register_backend("worker-test", backend);
    check(endpoint >= 0 && write(readiness[1], "R", 1) == 1, "SDK registration");
    close(readiness[1]);
    int result = userfs::serve(endpoint, backend);
    close(endpoint);
    exit(result < 0);
  }
  close(readiness[1]);
  char ready;
  check(read(readiness[0], &ready, 1) == 1 && ready == 'R', "provider ready");
  close(readiness[0]);
  check(mkdir("/tmp/workers") == 0 && mount("worker-test", "/tmp/workers") == 0,
        "mount worker backend");
  int files[2]{open("/tmp/workers/a", O_RDWR), open("/tmp/workers/a", O_RDWR)};
  check(files[0] >= 0 && files[1] >= 0, "open worker files");
  check(lseek(files[1], 4096, SEEK_SET) == 4096, "independent page in the same inode");
  char *views[2]{};
  for (unsigned i = 0; i < 2; ++i)
  {
    views[i] =
        static_cast<char *>(mmap(nullptr, 8192, PROT_READ, MAP_PRIVATE, files[i], 0));
    check(views[i] != MAP_FAILED, "reserve lazy views before starting either fault");
  }
  int children[2];
  for (unsigned i = 0; i < 2; ++i)
  {
    children[i] = fork();
    check(children[i] >= 0, "client fork");
    if (!children[i])
    {
      close(files[1 - i]);
      for (unsigned j = 0; j < 4096; ++j)
      {
        check(views[i][i * 4096 + j] == 'A',
              "parallel mapped fault and worker reply isolation");
      }
      close(files[i]);
      exit(0);
    }
  }
  for (int child : children)
  {
    int status;
    check(waitpid(child, &status, 0) == child && status == 0, "reader completed");
  }
  check(probe->peak == 2 && probe->readers[0] != probe->readers[1] &&
            probe->readers[0] != provider && probe->readers[1] != provider,
        "two distinct handler threads in the same process overlapped");
  check(probe->actors[0] != probe->actors[1] &&
            (probe->actors[0] == children[0] || probe->actors[0] == children[1]) &&
            (probe->actors[1] == children[0] || probe->actors[1] == children[1]),
        "concurrent callbacks retain their distinct client identities");
  probe->gate = 0;
  check(lseek(files[0], 0, SEEK_SET) == 0 && write(files[0], "Z", 1) == 1 &&
            lseek(files[0], 0, SEEK_SET) == 0 && read(files[0], &ready, 1) == 1 &&
            ready == 'Z',
        "parent write is visible in the unified cache");
  int uncached = open("/tmp/workers/b", O_RDWR);
  check(uncached >= 0 && write(uncached, "Z", 1) == 1 &&
            lseek(uncached, 0, SEEK_SET) == 0 && read(uncached, &ready, 1) == 1 &&
            ready == 'Z' && probe->entered >= 3,
        "next worker sees writes to the shared backend");
  close(uncached);
  for (auto *view : views)
  {
    check(munmap(view, 8192) == 0, "lazy view cleanup");
  }
  check(probe->writes == 2 && !probe->active && !probe->violations,
        "mutations/metadata remain serialized");
  close(files[0]);
  close(files[1]);
  check(umount("/tmp/workers") == 0 && unlink("/tmp/workers") == 0,
        "unmount worker backend");
  int status;
  check(waitpid(provider, &status, 0) == provider && status == 0,
        "workers and provider reaped");
  munmap(probe, 4096);
  printf(
      "fsworkertest: PASS: real concurrent handlers, isolated replies and metadata/write barriers\n");
  return 0;
}
