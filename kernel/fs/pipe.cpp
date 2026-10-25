import xv.sync;

#include "kernel/support/placement_new.hpp"
#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"
#include "kernel/support/param.h"
#include "kernel/process/proc.h"
#include "kernel/fs/fs.h"
#include "kernel/fs/file.h"

import xv.memory;

extern "C" {

#define PIPESIZE 512

struct pipe {
  xv::sync::SpinLock lock;
  char data[PIPESIZE];
  uint nread;    // number of bytes read
  uint nwrite;   // number of bytes written
  int readopen;  // read fd is still open
  int writeopen; // write fd is still open
};

int
pipealloc(struct file **f0, struct file **f1)
{
  struct pipe *pi;

  pi = 0;
  *f0 = *f1 = 0;
  if ((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
    goto bad;
  if ((pi = (struct pipe *)xv::memory::allocator().allocate()) == 0)
    goto bad;
  new (pi) pipe;
  pi->readopen = 1;
  pi->writeopen = 1;
  pi->nwrite = 0;
  pi->nread = 0;
  pi->lock.init("pipe");
  (*f0)->type = file::Kind::pipe;
  (*f0)->readable = 1;
  (*f0)->writable = 0;
  (*f0)->pipe = pi;
  (*f1)->type = file::Kind::pipe;
  (*f1)->readable = 0;
  (*f1)->writable = 1;
  (*f1)->pipe = pi;
  return 0;

bad:
  if (pi)
  {
    pi->~pipe();
    xv::memory::allocator().deallocate((char *)pi);
  }
  if (*f0)
    fileclose(*f0);
  if (*f1)
    fileclose(*f1);
  return -1;
}

void
pipeclose(struct pipe *pi, int writable)
{
  pi->lock.acquire();
  if (writable) {
    pi->writeopen = 0;
    wakeup(&pi->nread);
  } else {
    pi->readopen = 0;
    wakeup(&pi->nwrite);
  }
  if (pi->readopen == 0 && pi->writeopen == 0) {
    pi->lock.release();
    pi->~pipe();
    xv::memory::allocator().deallocate((char *)pi);
  } else
    pi->lock.release();
}

int
pipewrite(struct pipe *pi, uint64 addr, int n)
{
  int i = 0;
  struct proc *pr = myproc();

  pi->lock.acquire();
  while (i < n) {
    if (pi->readopen == 0 || killed(pr)) {
      pi->lock.release();
      return -1;
    }
    if (pi->nwrite == pi->nread + PIPESIZE) { //DOC: pipewrite-full
      wakeup(&pi->nread);
      sleep_prepare(&pi->nwrite);
      pi->lock.release();
      sleep();
      pi->lock.acquire();
    } else {
      char ch;
      if (copyin(pr->pagetable, pr->sz, &ch, addr + i, 1) == -1) {
        if (i == 0)
          i = -1;
        break;
      }
      pi->data[pi->nwrite++ % PIPESIZE] = ch;
      i++;
    }
  }
  wakeup(&pi->nread);
  pi->lock.release();

  return i;
}

int
piperead(struct pipe *pi, uint64 addr, int n)
{
  int i;
  struct proc *pr = myproc();
  char ch;

  pi->lock.acquire();
  while (pi->nread == pi->nwrite && pi->writeopen) { //DOC: pipe-empty
    if (killed(pr)) {
      pi->lock.release();
      return -1;
    }
    sleep_prepare(&pi->nread); //DOC: piperead-sleep
    pi->lock.release();
    sleep();
    pi->lock.acquire();
  }
  for (i = 0; i < n; i++) { //DOC: piperead-copy
    if (pi->nread == pi->nwrite)
      break;
    ch = pi->data[pi->nread % PIPESIZE];
    if (copyout(pr->pagetable, pr->sz, addr + i, &ch, 1) == -1) {
      if (i == 0)
        i = -1;
      break;
    }
    pi->nread++;
  }
  wakeup(&pi->nwrite); //DOC: piperead-wakeup
  pi->lock.release();
  return i;
}
}
