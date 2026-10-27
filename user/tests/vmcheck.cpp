#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/memory/layout/memlayout.h"
#include "user/runtime/user.h"
#include "user/runtime/proc.h"
static void check(bool value, const char *why)
{
  if (!value) { fprintf(2, "vmcheck: FAIL: %s\n", why); exit(1); }
}
static char token;
static void put(int fd) { check(write(fd, "x", 1) == 1, "pipe notify"); }
static void get(int fd) { check(read(fd, &token, 1) == 1, "pipe wait"); }
static void wait_ok(int pid)
{
  int status; check(wait(&status) == pid && status == 0, "child success");
}
extern "C" int main()
{
  check(pgpte(reinterpret_cast<void *>(MAXVA)) == 0, "invalid introspection address rejected");
  constexpr int bytes = 24 * 1024 * 1024;
  auto *old = sbrk(0);
  check(sbrk(bytes) != SBRK_ERROR, "allocate parent heap");
  auto base = PGROUNDUP(reinterpret_cast<uint64>(old));
  auto *data = reinterpret_cast<volatile char *>(base);
  for (int i = 0; i < bytes / PGSIZE - 1; ++i) data[i * PGSIZE] = i % 97;
  char memory[512];
  check(proc::read("/proc/meminfo", memory, sizeof(memory)) >= 0, "read free memory");
  int free_before = proc::number(memory, "MemFree");
  int to_child[2], to_parent[2];
  check(pipe(to_child) == 0 && pipe(to_parent) == 0, "create sync pipes");
  auto physical = PTE2PA(pgpte(reinterpret_cast<void *>(base)));
  int pid = fork(); check(pid >= 0, "COW fork");
  if (!pid)
  {
    auto pte = pgpte(reinterpret_cast<void *>(base));
    check(PTE2PA(pte) == physical && (pte & PTE_COW) && !(pte & PTE_W), "share parent heap read-only");
    data[0] = 100;
    check(pgaccess(reinterpret_cast<void *>(base), 1, reinterpret_cast<void *>(base + 3 * PGSIZE)) == 0 &&
          data[3 * PGSIZE] == 1, "pgaccess writes a COW output page");
    check(PTE2PA(pgpte(reinterpret_cast<void *>(base))) != physical, "first store creates private page");
    check(read(to_child[0], reinterpret_cast<void *>(base + PGSIZE), 1) == 1, "read copyout breaks COW");
    check(data[PGSIZE] == 'x', "copyout result");
    put(to_parent[1]); get(to_child[0]);
    check(data[2 * PGSIZE] == 2, "parent writes isolated");
    int grandchild = fork(); check(grandchild >= 0, "nested fork");
    if (!grandchild) { data[0] = 101; exit(0); }
    wait_ok(grandchild); check(data[0] == 100, "grandchild writes isolated");
    exit(0);
  }
  check(proc::read("/proc/meminfo", memory, sizeof(memory)) >= 0, "sample child memory");
  check(free_before - proc::number(memory, "MemFree") < 4096, "fork shares 24 MiB instead of copying it");
  put(to_child[1]); get(to_parent[0]);
  check(data[0] == 0 && data[PGSIZE] == 1 && data[3 * PGSIZE] == 3, "child store and kernel copyout preserve parent");
  data[2 * PGSIZE] = 102; put(to_child[1]); wait_ok(pid);
  close(to_child[0]); close(to_child[1]); close(to_parent[0]); close(to_parent[1]);
  check(sbrk(-bytes) != SBRK_ERROR, "release shared heap");
  auto *lazy = sbrklazy(16 * PGSIZE); check(lazy != SBRK_ERROR, "reserve lazy heap");
  pid = fork(); check(pid >= 0, "fork lazy holes");
  if (!pid) { lazy[8 * PGSIZE] = 77; exit(0); }
  wait_ok(pid); check(lazy[8 * PGSIZE] == 0, "lazy holes remain independent");
  check(sbrk(-16 * PGSIZE) != SBRK_ERROR, "release lazy heap");
  auto *execute = sbrklazy(PGSIZE); check(execute != SBRK_ERROR, "reserve NX page");
  *reinterpret_cast<unsigned *>(execute) = 0x00008067; // RISC-V ret, if execution were permitted.
  for (int mode = 0; mode < 4; ++mode)
  {
    pid = fork(); check(pid >= 0, "fork fault victim");
    if (!pid)
    {
      if (!mode) *reinterpret_cast<volatile char *>(reinterpret_cast<uint64>(&main)) = 0;
      if (mode == 1) *reinterpret_cast<volatile char *>(USYSCALL) = 0;
      if (mode == 2) *reinterpret_cast<volatile char *>(PGROUNDUP(reinterpret_cast<uint64>(sbrk(0))) + PGSIZE) = 0;
      if (mode == 3) reinterpret_cast<void (*)()>(execute)();
      exit(0);
    }
    int status; check(wait(&status) == pid && status == -1, "protection and unmapped faults terminate only victim");
  }
  check(sbrk(-PGSIZE) != SBRK_ERROR, "release NX page");
  for (int i = 0; i < 48; ++i)
  {
    pid = fork(); check(pid >= 0, "repeated fork");
    if (!pid) { token = 42; exit(0); }
    wait_ok(pid);
  }
  int ready[2]; check(pipe(ready) == 0, "statistics sync pipe");
  pid = fork(); check(pid >= 0, "statistics stress child");
  if (!pid)
  {
    put(ready[1]);
    for (int i = 0; i < 64; ++i)
    {
      int grandchild = fork(); check(grandchild >= 0, "concurrent stack reuse");
      if (!grandchild) exit(0);
      wait_ok(grandchild);
    }
    exit(0);
  }
  get(ready[0]);
  for (int i = 0; i < 96; ++i)
  {
    uint64 large, regular;
    check(ksupernpte(&large, &regular) == 0 && large && regular, "statistics during stack table reclamation");
  }
  wait_ok(pid); close(ready[0]); close(ready[1]);
  printf("vmcheck: regions, COW, copyout, lazy pages and protection passed\n");
  exit(0);
}
