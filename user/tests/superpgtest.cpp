// Extra regression checks; the original pgtbltest remains unchanged.
#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"

extern "C" {

static void
check(int ok, char *why)
{
  if(!ok){
    printf("superpgtest: FAIL: %s\n", why);
    exit(1);
  }
}

static uint64
allocate(uint64 *old)
{
  *old = (uint64)sbrk(0);
  uint64 base = SUPERPGROUNDUP(*old);
  check(sbrk(base + SUPERPGSIZE - *old) != SBRK_ERROR, "allocate superpage");
  check(pgpte((void *)base) == pgpte((void *)(base + SUPERPGSIZE - PGSIZE)),
        "one PTE covers 2 MiB");
  return base;
}

static void
copy_and_split(void)
{
  uint64 old;
  uint64 base = allocate(&old);
  char data[64], got[64];
  for(int i = 0; i < sizeof(data); i++)
    data[i] = i + 1;
  char *inside = (char *)(base + 213 * PGSIZE - 7);
  int fd = open("superio", O_CREATE | O_RDWR);
  check(fd >= 0 && write(fd, data, sizeof(data)) == sizeof(data), "prepare I/O");
  close(fd);
  fd = open("superio", O_RDONLY);
  check(read(fd, inside, sizeof(data)) == sizeof(data), "copyout into superpage");
  close(fd);
  check(memcmp(inside, data, sizeof(data)) == 0, "copyout physical offset");
  strcpy((char *)(base + 67 * PGSIZE + 13), "superio");
  fd = open((char *)(base + 67 * PGSIZE + 13), O_RDWR);
  check(fd >= 0, "copyinstr within superpage");
  check(write(fd, inside, sizeof(data)) == sizeof(data), "copyin from superpage");
  close(fd);
  fd = open("superio", O_RDONLY);
  check(read(fd, got, sizeof(got)) == sizeof(got) &&
        memcmp(got, data, sizeof(data)) == 0, "copyin physical offset");
  close(fd);
  unlink("superio");

  *(volatile int *)(base + 16) = 123;
  int pid = fork(), status;
  check(pid >= 0, "fork with superpage");
  if(pid == 0){
    check(*(volatile int *)(base + 16) == 123, "fork content");
    *(volatile int *)(base + 16) = 456;
    exit(0);
  }
  check(wait(&status) == pid && status == 0, "child completed");
  check(*(volatile int *)(base + 16) == 123, "fork memory is independent");

  uint64 top = (uint64)sbrk(0);
  uint64 split = base + 257 * PGSIZE + 113;
  check(sbrk(-(int)(top - split)) != SBRK_ERROR, "partial shrink");
  check(memcmp(inside, data, sizeof(data)) == 0, "split preserves data");
  check(pgpte((void *)base) != pgpte((void *)(base + PGSIZE)), "split into 4 KiB PTEs");
  check(pgpte((void *)PGROUNDUP(split)) == 0, "tail is unmapped");
  check(sbrk(-(int)((uint64)sbrk(0) - old)) != SBRK_ERROR, "release split pages");
  // Reuse the same VA after a demotion left an empty level-0 table.
  base = allocate(&old);
  check(sbrk(-(int)((uint64)sbrk(0) - old)) != SBRK_ERROR, "release reused superpage");
  printf("superpgtest: copy, fork, split and reuse: OK\n");
}

static void
access_bits(void)
{
  uint64 old;
  uint64 base = allocate(&old);
  volatile char *buf = (volatile char *)base;
  uint bits;
  check(pgaccess((void *)base, 32, &bits) == 0, "initial superpage access sample");
  check(pgpte((void *)base) != pgpte((void *)(base + PGSIZE)),
        "pgaccess demotes for per-page access bits");
  buf[PGSIZE] = 7;
  buf[31 * PGSIZE] = 8;
  check(pgaccess((void *)base, 32, &bits) == 0 &&
        bits == ((1U << 1) | (1U << 31)), "access bits after demotion");
  check(pgaccess((void *)base, 32, &bits) == 0 && bits == 0,
        "access bits cleared after demotion");
  check(buf[PGSIZE] == 7, "read from demoted page");
  check(pgaccess((void *)base, 32, &bits) == 0 && bits == (1U << 1),
        "repeated access after demotion");
  // Also fork a page table demoted by pgaccess, then free it in the child.
  int pid = fork(), status;
  check(pid >= 0, "fork pgaccess-demoted pages");
  if(pid == 0){
    check(buf[31 * PGSIZE] == 8, "fork demoted page content");
    exit(0);
  }
  check(wait(&status) == pid && status == 0, "demoted child completed");
  check(sbrk(-(int)((uint64)sbrk(0) - old)) != SBRK_ERROR, "release sampled pages");
  printf("superpgtest: pgaccess with user superpages: OK\n");
}

static void
split_without_free_memory(void)
{
  uint64 old;
  uint64 base = allocate(&old);
  *(volatile char *)(base + SUPERPGSIZE - 2 * PGSIZE) = 'Z';
  int ready[2], finish[2], status;
  check(pipe(ready) == 0 && pipe(finish) == 0, "create synchronization pipes");
  int pid = fork();
  check(pid >= 0, "fork for memory pressure");
  if(pid == 0){
    while(sbrk(PGSIZE) != SBRK_ERROR)
      ;
    char c = 'R';
    check(write(ready[1], &c, 1) == 1, "signal memory exhausted");
    check(read(finish[0], &c, 1) == 1, "wait for shrink");
    exit(0);
  }
  char c;
  check(read(ready[0], &c, 1) == 1, "wait for memory pressure");
  // A failed sbrk at a page-table boundary can leave one free data page.
  // A pipe consumes that page, forcing demotion's no-free-page path.
  int extra[2];
  int extra_ok = pipe(extra);
  check(sbrk(-PGSIZE) != SBRK_ERROR, "shrink under memory pressure");
  check(*(volatile char *)(base + SUPERPGSIZE - 2 * PGSIZE) == 'Z',
        "memory-pressure split preserves neighboring data");
  check(pgpte(sbrk(0)) == 0, "memory-pressure split unmaps tail");
  if(extra_ok == 0){
    close(extra[0]);
    close(extra[1]);
  }
  check(write(finish[1], &c, 1) == 1, "release memory hog");
  check(wait(&status) == pid && status == 0, "memory hog exited");
  close(ready[0]); close(ready[1]);
  close(finish[0]); close(finish[1]);
  check(sbrk(-(int)((uint64)sbrk(0) - old)) != SBRK_ERROR, "release remaining pages");
  printf("superpgtest: split with exhausted memory: OK\n");
}

int
main(void)
{
  copy_and_split();
  access_bits();
  split_without_free_memory();
  printf("superpgtest: all tests succeeded\n");
  exit(0);
}
}
