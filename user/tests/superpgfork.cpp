// User-superpage tests ported from MIT's 2025 pgtbltest.
// A failed first fork child is propagated as failure instead of success.
#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "user/runtime/user.h"

extern "C" {
#define SZ (8 * SUPERPGSIZE)
static char *testname;
void superpg_fork(void);
void superpg_free(void);
void err(char *why)
{
  printf("superpgfork: %s failed: %s, pid=%d\n", testname, why, getpid());
  exit(1);
}
int main(void)
{
  superpg_fork();
  superpg_free();
  printf("superpgfork: all tests succeeded\n");
  exit(0);
}

void
supercheck(char *end)
{
  pte_t last_pte = 0;
  uint64 a = (uint64) end;
  uint64 s = SUPERPGROUNDUP(a);

  for (; a < s; a += PGSIZE) {
    pte_t pte = (pte_t) pgpte((void *) a);
    if (pte == 0) {
      err("no pte");
    }
  }

  for (uint64 p = s;  p < s + 512 * PGSIZE; p += PGSIZE) {
    pte_t pte = (pte_t) pgpte((void *) p);
    if(pte == 0)
      err("no pte");
    if ((uint64) last_pte != 0 && pte != last_pte) {
        err("pte different");
    }
    if((pte & PTE_V) == 0 || (pte & PTE_R) == 0 || (pte & PTE_W) == 0){
      err("pte wrong");
    }
    last_pte = pte;
  }

  for(int i = 0; i < 512 * PGSIZE; i += PGSIZE){
    *(int*)(s+i) = i;
  }

  for(int i = 0; i < 512 * PGSIZE; i += PGSIZE){
    if(*(int*)(s+i) != i)
      err("wrong value");
  }
}

void
superpg_fork()
{
  int pid;
  
  printf("superpg_fork starting\n");
  testname = "superpg_fork";
  
  char *end = sbrk(SZ);
  if (end == 0 || end == SBRK_ERROR)
    err("sbrk failed");

  // check if parent has super pages
  supercheck(end);
  if((pid = fork()) < 0) {
    err("fork");
  } else if(pid == 0) {
    // check if child's address space has super pages
    supercheck(end);
    exit(0);
  } else {
    int status;
    wait(&status);
    if (status != 0) {
      exit(1);
    }
  }

  // free super pages
  sbrk(-SZ);
  if((pid = fork()) < 0) {
    err("fork");
  } else if(pid == 0) {
    // reference freed memory; this should result in page fault and
    // the kernel should kill the child.
    * (end + 1) = '9'; 
  } else {
    int status;
    wait(&status);
    if (status == 0) {
      err("child was able to reference free memory\n");
      exit(1);
    }
  }  
  printf("superpg_fork: OK\n");  
}

void
superpg_free()
{
  int pid;
  
  printf("superpg_free starting\n");
  testname = "superpg_free";

  char *end = sbrk(SZ);
  if (end == 0 || end == SBRK_ERROR)
    err("sbrk failed");

  // free pages beyond a super page
  char *a = sbrk(0);
  uint64 s = SUPERPGROUNDDOWN((uint64) a);
  sbrk(-((uint64) a-s));
  a = sbrk(0);

  pte_t pte1 = (pte_t) pgpte(a-PGSIZE);
  pte_t pte2 = (pte_t) pgpte(a-2*PGSIZE);
  if (pte1 != pte2) {
    err("not a super page");
  }
  
  // write to the last 8192-byte section of a super page
  * (a - PGSIZE + 1) = '8';
  * (a - 2*PGSIZE + 1) = '9';

  // free last 4096 bytes of a super page
  sbrk(-PGSIZE);
  a = sbrk(0);

  if (*(a - PGSIZE + 1) != '9') {
    err("lost content after freeing part of super page");
  }

  if((pid = fork()) < 0) {
    err("fork");
  } else if(pid == 0) {
     // the memory at address a shouldn't be in the child's address
     // space, since the parent freed it. The following reference
     // should result in page fault and the kernel should kill the
     // child.
    if (* (a + 1) == '9') {
      exit(0);
    }
  } else {
    int status;
    wait(&status);
    if (status == 0) {
      err("child was able to reference free memory\n");
      exit(1);
    }
  }

  pte1 = (pte_t) pgpte((void *) a);
  if(pte1 != 0) {
    err("pte for freed memory is valid");
  }

  s = SUPERPGROUNDDOWN((uint64) a);
  for (; (uint64) a > s; a -= PGSIZE) {
    a = sbrk(-PGSIZE);
    pte1 = (pte_t) pgpte(sbrk(0));
    if(pte1 != 0) {
      err("page hasn't been freed");
    }
  }
  
  printf("superpg_free: OK\n");  
}
}
