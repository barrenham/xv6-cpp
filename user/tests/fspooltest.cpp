#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/io/fs_control.h"
import xv.userfs;
#include "user/include/pthread.h"
extern "C" int __xv_gettid();

namespace
{
constexpr unsigned Size = 65536;

void
check(bool okay, const char *why) {
  if (!okay)
  {
    fprintf(2, "fspooltest: FAIL: %s errno=%d\n", why, errno);
    exit(1);
  }
}

struct Probe
{
  unsigned active = 0, peak = 0, write_peak = 0, active_writes = 0;
  unsigned entered[2]{}, node_active[2]{}, node_writes[2]{};
  unsigned gate = 0, hold = 0, violations = 0, calls[2]{}, metadata_done = 0,
           failed = 0;
  int workers[2]{};
  int abort_tid = 0;
  char values[2][Size];
};

void
peak(unsigned &value, unsigned count) {
  unsigned old = __atomic_load_n(&value, __ATOMIC_RELAXED);
  while (old < count &&
         !__atomic_compare_exchange_n(&value, &old, count, false, __ATOMIC_RELAXED,
                                      __ATOMIC_RELAXED))
  {
  }
}

class Backend final : public userfs::Backend
{
  Probe &probe_;
  int owner_ = getpid();

  void
  enter(unsigned node, bool writing) {
    int pid = __xv_gettid();
    bool known = false;
    for (unsigned i = 0; i < 2; ++i)
    {
      int expected = 0;
      if (__atomic_load_n(&probe_.workers[i], __ATOMIC_ACQUIRE) == pid ||
          __atomic_compare_exchange_n(&probe_.workers[i], &expected, pid, false,
                                      __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
      {
        __atomic_add_fetch(&probe_.calls[i], 1U, __ATOMIC_RELAXED);
        known = true;
        break;
      }
    }
    if (!known || getpid() != owner_ || pid == owner_)
    {
      __atomic_add_fetch(&probe_.violations, 1U, __ATOMIC_RELAXED);
    }
    unsigned index = node - 2;
    unsigned active =
        __atomic_add_fetch(&probe_.node_active[index], 1U, __ATOMIC_ACQ_REL);
    if ((writing && active != 1) ||
        (!writing && __atomic_load_n(&probe_.node_writes[index], __ATOMIC_ACQUIRE)))
    {
      __atomic_add_fetch(&probe_.violations, 1U, __ATOMIC_RELAXED);
    }
    if (writing)
    {
      __atomic_add_fetch(&probe_.node_writes[index], 1U, __ATOMIC_ACQ_REL);
    }
    peak(probe_.peak, __atomic_add_fetch(&probe_.active, 1U, __ATOMIC_ACQ_REL));
    if (writing)
    {
      peak(probe_.write_peak,
           __atomic_add_fetch(&probe_.active_writes, 1U, __ATOMIC_ACQ_REL));
    }
    __atomic_add_fetch(&probe_.entered[writing], 1U, __ATOMIC_ACQ_REL);
    if (__atomic_load_n(&probe_.gate, __ATOMIC_ACQUIRE) == unsigned(writing) + 1)
    {
      auto deadline = clockticks() + 30000000;
      while (__atomic_load_n(&probe_.entered[writing], __ATOMIC_ACQUIRE) < 2 ||
             __atomic_load_n(&probe_.hold, __ATOMIC_ACQUIRE))
      {
        check(clockticks() < deadline,
              "two persistent handlers must enter before release");
        if (__atomic_load_n(&probe_.abort_tid, __ATOMIC_ACQUIRE) == pid)
        {
          __atomic_store_n(&probe_.failed, 1U, __ATOMIC_RELEASE);
          pthread_exit(nullptr);
        }
        poll(nullptr, 0, 1);
      }
    }
  }

  void
  leave(unsigned node, bool writing) {
    if (writing)
    {
      __atomic_sub_fetch(&probe_.active_writes, 1U, __ATOMIC_ACQ_REL);
      __atomic_sub_fetch(&probe_.node_writes[node - 2], 1U, __ATOMIC_ACQ_REL);
    }
    __atomic_sub_fetch(&probe_.node_active[node - 2], 1U, __ATOMIC_ACQ_REL);
    __atomic_sub_fetch(&probe_.active, 1U, __ATOMIC_ACQ_REL);
  }

public:

  explicit Backend(Probe &probe) : probe_(probe) {
  }

  bool
  concurrent_data() const override {
    return true;
  }

  unsigned long
  operations() override {
    return xv::userfs::AllOperations;
  }

  userfs::Attributes
  root() override {
    return {1, 0, 0, 0, 0777, T_DIR, 2};
  }

  int
  getattr(unsigned node, userfs::Attributes &out) override {
    if (getpid() != owner_ || __atomic_load_n(&probe_.active, __ATOMIC_ACQUIRE))
    {
      __atomic_add_fetch(&probe_.violations, 1U, __ATOMIC_RELAXED);
    }
    if (node < 1 || node > 3)
    {
      return -1;
    }
    out = node == 1 ? root() : userfs::Attributes{node, Size, 0, 0, 0777, T_FILE, 1};
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
  control(unsigned, const char *name, unsigned version, unsigned command, unsigned,
          const void *, unsigned, void *, unsigned) override {
    if (strcmp(name, "pool.barrier") || version != 1 || command != 1)
    {
      return -1;
    }
    if (getpid() != owner_ || __atomic_load_n(&probe_.active, __ATOMIC_ACQUIRE))
    {
      __atomic_add_fetch(&probe_.violations, 1U, __ATOMIC_RELAXED);
    }
    return 0;
  }

  int
  read(unsigned node, unsigned long offset, void *data, unsigned count) override {
    if (node < 2 || node > 3 || offset > Size || count > Size - offset)
    {
      return -1;
    }
    enter(node, false);
    memmove(data, probe_.values[node - 2] + offset, count);
    leave(node, false);
    return count;
  }

  int
  write(unsigned node, unsigned long offset, const void *data,
        unsigned count) override {
    if (node < 2 || node > 3 || offset > Size || count > Size - offset)
    {
      return -1;
    }
    enter(node, true);
    memmove(probe_.values[node - 2] + offset, data, count);
    leave(node, true);
    return count;
  }
};

void
joined(int pid, int expected = 0) {
  int status = -1;
  check(waitpid(pid, &status, 0) == pid && status == (expected << 8),
        "client completed");
}
} // namespace

extern "C" int
main() {
  constexpr unsigned Mapping = (sizeof(Probe) + 4095) & ~4095U;
  auto *probe = static_cast<Probe *>(mmap(nullptr, Mapping, PROT_READ | PROT_WRITE,
                                          MAP_SHARED | MAP_ANONYMOUS, -1, 0));
  check(probe != MAP_FAILED, "shared backend state");
  memset(probe, 0, sizeof(*probe));
  memset(probe->values[0], 'A', Size);
  memset(probe->values[1], 'B', Size);
  int readiness[2];
  check(pipe(readiness) == 0, "readiness pipe");
  int provider = fork();
  check(provider >= 0, "provider fork");
  if (!provider)
  {
    close(readiness[0]);
    Backend backend(*probe);
    int descriptor = userfs::register_backend("pool-test", backend);
    check(descriptor >= 0 && write(readiness[1], "R", 1) == 1, "register pool backend");
    close(readiness[1]);
    int result = userfs::serve(descriptor, backend);
    close(descriptor);
    exit(result < 0);
  }
  close(readiness[1]);
  char ready = 0;
  check(read(readiness[0], &ready, 1) == 1 && ready == 'R', "provider ready");
  close(readiness[0]);
  check(mkdir("/tmp/pool") == 0 && mount("pool-test", "/tmp/pool") == 0,
        "mount pool backend");
  int files[2]{open("/tmp/pool/a", O_RDWR), open("/tmp/pool/b", O_RDWR)};
  int directory = open("/tmp/pool", O_RDONLY);
  check(files[0] >= 0 && files[1] >= 0 && directory >= 0, "open pool files");
  int children[2];
  for (unsigned writing = 0; writing < 2; ++writing)
  {
    __atomic_store_n(&probe->gate, writing + 1, __ATOMIC_RELEASE);
    __atomic_store_n(&probe->hold, writing, __ATOMIC_RELEASE);
    for (unsigned i = 0; i < 2; ++i)
    {
      children[i] = fork();
      check(children[i] >= 0, "parallel client fork");
      if (!children[i])
      {
        static char bytes[4096];
        if (writing)
        {
          memset(bytes, 'X' + i, sizeof(bytes));
          check(lseek(files[i], 16384, SEEK_SET) == 16384 &&
                    write(files[i], bytes, sizeof(bytes)) == sizeof(bytes),
                "parallel write");
        }
        else
        {
          check(read(files[i], bytes, sizeof(bytes)) == sizeof(bytes), "parallel read");
          for (char byte : bytes)
          {
            check(byte == 'A' + i, "isolated read response");
          }
        }
        exit(0);
      }
    }
    if (writing)
    {
      auto deadline = clockticks() + 20000000;
      while (__atomic_load_n(&probe->entered[1], __ATOMIC_ACQUIRE) < 2)
      {
        check(clockticks() < deadline, "both different-file write handlers are active");
        poll(nullptr, 0, 1);
      }
      int metadata = fork();
      check(metadata >= 0, "metadata client fork");
      if (!metadata)
      {
        xv::fscontrol::Request query;
        strcpy(query.interface_name, "pool.barrier");
        query.command = 1;
        check(ioctl(directory, xv::fscontrol::Ioctl,
                    reinterpret_cast<unsigned long>(&query)) == 0,
              "metadata/control request after data barrier");
        __atomic_store_n(&probe->metadata_done, 1U, __ATOMIC_RELEASE);
        exit(0);
      }
      poll(nullptr, 0, 50);
      check(!__atomic_load_n(&probe->metadata_done, __ATOMIC_ACQUIRE),
            "metadata waits for active writes");
      __atomic_store_n(&probe->hold, 0U, __ATOMIC_RELEASE);
      joined(metadata);
    }
    for (int child : children)
    {
      joined(child);
    }
  }
  __atomic_store_n(&probe->gate, 0U, __ATOMIC_RELEASE);
  check(probe->peak == 2 && probe->write_peak == 2 &&
            probe->workers[0] != probe->workers[1],
        "persistent handlers overlap for reads and different-file writes");
  for (unsigned i = 0; i < 2; ++i)
  {
    children[i] = fork();
    check(children[i] >= 0, "repeated client fork");
    if (!children[i])
    {
      static char bytes[4096];
      memset(bytes, 'X' + i, sizeof(bytes));
      for (unsigned round = 0; round < 8; ++round)
      {
        check(lseek(files[i], 32768 + round * 4096, SEEK_SET) ==
                      long(32768 + round * 4096) &&
                  write(files[i], bytes, sizeof(bytes)) == sizeof(bytes),
              "reuse persistent write handler");
      }
      exit(0);
    }
  }
  for (int child : children)
  {
    joined(child);
  }
  for (unsigned i = 0; i < 2; ++i)
  {
    static char bytes[4096];
    check(lseek(files[i], 16384, SEEK_SET) == 16384 &&
              read(files[i], bytes, sizeof(bytes)) == sizeof(bytes),
          "read shared state written by persistent worker");
    for (char byte : bytes)
    {
      check(byte == 'X' + i, "worker writes survive across requests");
    }
  }
  for (unsigned i = 0; i < 2; ++i)
  {
    children[i] = fork();
    check(children[i] >= 0, "same-inode writer fork");
    if (!children[i])
    {
      int file = open("/tmp/pool/a", O_RDWR);
      check(file >= 0, "independent description for shared inode");
      static char bytes[4096];
      memset(bytes, 'M' + i, sizeof(bytes));
      for (unsigned round = 0; round < 4; ++round)
      {
        check(lseek(file, (i + 1) * 4096, SEEK_SET) == long((i + 1) * 4096) &&
                  write(file, bytes, sizeof(bytes)) == sizeof(bytes),
              "same-inode write");
      }
      close(file);
      exit(0);
    }
  }
  for (int child : children)
  {
    joined(child);
  }
  check(probe->values[0][4096] == 'M' && probe->values[0][8192] == 'N',
        "same-inode writes retain independent ranges");
  check(!probe->violations && probe->calls[0] >= 3 && probe->calls[1] >= 3 &&
            probe->metadata_done,
        "stable worker PID set, inode reservations and metadata exclusion");
  __atomic_store_n(&probe->entered[1], 0U, __ATOMIC_RELEASE);
  __atomic_store_n(&probe->hold, 1U, __ATOMIC_RELEASE);
  __atomic_store_n(&probe->gate, 2U, __ATOMIC_RELEASE);
  for (unsigned i = 0; i < 2; ++i)
  {
    children[i] = fork();
    check(children[i] >= 0, "failure client fork");
    if (!children[i])
    {
      static char bytes[4096];
      check(lseek(files[i], 0, SEEK_SET) == 0 &&
                write(files[i], bytes, sizeof(bytes)) < 0,
            "worker failure cancels in-flight operations");
      exit(0);
    }
  }
  auto deadline = clockticks() + 20000000;
  while (__atomic_load_n(&probe->entered[1], __ATOMIC_ACQUIRE) < 2)
  {
    check(clockticks() < deadline, "failure handlers entered");
    poll(nullptr, 0, 1);
  }
  __atomic_store_n(&probe->abort_tid, probe->workers[0], __ATOMIC_RELEASE);
  for (int child : children)
  {
    joined(child);
  }
  joined(provider, 1);
  check(probe->failed && sigsend(probe->workers[1], 0) < 0,
        "unexpected worker exit terminates the provider and cancels the whole pool");
  close(files[0]);
  close(files[1]);
  close(directory);
  check(umount("/tmp/pool") == 0 && unlink("/tmp/pool") == 0,
        "unmount persistent backend");
  check(munmap(probe, Mapping) == 0, "release shared probe");
  printf(
      "fspooltest: PASS: persistent TIDs, parallel reads/writes, shared state, metadata barriers and failure cancellation\n");
  return 0;
}
