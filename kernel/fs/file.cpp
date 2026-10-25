//
// Support functions for system calls that involve file descriptors.
//

#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"
#include "kernel/support/param.h"
#include "kernel/fs/fs.h"
#include "kernel/sync/spinlock.h"
#include "kernel/sync/sleeplock.h"
#include "kernel/fs/file.h"
#include "kernel/fs/stat.h"
#include "kernel/process/proc.h"
#include "kernel/sync/scoped_lock.hpp"

extern "C" {

struct devsw devsw[NDEV];
struct {
  class SpinLock lock;
  struct file file[NFILE];
} ftable;

void
fileinit(void)
{
  ftable.lock.init("ftable");
}

// Allocate a file structure.
struct file *
filealloc(void)
{
  struct file *f;

  xv::sync::ScopedLock guard(ftable.lock);
  for (f = ftable.file; f < ftable.file + NFILE; f++) {
    if (f->ref == 0) {
      f->ref = 1;

      return f;
    }
  }

  return 0;
}

// Increment ref count for file f.
struct file *
filedup(struct file *f)
{
  xv::sync::ScopedLock guard(ftable.lock);
  if (f->ref < 1)
    panic("filedup");
  f->ref++;

  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  ftable.lock.acquire();
  if (f->ref < 1)
    panic("fileclose");
  if (--f->ref > 0) {
    ftable.lock.release();
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = file::Kind::none;
  ftable.lock.release();

  if (ff.type == file::Kind::pipe) {
    pipeclose(ff.pipe, ff.writable);
  } else if (ff.type == file::Kind::inode || ff.type == file::Kind::device) {
    begin_op();
    iput(ff.ip);
    end_op();
  }
}

// Get metadata about file f.
// addr is a user virtual address, pointing to a struct stat.
int
filestat(struct file *f, uint64 addr)
{
  struct proc *p = myproc();
  struct stat st;

  if (f->type == file::Kind::inode || f->type == file::Kind::device) {
    ilock(f->ip);
    stati(f->ip, &st);
    iunlock(f->ip);
    if (copyout(p->pagetable, p->sz, addr, (char *)&st, sizeof(st)) < 0)
      return -1;
    return 0;
  }
  return -1;
}

// Read from file f.
// addr is a user virtual address.
int
fileread(struct file *f, uint64 addr, int n)
{
  int r = 0;

  if (f->readable == 0 || n < 0)
    return -1;

  if (f->type == file::Kind::pipe) {
    r = piperead(f->pipe, addr, n);
  } else if (f->type == file::Kind::device) {
    if (f->major < 0 || f->major >= NDEV || !devsw[f->major].read)
      return -1;
    r = devsw[f->major].read(1, addr, n);
  } else if (f->type == file::Kind::inode) {
    ilock(f->ip);
    if ((r = readi(f->ip, 1, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
  } else {
    panic("fileread");
  }

  return r;
}

// Write to file f.
// addr is a user virtual address.
int
filewrite(struct file *f, uint64 addr, int n)
{
  int r, ret = 0;

  if (f->writable == 0 || n < 0)
    return -1;

  if (f->type == file::Kind::pipe) {
    ret = pipewrite(f->pipe, addr, n);
  } else if (f->type == file::Kind::device) {
    if (f->major < 0 || f->major >= NDEV || !devsw[f->major].write)
      return -1;
    ret = devsw[f->major].write(1, addr, n);
  } else if (f->type == file::Kind::inode) {
    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    int max = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * BSIZE;
    int i = 0;
    while (i < n) {
      int n1 = n - i;
      if (n1 > max)
        n1 = max;

      begin_op();
      ilock(f->ip);
      if ((r = writei(f->ip, 1, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      end_op();

      if (r != n1) {
        // error from writei
        break;
      }
      i += r;
    }
    ret = (i == n ? n : -1);
  } else {
    panic("filewrite");
  }

  return ret;
}
}
