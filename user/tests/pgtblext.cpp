// Additional boundary tests; the supplied pgtbltest.c remains unchanged.
#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/memory/layout/memlayout.h"
#include "user/runtime/user.h"

extern "C"
{

  static void check(int ok, char *why)
  {
    if (!ok)
    {
      printf("pgtblext: FAIL: %s\n", why);
      exit(1);
    }
  }

  int main(void)
  {
    char *old = sbrk(0);
    uint64 start = PGROUNDUP((uint64)old);
    check(sbrk(36 * PGSIZE) != SBRK_ERROR, "allocate pages");
    volatile char *buf = (volatile char *)start;
    uint bits;

    check(pgaccess((void *)buf, 32, &bits) == 0, "initial sample");
    buf[0] = 1;
    buf[31 * PGSIZE] = 1;
    check(pgaccess((void *)buf, 32, &bits) == 0 && bits == (1U | (1U << 31)),
          "first and last bits");
    check(pgaccess((void *)buf, 32, &bits) == 0 && bits == 0,
          "sampling clears access bits");
    check(buf[31 * PGSIZE] == 1, "read after TLB invalidation");
    check(pgaccess((void *)buf, 32, &bits) == 0 && bits == (1U << 31),
          "repeated accesses are detected");

    // Two output bytes straddle a page boundary; surrounding bytes survive.
    uchar *out = (uchar *)(start + 33 * PGSIZE - 1);
    out[-1] = out[2] = 0xa5;
    check(pgaccess((void *)buf, 9, out) == 0, "cross-page initial sample");
    buf[0] = 2;
    buf[8 * PGSIZE] = 2;
    check(pgaccess((void *)buf, 9, out) == 0 && out[0] == 1 && out[1] == 1,
          "cross-page result and partial final byte");
    check(out[-1] == 0xa5 && out[2] == 0xa5, "output buffer bounds");

    check(pgaccess((void *)buf, 0, &bits) == -1, "zero count");
    check(pgaccess((void *)buf, -1, &bits) == -1, "negative count");
    check(pgaccess((void *)buf, 4097, &bits) == -1, "count limit");
    check(pgaccess((void *)MAXVA, 1, &bits) == -1, "invalid base");
    check(pgaccess((void *)(MAXVA - PGSIZE), 2, &bits) == -1, "range beyond MAXVA");
    check(pgaccess(sbrk(0), 1, &bits) == -1, "unmapped source");
    check(pgaccess((void *)TRAPFRAME, 1, &bits) == -1, "supervisor source");
    buf[PGSIZE] = 3;
    check(pgaccess((void *)buf, 32, (void *)USYSCALL) == -1, "read-only output");
    check(pgaccess((void *)buf, 32, (void *)(MAXVA - 1)) == -1,
          "overflowing output range");
    check(pgaccess((void *)buf, 32, sbrk(0)) == -1, "unmapped output");
    check(pgaccess((void *)buf, 32, &bits) == 0 && (bits & (1U << 1)),
          "failed calls preserve access bits");

    pte_t pte = pgpte((void *)USYSCALL);
    check((pte & (PTE_V | PTE_R | PTE_U)) == (PTE_V | PTE_R | PTE_U) &&
              (pte & (PTE_W | PTE_X)) == 0,
          "PID page permissions");
    int pid = fork();
    check(pid >= 0, "fork");
    if (pid == 0)
    {
      check(ugetpid() == getpid(), "child PID");
      *(volatile int *)USYSCALL = 0; // must fault
      exit(0);
    }
    int status;
    check(wait(&status) == pid && status == -1, "PID page rejects user writes");
    check(ugetpid() == getpid(), "parent PID after child exit");
    check(sbrk(-36 * PGSIZE) != SBRK_ERROR, "release test pages");

    uint64 super, regular;
    check(ksupernpte(&super, &regular) == 0, "kernel page counts");
    check(super > 0 && regular < 1610, "kernel uses bounded mixed-size mappings");
    uint64 first = ~0UL;
    check(ksupernpte(&first, (uint64 *)MAXVA) == -1 && first == super,
          "second statistics copy failure preserves first write");
    regular = ~0UL;
    check(ksupernpte((uint64 *)MAXVA, &regular) == -1 && regular == ~0UL,
          "first statistics copy failure skips second write");
    printf("pgtblext: kernel superpages=%ld regular leaves=%ld\n", super, regular);
    printf("pgtblext: all tests succeeded\n");
    exit(0);
  }
}
