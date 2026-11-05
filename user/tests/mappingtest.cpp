#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/mman.h"
#include "kernel/io/posix_errno.h"
#include "user/runtime/user.h"

namespace
{
constexpr unsigned Page = 4096;

void
check(bool value, const char *message) {
  if (!value)
  {
    fprintf(2, "mappingtest: FAIL: %s errno=%d\n", message, errno);
    exit(1);
  }
}

void
child_done(int child, int expected = 0) {
  int status = 7;
  check(child >= 0 && wait(&status) == child && status == expected, "child completion");
}

char *
anonymous(unsigned pages, int flags = MAP_PRIVATE,
          int protection = PROT_READ | PROT_WRITE) {
  auto *memory = static_cast<char *>(
      mmap(nullptr, pages * Page, protection, flags | MAP_ANONYMOUS, -1, 0));
  check(memory != MAP_FAILED, "anonymous mapping");
  return memory;
}

void
permissions() {
  auto *memory = anonymous(3);
  memory[0] = 11;
  memory[Page] = 22;
  memory[2 * Page] = 33;
  check(mprotect(memory + Page, Page, PROT_NONE) == 0, "middle page inaccessible");
  int child = fork();
  if (!child)
  {
    volatile char value = memory[Page];
    (void)value;
    exit(0);
  }
  child_done(child, -1);
  child = fork();
  if (!child)
  {
    check(mprotect(memory + Page, Page, PROT_READ | PROT_WRITE) == 0 &&
              memory[Page] == 22,
          "fork retains inaccessible private backing");
    memory[Page] = 44;
    exit(0);
  }
  child_done(child);
  check(mprotect(memory + Page, Page, PROT_READ) == 0 && memory[Page] == 22,
        "restoring access preserves private contents");
  child = fork();
  if (!child)
  {
    memory[Page] = 0;
    exit(0);
  }
  child_done(child, -1);
  check(mprotect(memory, 3 * Page, PROT_READ | PROT_WRITE) == 0 && memory[0] == 11 &&
            memory[2 * Page] == 33,
        "protect across split regions");
  check(mprotect(memory + 1, Page, PROT_READ) < 0 && errno == EINVAL,
        "unaligned protection rejected");
  check(munmap(memory + Page, Page) == 0, "partial unmap after protection split");
  check(mprotect(memory, 3 * Page, PROT_NONE) < 0 && errno == ENOMEM,
        "protection crossing hole fails atomically");
  memory[0] = 12;
  memory[2 * Page] = 34;
  check(munmap(memory, 3 * Page) == 0, "unmap split fragments and hole");
  auto *initially_none = anonymous(1, MAP_PRIVATE, PROT_NONE);
  check(mprotect(initially_none, Page, PROT_READ | PROT_WRITE) == 0 &&
            initially_none[0] == 0,
        "initial PROT_NONE retains zero backing");
  initially_none[0] = 19;
  check(mprotect(initially_none, Page, PROT_NONE) == 0 &&
            munmap(initially_none, Page) == 0,
        "release inaccessible backing");
}

void
inheritance() {
  auto *shared = anonymous(2, MAP_SHARED);
  auto *private_memory = anonymous(2);
  for (unsigned i = 0; i < 2 * Page; ++i)
  {
    check(!shared[i] && !private_memory[i], "anonymous memory zero filled");
  }
  shared[0] = 1;
  private_memory[0] = 2;
  check(mprotect(shared + Page, Page, PROT_NONE) == 0,
        "shared backing inaccessible before fork");
  int child = fork();
  if (!child)
  {
    shared[0] = 3;
    private_memory[0] = 4;
    check(mprotect(shared + Page, Page, PROT_READ | PROT_WRITE) == 0,
          "child restores shared backing");
    shared[Page] = 5;
    exit(0);
  }
  child_done(child);
  check(shared[0] == 3 && private_memory[0] == 2 &&
            mprotect(shared + Page, Page, PROT_READ) == 0 && shared[Page] == 5,
        "fork sharing, isolation and independent protections");
  check(munmap(shared, 2 * Page) == 0 && munmap(private_memory, 2 * Page) == 0,
        "inherited mappings cleanup");
}

void
shared_access() {
  constexpr const char *name = "/mappingtest";
  shm_unlink(name);
  int writable = shm_open(name, O_CREATE | O_RDWR, 0600);
  check(writable >= 0 && ftruncate(writable, Page) == 0, "shared object fixture");
  int readonly = shm_open(name, O_RDONLY, 0);
  check(readonly >= 0, "readonly shared object");
  auto *memory =
      static_cast<char *>(mmap(nullptr, Page, PROT_READ, MAP_SHARED, readonly, 0));
  check(memory != MAP_FAILED && mprotect(memory, Page, PROT_READ | PROT_WRITE) < 0 &&
            errno == EACCES,
        "mprotect cannot escalate readonly shared descriptor");
  check(munmap(memory, Page) == 0, "readonly mapping cleanup");
  memory =
      static_cast<char *>(mmap(nullptr, Page, PROT_NONE, MAP_PRIVATE, readonly, 0));
  check(memory != MAP_FAILED && mprotect(memory, Page, PROT_READ | PROT_WRITE) == 0,
        "private mapping can gain write permission");
  memory[0] = 42;
  check(munmap(memory, Page) == 0 && close(readonly) == 0 && close(writable) == 0 &&
            shm_unlink(name) == 0,
        "shared object cleanup");
}

void
executable() {
  auto *memory = anonymous(1);
  *reinterpret_cast<unsigned *>(memory) = 0x02a00513;     // addi a0, zero, 42
  *reinterpret_cast<unsigned *>(memory + 4) = 0x00008067; // ret
  check(mprotect(memory, Page, PROT_READ | PROT_EXEC) == 0, "executable protection");
  check(reinterpret_cast<int (*)()>(memory)() == 42, "execute generated instructions");
  check(signal(SIGUSR1, reinterpret_cast<sighandler_t>(memory)) != SIG_ERR &&
            sigsend(getpid(), SIGUSR1) == 0 && signal(SIGUSR1, SIG_DFL) != SIG_ERR,
        "signal handler may execute from mmap memory");
  check(munmap(memory, Page) == 0, "executable cleanup");
}
} // namespace

extern "C" int
main() {
  permissions();
  inheritance();
  shared_access();
  executable();
  for (unsigned i = 0; i < 48; ++i)
  {
    auto *memory = anonymous(3);
    check(mprotect(memory + Page, Page, PROT_NONE) == 0 &&
              munmap(memory, 3 * Page) == 0,
          "repeated protection lifecycle");
  }
  printf(
      "mappingtest: PASS: anonymous, protections, backing retention, fork, holes, access ceilings and executable pages\n");
  return 0;
}
