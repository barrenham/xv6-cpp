module;

//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/support/param.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/fcntl.h"

module xv.syscall;

import xv.runtime;

import :arguments;
import :handlers;
import xv.fs;
import xv.process;
import xv.sync;
import xv.memory;

namespace xv::syscall
{

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding xv::fs::File.
static int argfd(int n, int *pfd, xv::fs::File **pf)
{
  int fd;
  xv::fs::File *f;

  argument_integer(n, &fd);
  if (fd < 0 || fd >= NOFILE || (f = xv::process::current()->files().get(fd)) == 0)
  {
    return -1;
  }
  if (pfd)
  {
    *pfd = fd;
  }
  if (pf)
  {
    *pf = f;
  }
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int fdalloc(xv::fs::File *f)
{
  xv::process::Process *p = xv::process::current();

  return p->files().install(f);
}

uint64 sys_dup(void)
{
  int descriptor;
  argument_integer(0, &descriptor);
  return xv::process::current()->files().duplicate(descriptor);
}

uint64 sys_read(void)
{
  xv::fs::File *f;
  int n;
  uint64 p;

  argument_address(1, &p);
  argument_integer(2, &n);
  if (argfd(0, 0, &f) < 0)
  {
    return -1;
  }
  return xv::fs::files().read(f, p, n);
}

uint64 sys_write(void)
{
  xv::fs::File *f;
  int n;
  uint64 p;

  argument_address(1, &p);
  argument_integer(2, &n);
  if (argfd(0, 0, &f) < 0)
  {
    return -1;
  }

  return xv::fs::files().write(f, p, n);
}

uint64 sys_close(void)
{
  int descriptor;
  argument_integer(0, &descriptor);
  return xv::process::current()->files().close(descriptor) ? 0 : -1;
}

uint64 sys_fstat(void)
{
  xv::fs::File *f;
  uint64 st; // user pointer to struct stat

  argument_address(1, &st);
  if (argfd(0, 0, &f) < 0)
  {
    return -1;
  }
  return xv::fs::files().stat(f, st);
}

uint64 sys_link(void)
{
  char old[MAXPATH], next[MAXPATH];
  if (argument_string(0, old, MAXPATH) < 0 || argument_string(1, next, MAXPATH) < 0)
  {
    return -1;
  }
  return xv::fs::filesystem().link(old, next);
}

uint64 sys_unlink(void)
{
  char path[MAXPATH];
  if (argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  return xv::fs::filesystem().unlink(path);
}

uint64 sys_open(void)
{
  char path[MAXPATH];
  int fd, omode;
  xv::fs::File *f;
  xv::fs::Inode *ip;
  int n;

  argument_integer(1, &omode);
  if ((n = argument_string(0, path, MAXPATH)) < 0)
  {
    return -1;
  }

  xv::fs::Transaction transaction;

  if (omode & O_CREATE)
  {
    ip = xv::fs::filesystem().create(path, T_FILE, 0, 0);
    if (ip == 0)
    {
      return -1;
    }
  }
  else
  {
    if ((ip = xv::fs::filesystem().lookup(path)) == 0)
    {
      return -1;
    }
    xv::fs::filesystem().lock(ip);
    if (ip->kind() == T_DIR && omode != O_RDONLY)
    {
      xv::fs::filesystem().unlock_release(ip);
      return -1;
    }
  }

  if (ip->kind() == T_DEVICE && (ip->device_major() < 0 || ip->device_major() >= NDEV))
  {
    xv::fs::filesystem().unlock_release(ip);
    return -1;
  }

  if ((f = xv::fs::files().allocate()) == 0 || (fd = fdalloc(f)) < 0)
  {
    if (f)
    {
      xv::fs::files().release(f);
    }
    xv::fs::filesystem().unlock_release(ip);
    return -1;
  }

  xv::fs::files().bind_inode(*f, *ip, !(omode & O_WRONLY),
                             (omode & O_WRONLY) || (omode & O_RDWR));

  if ((omode & O_TRUNC) && ip->kind() == T_FILE)
  {
    xv::fs::filesystem().truncate(ip);
  }

  xv::fs::filesystem().unlock(ip);

  return fd;
}

uint64 sys_mkdir(void)
{
  char path[MAXPATH];
  xv::fs::Inode *ip;

  xv::fs::Transaction transaction;
  if (argument_string(0, path, MAXPATH) < 0 ||
      (ip = xv::fs::filesystem().create(path, T_DIR, 0, 0)) == 0)
  {
    return -1;
  }
  xv::fs::filesystem().unlock_release(ip);
  return 0;
}

uint64 sys_mknod(void)
{
  xv::fs::Inode *ip;
  char path[MAXPATH];
  int major, minor;

  xv::fs::Transaction transaction;
  argument_integer(1, &major);
  argument_integer(2, &minor);
  if ((argument_string(0, path, MAXPATH)) < 0 ||
      (ip = xv::fs::filesystem().create(path, T_DEVICE, major, minor)) == 0)
  {
    return -1;
  }
  xv::fs::filesystem().unlock_release(ip);
  return 0;
}

uint64 sys_chdir(void)
{
  char path[MAXPATH];
  xv::fs::Inode *ip;
  xv::process::Process *p = xv::process::current();

  xv::fs::Transaction transaction;
  if (argument_string(0, path, MAXPATH) < 0 ||
      (ip = xv::fs::filesystem().lookup(path)) == 0)
  {
    return -1;
  }
  xv::fs::filesystem().lock(ip);
  if (ip->kind() != T_DIR)
  {
    xv::fs::filesystem().unlock_release(ip);
    return -1;
  }
  xv::fs::filesystem().unlock(ip);
  p->directory().replace(ip);
  return 0;
}

uint64 sys_exec(void)
{
  char path[MAXPATH], *argv[MAXARG];
  int i;
  uint64 uargv, uarg;
  int ret;

  argument_address(1, &uargv);
  if (argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  xv::runtime::fill(argv, 0, sizeof(argv));
  for (i = 0;; i++)
  {
    if (i >= xv::runtime::array_size(argv))
    {
      goto bad;
    }
    if (fetch_address(uargv + sizeof(uint64) * i, (uint64 *)&uarg) < 0)
    {
      goto bad;
    }
    if (uarg == 0)
    {
      argv[i] = 0;
      break;
    }
    argv[i] = static_cast<char *>(xv::memory::allocator().allocate());
    if (argv[i] == 0)
    {
      goto bad;
    }
    if (fetch_string(uarg, argv[i], PGSIZE) < 0)
    {
      goto bad;
    }
  }

  ret = xv::process::execute(path, argv);

  for (i = 0; i < xv::runtime::array_size(argv) && argv[i] != 0; i++)
  {
    xv::memory::allocator().deallocate(argv[i]);
  }

  return ret;

bad:
  for (i = 0; i < xv::runtime::array_size(argv) && argv[i] != 0; i++)
  {
    xv::memory::allocator().deallocate(argv[i]);
  }
  return -1;
}

uint64 sys_pipe(void)
{
  uint64 fdarray; // user pointer to array of two integers
  xv::fs::File *rf, *wf;
  int fd0, fd1;
  xv::process::Process *p = xv::process::current();

  argument_address(0, &fdarray);
  if (!xv::fs::create_pipe(&rf, &wf))
  {
    return -1;
  }
  fd0 = -1;
  if ((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0)
  {
    if (fd0 >= 0)
    {
      p->files().take(fd0);
    }
    xv::fs::files().release(rf);
    xv::fs::files().release(wf);
    return -1;
  }
  if (p->memory().copy_out(fdarray, &fd0, sizeof(fd0)) < 0 ||
      p->memory().copy_out(fdarray + sizeof(fd0), &fd1, sizeof(fd1)) < 0)
  {
    p->files().take(fd0);
    p->files().take(fd1);
    xv::fs::files().release(rf);
    xv::fs::files().release(wf);
    return -1;
  }
  return 0;
}

uint64 sys_sync(void)
{
  xv::fs::journal().sync();
  return 0;
}
} // namespace xv::syscall
