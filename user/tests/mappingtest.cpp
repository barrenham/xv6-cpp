#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/mman.h"
#include "kernel/io/posix_errno.h"
#include "kernel/process/wait/wait.h"
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

void
bus(int) {
  exit(42);
}

void
wait_bus(int child) {
  int status = -1;
  check(child > 0 && waitpid(child, &status, 0) == child && WIFEXITED(status) &&
            WEXITSTATUS(status) == 42,
        "shared memory bounds deliver catchable SIGBUS");
}

void
shared_lifecycle() {
  constexpr const char *name = "/mapping-lifecycle";
  shm_unlink(name);
  int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
  check(fd >= 0, "create empty shared object");
  auto *memory = static_cast<volatile char *>(
      mmap(nullptr, 3 * Page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  check(memory != MAP_FAILED, "reserve shared mapping beyond current EOF");
  int child = fork();
  if (!child)
  {
    signal(SIGBUS, bus);
    (void)memory[0];
    exit(1);
  }
  wait_bus(child);
  check(ftruncate(fd, 3 * Page) == 0 && memory[0] == 0 && memory[2 * Page] == 0,
        "growth makes previously empty mapping accessible");
  memory[0] = 'A';
  memory[Page] = 'B';
  memory[2 * Page] = 'C';
  int ready[2];
  check(pipe(ready) == 0, "SMP truncate handshake");
  child = fork();
  if (!child)
  {
    close(ready[0]);
    signal(SIGBUS, bus);
    check(memory[Page] == 'B', "resident shared page before truncate");
    check(write(ready[1], "R", 1) == 1, "publish resident child PTE");
    for (;;)
    {
      (void)memory[Page]; // no syscall: force remote-hart TLB revocation
    }
  }
  close(ready[1]);
  char token;
  check(child > 0 && read(ready[0], &token, 1) == 1, "child page ready");
  close(ready[0]);
  check(ftruncate(fd, 17) == 0, "truncate shared object with active remote mapping");
  wait_bus(child);
  check(memory[0] == 'A' && memory[17] == 0 && memory[Page - 1] == 0,
        "partial EOF page zero filled");
  memory[17] = 'P';
  memory[Page - 1] = 'Q';
  check(ftruncate(fd, 3 * Page) == 0 && memory[Page] == 0 && memory[2 * Page] == 0,
        "regrowth does not resurrect truncated bytes");
  check(memory[17] == 0 && memory[Page - 1] == 0,
        "growth clears mapped EOF padding stores");
  auto *offset = static_cast<char *>(
      mmap(nullptr, Page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 2 * Page));
  check(offset != MAP_FAILED, "nonzero shared mapping offset");
  memory[2 * Page] = 'Z';
  check(offset[0] == 'Z', "shared offset aliases same object");
  check(shm_unlink(name) == 0 && close(fd) == 0, "unlink and close with live mappings");
  fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
  check(fd >= 0 && ftruncate(fd, Page) == 0, "same name creates an independent object");
  check(memory[0] == 'A' && offset[0] == 'Z', "mapping owns unlinked object lifetime");
  auto *reset = static_cast<volatile char *>(
      mmap(nullptr, Page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  check(reset != MAP_FAILED, "map replacement object");
  reset[0] = 'N';
  child = fork();
  if (!child)
  {
    int truncated = shm_open(name, O_RDWR | O_TRUNC, 0);
    check(truncated >= 0 && close(truncated) == 0, "independent O_TRUNC");
    exit(0);
  }
  child_done(child);
  child = fork();
  if (!child)
  {
    signal(SIGBUS, bus);
    (void)reset[0];
    exit(1);
  }
  wait_bus(child);
  check(pipe(ready) == 0 && write(ready[1], const_cast<char *>(reset), 1) < 0,
        "kernel copy rejects truncated shared buffer without killing caller");
  close(ready[0]);
  close(ready[1]);
  check(ftruncate(fd, Page) == 0 && reset[0] == 0 &&
            munmap(const_cast<char *>(reset), Page) == 0,
        "O_TRUNC regrowth and mapping cleanup");
  check(munmap(const_cast<char *>(memory) + Page, Page) == 0 &&
            munmap(const_cast<char *>(memory), 3 * Page) == 0 &&
            munmap(offset, Page) == 0 && close(fd) == 0 && shm_unlink(name) == 0,
        "shared mapping split, descriptor and object cleanup");
}

void
mapped_arguments() {
  auto *memory = anonymous(2);
  auto **arguments = reinterpret_cast<char **>(memory);
  arguments[0] = const_cast<char *>("echo");
  arguments[1] = const_cast<char *>("mapped-argument-vector");
  arguments[2] = nullptr;
  auto **environment = reinterpret_cast<char **>(memory + 64);
  environment[0] = nullptr;
  int child = fork();
  if (!child)
  {
    execve("/bin/echo", arguments, environment);
    check(false, "exec accepts argv and envp from mmap regions above the heap");
  }
  child_done(child);
  child = fork();
  if (!child)
  {
    check(mprotect(memory, Page, PROT_NONE) == 0 &&
              execve("/bin/echo", arguments, environment) < 0,
          "exec rejects protected mapped argument vectors");
    exit(0);
  }
  child_done(child);
  child = fork();
  if (!child)
  {
    auto *boundary = memory + Page - 4;
    char *value = const_cast<char *>("echo");
    memmove(boundary, &value, sizeof(value));
    check(mprotect(memory + Page, Page, PROT_NONE) == 0 &&
              execve("/bin/echo", reinterpret_cast<char **>(boundary), environment) < 0,
          "exec rejects a pointer word crossing into an inaccessible page");
    exit(0);
  }
  child_done(child);
  check(munmap(memory, 2 * Page) == 0, "release mapped argument vectors");
}
} // namespace

extern "C" int
main() {
  permissions();
  inheritance();
  shared_access();
  shared_lifecycle();
  executable();
  mapped_arguments();
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
