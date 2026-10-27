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
#include "kernel/fs/mount.h"

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
    if (ip->kind() == T_DIR && (omode & ~O_NONBLOCK) != O_RDONLY)
    {
      xv::fs::filesystem().unlock_release(ip);
      return -1;
    }
  }

  unsigned access_mode = (omode & O_ACCMODE) == O_WRONLY ? 2 : (omode & O_ACCMODE) == O_RDWR ? 6 : 4;
  if ((omode & O_ACCMODE) == 3 || !xv::fs::filesystem().allowed(ip, access_mode | ((omode & O_TRUNC) && ip->kind() == T_FILE ? 2 : 0)))
  { xv::fs::filesystem().unlock_release(ip); return -1; }

  if (ip->kind() == T_FILE && ip->owner().read_only() &&
      ((omode & O_ACCMODE) != O_RDONLY || (omode & O_TRUNC)))
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

  if (!xv::fs::files().bind_inode(*f, *ip, !(omode & O_WRONLY),
                                (omode & O_WRONLY) || (omode & O_RDWR), omode & O_NONBLOCK))
  {
    xv::process::current()->files().take(fd);
    xv::fs::files().release(f);
    xv::fs::filesystem().unlock_release(ip);
    return -1;
  }

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
  if (ip->kind() != T_DIR || !xv::fs::filesystem().allowed(ip, 1))
  {
    xv::fs::filesystem().unlock_release(ip);
    return -1;
  }
  xv::fs::filesystem().unlock(ip);
  p->directory().replace(ip);
  return 0;
}

namespace
{
struct ArgumentVector
{
  char *items[MAXARG]{};
  ~ArgumentVector()
  { for (auto *item : items) if (item) xv::memory::allocator().deallocate(item); }
  bool copy(uint64 address)
  {
    if (!address) return false;
    for (unsigned i = 0; i < MAXARG; ++i)
    {
      uint64 argument;
      if (address > ~0UL - i * sizeof(uint64) || fetch_address(address + i * sizeof(uint64), &argument) < 0) return false;
      if (!argument) return true;
      if (i == MAXARG - 1) return false;
      items[i] = static_cast<char *>(xv::memory::allocator().allocate());
      if (!items[i] || fetch_string(argument, items[i], PGSIZE) < 0) return false;
    }
    return false;
  }
};
uint64 execute_image(bool with_environment)
{
  char path[MAXPATH]; uint64 arguments, environment = 0;
  argument_address(1, &arguments);
  if (with_environment) argument_address(2, &environment);
  if (argument_string(0, path, MAXPATH) < 0) return -1;
  ArgumentVector argv, envp;
  if (!argv.copy(arguments) || (environment && !envp.copy(environment))) return -1;
  return xv::process::execute(path, argv.items, envp.items);
}
}
uint64 sys_exec() { return execute_image(false); }
uint64 sys_execve() { return execute_image(true); }

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

uint64 sys_openpty(void)
{
  uint64 address;
  argument_address(0, &address);
  auto *master_file = xv::fs::files().allocate();
  auto *slave_file = xv::fs::files().allocate();
  xv::drivers::CharacterDevice *master = nullptr, *slave = nullptr;
  if (!master_file || !slave_file || !xv::drivers::create_pty(&master, &slave))
  {
    if (master_file) xv::fs::files().release(master_file);
    if (slave_file) xv::fs::files().release(slave_file);
    return -1;
  }
  xv::fs::files().bind_endpoint(*master_file, *master);
  xv::fs::files().bind_endpoint(*slave_file, *slave);
  int fds[2] = {-1, -1};
  auto &table = xv::process::current()->files();
  fds[0] = table.install(master_file);
  if (fds[0] >= 0) fds[1] = table.install(slave_file);
  if (fds[1] < 0 || xv::process::current()->memory().copy_out(address, fds, sizeof(fds)) < 0)
  {
    if (fds[0] >= 0) table.take(fds[0]);
    if (fds[1] >= 0) table.take(fds[1]);
    xv::fs::files().release(master_file);
    xv::fs::files().release(slave_file);
    return -1;
  }
  return 0;
}

uint64 sys_fcntl(void)
{
  xv::fs::File *file;
  int command, flags = 0;
  argument_integer(1, &command);
  if (command == F_SETFL) argument_integer(2, &flags);
  if (argfd(0, nullptr, &file) < 0) return -1;
  return xv::fs::files().fcntl(file, command, static_cast<unsigned>(flags));
}

uint64 sys_ioctl(void)
{
  xv::fs::File *file;
  int request;
  uint64 argument;
  argument_integer(1, &request);
  argument_address(2, &argument);
  if (argfd(0, nullptr, &file) < 0)
  {
    return -1;
  }
  return xv::fs::files().ioctl(file, static_cast<unsigned>(request), argument);
}

uint64 sys_sync(void)
{
  xv::fs::journal().sync();
  return 0;
}
namespace
{
uint64 attributes(bool ownership)
{
  char path[MAXPATH]; int first, second = -1;
  argument_integer(1, &first); if (ownership) argument_integer(2, &second);
  if (argument_string(0, path, MAXPATH) < 0) return -1;
  xv::fs::Transaction transaction;
  auto *node = xv::fs::filesystem().lookup(path);
  if (!node) return -1;
  xv::fs::filesystem().lock(node);
  auto who = xv::process::current()->credentials().snapshot();
  bool valid = !node->owner().read_only() && (who.effective_uid == 0 || who.effective_uid == node->uid());
  unsigned uid = node->uid(), gid = node->gid(), mode = node->mode();
  if (ownership)
  {
    valid &= first >= -1 && first <= 65534 && second >= -1 && second <= 65534;
    if (first >= 0) { valid &= who.effective_uid == 0 || static_cast<unsigned>(first) == uid; uid = first; }
    if (second >= 0)
    {
      bool member = who.effective_gid == static_cast<unsigned>(second);
      for (unsigned i = 0; i < who.group_count; ++i) member |= who.groups[i] == static_cast<unsigned>(second);
      valid &= who.effective_uid == 0 || member; gid = second;
    }
  }
  else { valid &= first >= 0 && !(first & ~01777); mode = first; }
  if (valid) node->owner().attributes(node, uid, gid, mode);
  xv::fs::filesystem().unlock_release(node);
  return valid ? 0 : -1;
}
}
uint64 sys_chmod() { return attributes(false); }
uint64 sys_chown() { return attributes(true); }
uint64 sys_access()
{
  char path[MAXPATH]; int mode; argument_integer(1, &mode);
  if ((mode & ~7) || argument_string(0, path, MAXPATH) < 0) return -1;
  xv::fs::Transaction transaction;
  auto *node = xv::fs::filesystem().lookup(path, true);
  if (!node) return -1;
  xv::fs::filesystem().lock(node);
  bool allowed = xv::fs::filesystem().allowed(node, mode, true);
  xv::fs::filesystem().unlock_release(node); return allowed ? 0 : -1;
}
uint64 sys_stat()
{
  char path[MAXPATH]; uint64 address; argument_address(1, &address);
  if (argument_string(0, path, MAXPATH) < 0) return -1;
  xv::fs::Transaction transaction;
  auto *node = xv::fs::filesystem().lookup(path);
  if (!node) return -1;
  xv::fs::Stat value{};
  xv::fs::filesystem().lock(node); xv::fs::filesystem().stat(node, &value);
  xv::fs::filesystem().unlock_release(node);
  return xv::process::current()->memory().copy_out(address, &value, sizeof(value));
}

uint64 sys_rename()
{
  char old[MAXPATH], next[MAXPATH];
  if (argument_string(0, old, MAXPATH) <= 0 || argument_string(1, next, MAXPATH) <= 0) return -1;
  const char *paths[] = {old, next};
  for (const char *path : paths)
  {
    unsigned component = 0;
    for (const char *p = path; *p; ++p) { if (*p == '/') component = 0; else if (++component > DIRSIZ) return -1; }
    if (!component) return -1;
  }
  return xv::fs::filesystem().rename(old, next);
}
uint64 sys_mount()
{
  if (xv::process::current()->credentials().snapshot().effective_uid != 0) return -1;
  char path[MAXPATH], type[16];
  if (argument_string(0, type, sizeof(type)) < 0 || argument_string(1, path, sizeof(path)) < 0 ||
      xv::runtime::compare_text(type, "ramfs", 16) != 0) return -1;
  return xv::fs::filesystem().mount_ram(path);
}
uint64 sys_umount()
{
  if (xv::process::current()->credentials().snapshot().effective_uid != 0) return -1;
  char path[MAXPATH]; if (argument_string(0, path, sizeof(path)) < 0) return -1;
  return xv::fs::filesystem().unmount(path);
}
uint64 sys_mountinfo()
{
  int index; uint64 address; argument_integer(0, &index); argument_address(1, &address);
  if (index < 0) return -1;
  mount_info record{};
  int result = xv::fs::filesystem().mounts(index, record.path, record.type);
  if (result && xv::process::current()->memory().copy_out(address, &record, sizeof(record)) < 0) return -1;
  return result;
}
} // namespace xv::syscall
