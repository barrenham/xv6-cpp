module;
#include "kernel/support/flow.h"

//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "kernel/arch/riscv/register_constants.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/mount.h"
#include "kernel/fs/stat.h"
#include "kernel/support/param.h"
#include "kernel/support/types.h"

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

namespace
{
namespace flow = xv::flow;
struct OpenFlow
{
  struct Context
  {
    char *path;
    int mode;
    xv::fs::Inode *inode = nullptr;
    xv::fs::File *file = nullptr;
    int descriptor = -1;
    uint64 result = static_cast<uint64>(-1);
  };

  static bool creating(Context &c) noexcept { return c.mode & O_CREATE; }
  static bool create(Context &c) noexcept
  {
    c.inode = xv::fs::filesystem().create(c.path, T_FILE, 0, 0, !(c.mode & O_NOFOLLOW));
    return c.inode;
  }
  static bool lookup(Context &c) noexcept
  {
    c.inode = xv::fs::filesystem().lookup(c.path, false, !(c.mode & O_NOFOLLOW));
    return c.inode;
  }
  static void lock(Context &c) noexcept { xv::fs::filesystem().lock(c.inode); }
  static bool existing_kind(Context &c) noexcept
  {
    return c.inode->kind() != T_SYMLINK &&
           (c.inode->kind() != T_DIR || (c.mode & ~O_NONBLOCK) == O_RDONLY);
  }
  static bool permitted(Context &c) noexcept
  {
    unsigned access = (c.mode & O_ACCMODE) == O_WRONLY ? 2
                      : (c.mode & O_ACCMODE) == O_RDWR ? 6
                                                       : 4;
    return (c.mode & O_ACCMODE) != 3 &&
           xv::fs::filesystem().allowed(
               c.inode,
               access | ((c.mode & O_TRUNC) && c.inode->kind() == T_FILE ? 2 : 0));
  }
  static bool writable_mount(Context &c) noexcept
  {
    return !(c.inode->kind() == T_FILE && c.inode->owner().read_only() &&
             ((c.mode & O_ACCMODE) != O_RDONLY || (c.mode & O_TRUNC)));
  }
  static bool allocate(Context &c) noexcept
  {
    c.file = xv::fs::files().allocate();
    return c.file;
  }
  static bool install(Context &c) noexcept
  {
    c.descriptor = fdalloc(c.file);
    return c.descriptor >= 0;
  }
  static bool bind(Context &c) noexcept
  {
    return xv::fs::files().bind_inode(*c.file, *c.inode, !(c.mode & O_WRONLY),
                                      (c.mode & O_WRONLY) || (c.mode & O_RDWR),
                                      c.mode & O_NONBLOCK, c.mode & O_APPEND);
  }
  static bool truncating(Context &c) noexcept
  {
    return (c.mode & O_TRUNC) && c.inode->kind() == T_FILE;
  }
  static void truncate(Context &c) noexcept
  {
    if (c.inode->mode() & 06000)
    {
      c.inode->owner().attributes(c.inode, c.inode->uid(), c.inode->gid(),
                                  c.inode->mode() & ~06000U);
    }
    xv::fs::filesystem().truncate(c.inode);
  }
  static void detach(Context &c) noexcept
  {
    xv::process::current()->files().take(c.descriptor);
  }
  static void release_file(Context &c) noexcept { xv::fs::files().release(c.file); }
  static void release_inode(Context &c) noexcept
  {
    xv::fs::filesystem().unlock_release(c.inode);
  }
  static void success(Context &c) noexcept
  {
    xv::fs::filesystem().unlock(c.inode);
    c.result = c.descriptor;
  }
  struct Choose;
  struct Create;
  struct Lookup;
  struct Lock;
  struct Permission;
  struct Mount;
  struct Allocate;
  struct Install;
  struct Bind;
  struct Truncating;
  struct Truncate;
  struct Success;
  struct Detach;
  struct ReleaseFile;
  struct ReleaseInode;
  using Graph = flow::graph<
      Choose,
      flow::types<
          flow::node<Choose, flow::noop,
                     flow::branch<flow::condition<&creating>, flow::go<Create>,
                                  flow::go<Lookup>>>,
          flow::node<
              Create, flow::noop,
              flow::branch<flow::condition<&create>, flow::go<Permission>, flow::stop>>,
          flow::node<
              Lookup, flow::noop,
              flow::branch<flow::condition<&lookup>, flow::go<Lock>, flow::stop>>,
          flow::node<Lock, flow::action<&lock>,
                     flow::branch<flow::condition<&existing_kind>, flow::go<Permission>,
                                  flow::go<ReleaseInode>>>,
          flow::node<Permission, flow::noop,
                     flow::branch<flow::condition<&permitted>, flow::go<Mount>,
                                  flow::go<ReleaseInode>>>,
          flow::node<Mount, flow::noop,
                     flow::branch<flow::condition<&writable_mount>, flow::go<Allocate>,
                                  flow::go<ReleaseInode>>>,
          flow::node<Allocate, flow::noop,
                     flow::branch<flow::condition<&allocate>, flow::go<Install>,
                                  flow::go<ReleaseInode>>>,
          flow::node<Install, flow::noop,
                     flow::branch<flow::condition<&install>, flow::go<Bind>,
                                  flow::go<ReleaseFile>>>,
          flow::node<Bind, flow::noop,
                     flow::branch<flow::condition<&bind>, flow::go<Truncating>,
                                  flow::go<Detach>>>,
          flow::node<Truncating, flow::noop,
                     flow::branch<flow::condition<&truncating>, flow::go<Truncate>,
                                  flow::go<Success>>>,
          flow::node<Truncate, flow::action<&truncate>, flow::go<Success>>,
          flow::node<Success, flow::action<&success>, flow::stop>,
          flow::node<Detach, flow::action<&detach>, flow::go<ReleaseFile>>,
          flow::node<ReleaseFile, flow::action<&release_file>, flow::go<ReleaseInode>>,
          flow::node<ReleaseInode, flow::action<&release_inode>, flow::stop>>>;
};
struct CreateNodeFlow
{
  struct Context
  {
    char *path;
    short kind;
    int major, minor;
    xv::fs::Inode *inode = nullptr;
    uint64 result = static_cast<uint64>(-1);
  };

  static bool create(Context &c) noexcept
  {
    c.inode = xv::fs::filesystem().create(c.path, c.kind, c.major, c.minor);
    return c.inode;
  }
  static void release(Context &c) noexcept
  {
    xv::fs::filesystem().unlock_release(c.inode);
    c.result = 0;
  }
  struct Create;
  struct Release;
  using Graph =
      flow::graph<Create,
                  flow::types<flow::node<Create, flow::noop,
                                         flow::branch<flow::condition<&create>,
                                                      flow::go<Release>, flow::stop>>,
                              flow::node<Release, flow::action<&release>, flow::stop>>>;
};
struct DirectoryFlow
{
  struct Context
  {
    xv::process::Process *process;
    const char *path;
    xv::fs::Inode *inode = nullptr;
    uint64 result = static_cast<uint64>(-1);
  };

  static bool lookup(Context &c) noexcept
  {
    c.inode = xv::fs::filesystem().lookup(c.path);
    return c.inode;
  }
  static void lock(Context &c) noexcept { xv::fs::filesystem().lock(c.inode); }
  static bool searchable(Context &c) noexcept
  {
    return c.inode->kind() == T_DIR && xv::fs::filesystem().allowed(c.inode, 1);
  }
  static void release(Context &c) noexcept
  {
    xv::fs::filesystem().unlock_release(c.inode);
  }
  static void commit(Context &c) noexcept
  {
    xv::fs::filesystem().unlock(c.inode);
    c.process->directory().replace(c.inode);
    c.result = 0;
  }
  struct Lookup;
  struct Lock;
  struct Commit;
  struct Release;
  using Graph = flow::graph<
      Lookup, flow::types<flow::node<Lookup, flow::noop,
                                     flow::branch<flow::condition<&lookup>,
                                                  flow::go<Lock>, flow::stop>>,
                          flow::node<Lock, flow::action<&lock>,
                                     flow::branch<flow::condition<&searchable>,
                                                  flow::go<Commit>, flow::go<Release>>>,
                          flow::node<Commit, flow::action<&commit>, flow::stop>,
                          flow::node<Release, flow::action<&release>, flow::stop>>>;
};
struct PipeFlow
{
  struct Context
  {
    xv::process::Process *process;
    uint64 address;
    xv::fs::File *reader = nullptr, *writer = nullptr;
    int first = -1, second = -1;
    uint64 result = static_cast<uint64>(-1);
  };

  static bool create(Context &c) noexcept
  {
    return xv::fs::create_pipe(&c.reader, &c.writer);
  }
  static bool install_reader(Context &c) noexcept
  {
    c.first = fdalloc(c.reader);
    return c.first >= 0;
  }
  static bool install_writer(Context &c) noexcept
  {
    c.second = fdalloc(c.writer);
    return c.second >= 0;
  }
  static bool copy_reader(Context &c) noexcept
  {
    return c.process->memory().copy_out(c.address, &c.first, sizeof(c.first)) >= 0;
  }
  static bool copy_writer(Context &c) noexcept
  {
    return c.process->memory().copy_out(c.address + sizeof(c.first), &c.second,
                                        sizeof(c.second)) >= 0;
  }
  static void detach_reader(Context &c) noexcept { c.process->files().take(c.first); }
  static void detach_both(Context &c) noexcept
  {
    c.process->files().take(c.first);
    c.process->files().take(c.second);
  }
  static void release(Context &c) noexcept
  {
    xv::fs::files().release(c.reader);
    xv::fs::files().release(c.writer);
  }
  static void success(Context &c) noexcept { c.result = 0; }
  struct Create;
  struct InstallReader;
  struct InstallWriter;
  struct CopyReader;
  struct CopyWriter;
  struct Success;
  struct DetachReader;
  struct DetachBoth;
  struct Release;
  using Graph = flow::graph<
      Create,
      flow::types<
          flow::node<Create, flow::noop,
                     flow::branch<flow::condition<&create>, flow::go<InstallReader>,
                                  flow::stop>>,
          flow::node<InstallReader, flow::noop,
                     flow::branch<flow::condition<&install_reader>,
                                  flow::go<InstallWriter>, flow::go<Release>>>,
          flow::node<InstallWriter, flow::noop,
                     flow::branch<flow::condition<&install_writer>,
                                  flow::go<CopyReader>, flow::go<DetachReader>>>,
          flow::node<CopyReader, flow::noop,
                     flow::branch<flow::condition<&copy_reader>, flow::go<CopyWriter>,
                                  flow::go<DetachBoth>>>,
          flow::node<CopyWriter, flow::noop,
                     flow::branch<flow::condition<&copy_writer>, flow::go<Success>,
                                  flow::go<DetachBoth>>>,
          flow::node<Success, flow::action<&success>, flow::stop>,
          flow::node<DetachReader, flow::action<&detach_reader>, flow::go<Release>>,
          flow::node<DetachBoth, flow::action<&detach_both>, flow::go<Release>>,
          flow::node<Release, flow::action<&release>, flow::stop>>>;
};
struct PtyOpenFlow
{
  struct Context
  {
    uint64 address;
    xv::fs::File *master_file, *slave_file;
    xv::drivers::CharacterDevice *master = nullptr, *slave = nullptr;
    int descriptors[2] = {-1, -1};
    xv::process::DescriptorTable *table = nullptr;
    uint64 result = static_cast<uint64>(-1);
  };

  static void allocate(Context &c) noexcept
  {
    c.master_file = xv::fs::files().allocate();
    c.slave_file = xv::fs::files().allocate();
  }
  static bool create(Context &c) noexcept
  {
    return c.master_file && c.slave_file &&
           xv::drivers::create_pty(&c.master, &c.slave);
  }
  static void bind(Context &c) noexcept
  {
    xv::fs::files().bind_endpoint(*c.master_file, *c.master);
    xv::fs::files().bind_endpoint(*c.slave_file, *c.slave);
  }
  static bool install(Context &c) noexcept
  {
    c.table = &xv::process::current()->files();
    c.descriptors[0] = c.table->install(c.master_file);
    if (c.descriptors[0] >= 0)
    {
      c.descriptors[1] = c.table->install(c.slave_file);
    }
    return c.descriptors[1] >= 0;
  }
  static bool copy(Context &c) noexcept
  {
    return xv::process::current()->memory().copy_out(c.address, c.descriptors,
                                                     sizeof(c.descriptors)) >= 0;
  }
  static void detach(Context &c) noexcept
  {
    if (c.descriptors[0] >= 0)
    {
      c.table->take(c.descriptors[0]);
    }
    if (c.descriptors[1] >= 0)
    {
      c.table->take(c.descriptors[1]);
    }
  }
  static void release(Context &c) noexcept
  {
    if (c.master_file)
    {
      xv::fs::files().release(c.master_file);
    }
    if (c.slave_file)
    {
      xv::fs::files().release(c.slave_file);
    }
  }
  static void success(Context &c) noexcept { c.result = 0; }
  struct Allocate;
  struct Bind;
  struct Install;
  struct Copy;
  struct Success;
  struct Detach;
  struct Release;
  using Graph = flow::graph<
      Allocate,
      flow::types<flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&create>, flow::go<Bind>,
                                          flow::go<Release>>>,
                  flow::node<Bind, flow::action<&bind>, flow::go<Install>>,
                  flow::node<Install, flow::noop,
                             flow::branch<flow::condition<&install>, flow::go<Copy>,
                                          flow::go<Detach>>>,
                  flow::node<Copy, flow::noop,
                             flow::branch<flow::condition<&copy>, flow::go<Success>,
                                          flow::go<Detach>>>,
                  flow::node<Success, flow::action<&success>, flow::stop>,
                  flow::node<Detach, flow::action<&detach>, flow::go<Release>>,
                  flow::node<Release, flow::action<&release>, flow::stop>>>;
};
struct AttributesFlow
{
  struct Context
  {
    const char *path;
    bool ownership;
    int first, second;
    xv::fs::Inode *node = nullptr;
    bool valid = false;
    unsigned uid = 0, gid = 0, mode = 0;
  };

  static bool lookup(Context &c) noexcept
  {
    c.node = xv::fs::filesystem().lookup(c.path);
    return c.node;
  }
  static void lock(Context &c) noexcept { xv::fs::filesystem().lock(c.node); }
  static void assess(Context &c) noexcept
  {
    auto who = xv::process::current()->credentials().snapshot();
    c.valid = !c.node->owner().read_only() &&
              (who.effective_uid == 0 || who.effective_uid == c.node->uid());
    c.uid = c.node->uid(), c.gid = c.node->gid(), c.mode = c.node->mode();
    if (c.ownership)
    {
      c.valid &=
          c.first >= -1 && c.first <= 65534 && c.second >= -1 && c.second <= 65534;
      if (c.first >= 0)
      {
        c.valid &= who.effective_uid == 0 || static_cast<unsigned>(c.first) == c.uid;
        c.uid = c.first;
      }
      if (c.second >= 0)
      {
        bool member = who.effective_gid == static_cast<unsigned>(c.second);
        for (unsigned i = 0; i < who.group_count; ++i)
        {
          member |= who.groups[i] == static_cast<unsigned>(c.second);
        }
        c.valid &= who.effective_uid == 0 || member;
        c.gid = c.second;
      }
    }
    else
    {
      c.valid &= c.first >= 0 && !(c.first & ~07777) &&
                 (!(c.first & 06000) || c.node->kind() == T_FILE);
      c.mode = c.first;
      if (who.effective_uid != 0 && (c.mode & 02000))
      {
        bool member = who.effective_gid == c.gid;
        for (unsigned i = 0; i < who.group_count; ++i)
        {
          member |= who.groups[i] == c.gid;
        }
        if (!member)
        {
          c.mode &= ~02000U;
        }
      }
    }
    if (c.ownership)
    {
      c.mode &= ~06000U;
    }
  }
  static bool valid(Context &c) noexcept { return c.valid; }
  static void commit(Context &c) noexcept
  {
    c.node->owner().attributes(c.node, c.uid, c.gid, c.mode);
  }
  static void release(Context &c) noexcept
  {
    xv::fs::filesystem().unlock_release(c.node);
  }
  struct Lookup;
  struct Lock;
  struct Assess;
  struct Commit;
  struct Release;
  using Graph = flow::graph<
      Lookup, flow::types<flow::node<Lookup, flow::noop,
                                     flow::branch<flow::condition<&lookup>,
                                                  flow::go<Lock>, flow::stop>>,
                          flow::node<Lock, flow::action<&lock>, flow::go<Assess>>,
                          flow::node<Assess, flow::action<&assess>,
                                     flow::branch<flow::condition<&valid>,
                                                  flow::go<Commit>, flow::go<Release>>>,
                          flow::node<Commit, flow::action<&commit>, flow::go<Release>>,
                          flow::node<Release, flow::action<&release>, flow::stop>>>;
};
struct AccessFlow
{
  struct Context
  {
    const char *path;
    int mode;
    xv::fs::Inode *inode = nullptr;
    uint64 result = static_cast<uint64>(-1);
  };

  static bool lookup(Context &c) noexcept
  {
    c.inode = xv::fs::filesystem().lookup(c.path, true);
    return c.inode;
  }
  static void lock(Context &c) noexcept { xv::fs::filesystem().lock(c.inode); }
  static void assess(Context &c) noexcept
  {
    c.result = xv::fs::filesystem().allowed(c.inode, c.mode, true) ? 0 : -1;
  }
  static void release(Context &c) noexcept
  {
    xv::fs::filesystem().unlock_release(c.inode);
  }
  struct Lookup;
  struct Lock;
  struct Assess;
  struct Release;
  using Graph = flow::graph<
      Lookup, flow::types<flow::node<Lookup, flow::noop,
                                     flow::branch<flow::condition<&lookup>,
                                                  flow::go<Lock>, flow::stop>>,
                          flow::node<Lock, flow::action<&lock>, flow::go<Assess>>,
                          flow::node<Assess, flow::action<&assess>, flow::go<Release>>,
                          flow::node<Release, flow::action<&release>, flow::stop>>>;
};
struct StatFlow
{
  struct Context
  {
    const char *path;
    uint64 address;
    bool follow;
    xv::fs::Stat &value;
    xv::fs::Inode *inode = nullptr;
    uint64 result = static_cast<uint64>(-1);
  };

  static bool lookup(Context &c) noexcept
  {
    c.inode = xv::fs::filesystem().lookup(c.path, false, c.follow);
    return c.inode;
  }
  static void lock(Context &c) noexcept
  {
    c.value = {};
    xv::fs::filesystem().lock(c.inode);
  }
  static void snapshot(Context &c) noexcept
  {
    xv::fs::filesystem().stat(c.inode, &c.value);
  }
  static void release(Context &c) noexcept
  {
    xv::fs::filesystem().unlock_release(c.inode);
  }
  static void copy(Context &c) noexcept
  {
    c.result =
        xv::process::current()->memory().copy_out(c.address, &c.value, sizeof(c.value));
  }
  struct Lookup;
  struct Lock;
  struct Snapshot;
  struct Release;
  struct Copy;
  using Graph = flow::graph<
      Lookup,
      flow::types<flow::node<Lookup, flow::noop,
                             flow::branch<flow::condition<&lookup>, flow::go<Lock>,
                                          flow::stop>>,
                  flow::node<Lock, flow::action<&lock>, flow::go<Snapshot>>,
                  flow::node<Snapshot, flow::action<&snapshot>, flow::go<Release>>,
                  flow::node<Release, flow::action<&release>, flow::go<Copy>>,
                  flow::node<Copy, flow::action<&copy>, flow::stop>>>;
};
struct MountFlow
{
  struct Context
  {
    const char *path, *type;
    uint64 result = static_cast<uint64>(-1);
  };

  static bool disk(Context &c) noexcept
  {
    return !xv::runtime::compare_text(c.type, "xv6", 16);
  }
  static bool ram(Context &c) noexcept
  {
    return !xv::runtime::compare_text(c.type, "ramfs", 16);
  }
  static void mount_disk(Context &c) noexcept
  {
    c.result = xv::fs::filesystem().mount_disk(c.path);
  }
  static void mount_ram(Context &c) noexcept
  {
    c.result = xv::fs::filesystem().mount_ram(c.path);
  }
  struct Disk;
  struct Ram;
  struct MountDisk;
  struct MountRam;
  using Graph = flow::graph<
      Disk, flow::types<flow::node<Disk, flow::noop,
                                   flow::branch<flow::condition<&disk>,
                                                flow::go<MountDisk>, flow::go<Ram>>>,
                        flow::node<Ram, flow::noop,
                                   flow::branch<flow::condition<&ram>,
                                                flow::go<MountRam>, flow::stop>>,
                        flow::node<MountDisk, flow::action<&mount_disk>, flow::stop>,
                        flow::node<MountRam, flow::action<&mount_ram>, flow::stop>>>;
};
struct UnmountFlow
{
  struct Context
  {
    const char *path;
    int result = -1;
  };

  static void unmount(Context &c) noexcept
  {
    c.result = xv::fs::filesystem().unmount(c.path);
  }
  static bool removed(Context &c) noexcept { return c.result == 0; }
  static void sync(Context &) noexcept
  {
    xv::fs::journal().sync();
    if (xv::fs::Transaction::secondary)
    {
      xv::fs::Transaction::secondary->sync();
    }
  }
  struct Unmount;
  struct Sync;
  using Graph =
      flow::graph<Unmount,
                  flow::types<flow::node<Unmount, flow::action<&unmount>,
                                         flow::branch<flow::condition<&removed>,
                                                      flow::go<Sync>, flow::stop>>,
                              flow::node<Sync, flow::action<&sync>, flow::stop>>>;
};
struct MountInfoFlow
{
  struct Context
  {
    int index;
    uint64 address;
    mount_info &record;
    int result = 0;
  };

  static void describe(Context &c) noexcept
  {
    c.result = xv::fs::filesystem().mounts(c.index, c.record.path, c.record.type);
  }
  static bool has_result(Context &c) noexcept { return c.result != 0; }
  static bool copy(Context &c) noexcept
  {
    return xv::process::current()->memory().copy_out(c.address, &c.record,
                                                     sizeof(c.record)) >= 0;
  }
  static void bad_copy(Context &c) noexcept { c.result = -1; }
  struct Describe;
  struct Copy;
  struct BadCopy;
  using Graph = flow::graph<
      Describe, flow::types<flow::node<Describe, flow::action<&describe>,
                                       flow::branch<flow::condition<&has_result>,
                                                    flow::go<Copy>, flow::stop>>,
                            flow::node<Copy, flow::noop,
                                       flow::branch<flow::condition<&copy>, flow::stop,
                                                    flow::go<BadCopy>>>,
                            flow::node<BadCopy, flow::action<&bad_copy>, flow::stop>>>;
};
struct ReadlinkFlow
{
  struct Context
  {
    const char *path;
    uint64 address;
    int size;
    xv::fs::Inode *inode = nullptr;
    int result = -1;
  };

  static bool lookup(Context &c) noexcept
  {
    c.inode = xv::fs::filesystem().lookup(c.path, false, false);
    return c.inode;
  }
  static void lock(Context &c) noexcept { xv::fs::filesystem().lock(c.inode); }
  static bool symlink(Context &c) noexcept { return c.inode->kind() == T_SYMLINK; }
  static void read(Context &c) noexcept
  {
    c.result = c.inode->owner().read(c.inode, 1, c.address, 0, c.size);
  }
  static void release(Context &c) noexcept
  {
    xv::fs::filesystem().unlock_release(c.inode);
  }
  struct Lookup;
  struct Lock;
  struct Read;
  struct Release;
  using Graph = flow::graph<
      Lookup, flow::types<flow::node<Lookup, flow::noop,
                                     flow::branch<flow::condition<&lookup>,
                                                  flow::go<Lock>, flow::stop>>,
                          flow::node<Lock, flow::action<&lock>,
                                     flow::branch<flow::condition<&symlink>,
                                                  flow::go<Read>, flow::go<Release>>>,
                          flow::node<Read, flow::action<&read>, flow::go<Release>>,
                          flow::node<Release, flow::action<&release>, flow::stop>>>;
};

} // namespace

uint64 sys_open(void)
{
  char path[MAXPATH];
  int mode;
  argument_integer(1, &mode);
  if (argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  xv::fs::Transaction transaction;
  OpenFlow::Context context{path, mode};
  (void)OpenFlow::Graph::run<0>(context);
  return context.result;
}

uint64 sys_mkdir(void)
{
  char path[MAXPATH];
  xv::fs::Transaction transaction;
  if (argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  CreateNodeFlow::Context context{path, T_DIR, 0, 0};
  (void)CreateNodeFlow::Graph::run<0>(context);
  return context.result;
}

uint64 sys_mknod(void)
{
  char path[MAXPATH];
  int major, minor;
  xv::fs::Transaction transaction;
  argument_integer(1, &major);
  argument_integer(2, &minor);
  if (argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  CreateNodeFlow::Context context{path, T_DEVICE, major, minor};
  (void)CreateNodeFlow::Graph::run<0>(context);
  return context.result;
}

uint64 sys_chdir(void)
{
  char path[MAXPATH];
  auto *process = xv::process::current();
  xv::fs::Transaction transaction;
  if (argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  DirectoryFlow::Context context{process, path};
  (void)DirectoryFlow::Graph::run<0>(context);
  return context.result;
}

namespace
{
struct ArgumentVector
{
  struct CopyFlow;
  char *items[MAXARG]{};
  ~ArgumentVector()
  {
    for (auto *item : items)
    {
      if (item)
      {
        xv::memory::allocator().deallocate(item);
      }
    }
  }
  bool copy(uint64 address);
};
struct ArgumentVector::CopyFlow
{
  struct Context
  {
    ArgumentVector &owner;
    uint64 address;
    unsigned index = 0;
    uint64 argument = 0;
    bool result = false;
  };
  static bool present(Context &c) noexcept { return c.address != 0; }
  static bool fetch(Context &c) noexcept
  {
    return c.address <= ~0UL - c.index * sizeof(uint64) &&
           fetch_address(c.address + c.index * sizeof(uint64), &c.argument) >= 0;
  }
  static bool terminated(Context &c) noexcept { return c.argument == 0; }
  static bool capacity(Context &c) noexcept { return c.index != MAXARG - 1; }
  static bool allocate(Context &c) noexcept
  {
    c.owner.items[c.index] = static_cast<char *>(xv::memory::allocator().allocate());
    return c.owner.items[c.index];
  }
  static bool copy_string(Context &c) noexcept
  {
    return fetch_string(c.argument, c.owner.items[c.index], PGSIZE) >= 0;
  }
  static void advance(Context &c) noexcept { ++c.index; }
  static void success(Context &c) noexcept { c.result = true; }
  struct Present;
  struct Fetch;
  struct Terminated;
  struct Capacity;
  struct Allocate;
  struct CopyString;
  struct Advance;
  struct Success;
  using Graph = flow::graph<
      Present,
      flow::types<flow::node<Present, flow::noop,
                             flow::branch<flow::condition<&present>, flow::go<Fetch>,
                                          flow::stop>>,
                  flow::node<Fetch, flow::noop,
                             flow::branch<flow::condition<&fetch>, flow::go<Terminated>,
                                          flow::stop>>,
                  flow::node<Terminated, flow::noop,
                             flow::branch<flow::condition<&terminated>,
                                          flow::go<Success>, flow::go<Capacity>>>,
                  flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&capacity>,
                                          flow::go<Allocate>, flow::stop>>,
                  flow::node<Allocate, flow::noop,
                             flow::branch<flow::condition<&allocate>,
                                          flow::go<CopyString>, flow::stop>>,
                  flow::node<CopyString, flow::noop,
                             flow::branch<flow::condition<&copy_string>,
                                          flow::go<Advance>, flow::stop>>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Fetch>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

bool ArgumentVector::copy(uint64 address)
{
  CopyFlow::Context context{*this, address};
  (void)CopyFlow::Graph::run<0>(context);
  return context.result;
}

struct ExecuteImageFlow
{
  struct Context
  {
    const char *path;
    uint64 arguments, environment;
    ArgumentVector &argv, &envp;
    uint64 result = static_cast<uint64>(-1);
  };
  static bool arguments(Context &c) noexcept { return c.argv.copy(c.arguments); }
  static bool environment(Context &c) noexcept
  {
    return !c.environment || c.envp.copy(c.environment);
  }
  static void execute(Context &c) noexcept
  {
    c.result = xv::process::execute(c.path, c.argv.items, c.envp.items);
  }
  struct Arguments;
  struct Environment;
  struct Execute;
  using Graph = flow::graph<
      Arguments,
      flow::types<flow::node<Arguments, flow::noop,
                             flow::branch<flow::condition<&arguments>,
                                          flow::go<Environment>, flow::stop>>,
                  flow::node<Environment, flow::noop,
                             flow::branch<flow::condition<&environment>,
                                          flow::go<Execute>, flow::stop>>,
                  flow::node<Execute, flow::action<&execute>, flow::stop>>>;
};

uint64 execute_image(bool with_environment)
{
  char path[MAXPATH];
  uint64 arguments, environment = 0;
  argument_address(1, &arguments);
  if (with_environment)
  {
    argument_address(2, &environment);
  }
  if (argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  ArgumentVector argv, envp;
  ExecuteImageFlow::Context context{path, arguments, environment, argv, envp};
  (void)ExecuteImageFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace
uint64 sys_exec() { return execute_image(false); }
uint64 sys_execve() { return execute_image(true); }

uint64 sys_pipe(void)
{
  uint64 address;
  auto *process = xv::process::current();
  argument_address(0, &address);
  PipeFlow::Context context{process, address};
  (void)PipeFlow::Graph::run<0>(context);
  return context.result;
}

uint64 sys_openpty(void)
{
  uint64 address;
  argument_address(0, &address);
  PtyOpenFlow::Context context{address};
  (void)PtyOpenFlow::Graph::run<0>(context);
  return context.result;
}

uint64 sys_fcntl(void)
{
  xv::fs::File *file;
  int command, flags = 0;
  argument_integer(1, &command);
  if (command == F_SETFL)
  {
    argument_integer(2, &flags);
  }
  if (argfd(0, nullptr, &file) < 0)
  {
    return -1;
  }
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
  if (xv::fs::Transaction::secondary)
  {
    xv::fs::Transaction::secondary->sync();
  }
  return 0;
}
namespace
{
uint64 attributes(bool ownership)
{
  char path[MAXPATH];
  int first, second = -1;
  argument_integer(1, &first);
  if (ownership)
  {
    argument_integer(2, &second);
  }
  if (argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  xv::fs::Transaction transaction;
  AttributesFlow::Context context{path, ownership, first, second};
  (void)AttributesFlow::Graph::run<0>(context);
  return context.valid ? 0 : -1;
}
} // namespace
uint64 sys_chmod() { return attributes(false); }
uint64 sys_chown() { return attributes(true); }
uint64 sys_access()
{
  char path[MAXPATH];
  int mode;
  argument_integer(1, &mode);
  if ((mode & ~7) || argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  xv::fs::Transaction transaction;
  AccessFlow::Context context{path, mode};
  (void)AccessFlow::Graph::run<0>(context);
  return context.result;
}
uint64 sys_stat()
{
  char path[MAXPATH];
  uint64 address;
  argument_address(1, &address);
  if (argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  xv::fs::Transaction transaction;
  xv::fs::Stat value;
  StatFlow::Context context{path, address, true, value};
  (void)StatFlow::Graph::run<0>(context);
  return context.result;
}

uint64 sys_rename()
{
  char old[MAXPATH], next[MAXPATH];
  if (argument_string(0, old, MAXPATH) <= 0 || argument_string(1, next, MAXPATH) <= 0)
  {
    return -1;
  }
  const char *paths[] = {old, next};
  for (const char *path : paths)
  {
    unsigned component = 0;
    for (const char *p = path; *p; ++p)
    {
      if (*p == '/')
      {
        component = 0;
      }
      else if (++component > DIRSIZ)
      {
        return -1;
      }
    }
    if (!component)
    {
      return -1;
    }
  }
  return xv::fs::filesystem().rename(old, next);
}
uint64 sys_mount()
{
  if (xv::process::current()->credentials().snapshot().effective_uid != 0)
  {
    return -1;
  }
  char path[MAXPATH], type[16];
  if (argument_string(0, type, sizeof(type)) < 0 ||
      argument_string(1, path, sizeof(path)) < 0 || !type[0])
  {
    return -1;
  }
  MountFlow::Context context{path, type};
  (void)MountFlow::Graph::run<0>(context);
  return context.result;
}
uint64 sys_umount()
{
  if (xv::process::current()->credentials().snapshot().effective_uid != 0)
  {
    return -1;
  }
  char path[MAXPATH];
  if (argument_string(0, path, sizeof(path)) < 0)
  {
    return -1;
  }
  UnmountFlow::Context context{path};
  (void)UnmountFlow::Graph::run<0>(context);
  return context.result;
}
uint64 sys_mountinfo()
{
  int index;
  uint64 address;
  argument_integer(0, &index);
  argument_address(1, &address);
  if (index < 0)
  {
    return -1;
  }
  mount_info record{};
  MountInfoFlow::Context context{index, address, record};
  (void)MountInfoFlow::Graph::run<0>(context);
  return context.result;
}

uint64 sys_lseek()
{
  xv::fs::File *file;
  uint64 offset;
  int origin;
  argument_address(1, &offset);
  argument_integer(2, &origin);
  return argfd(0, nullptr, &file) < 0
             ? -1
             : xv::fs::files().seek(file, static_cast<long>(offset), origin);
}
uint64 sys_ftruncate()
{
  xv::fs::File *file;
  uint64 size;
  argument_address(1, &size);
  return size > 0x7fffffffUL || argfd(0, nullptr, &file) < 0
             ? -1
             : xv::fs::files().resize(file, size);
}
uint64 sys_fsync()
{
  xv::fs::File *file;
  return argfd(0, nullptr, &file) < 0 ? -1 : xv::fs::files().sync(file);
}
uint64 sys_symlink()
{
  char target[MAXPATH], path[MAXPATH];
  if (argument_string(0, target, sizeof(target)) < 0 ||
      argument_string(1, path, sizeof(path)) < 0)
  {
    return -1;
  }
  return xv::fs::filesystem().symlink(target, path);
}
uint64 sys_readlink()
{
  char path[MAXPATH];
  uint64 address;
  int size;
  argument_address(1, &address);
  argument_integer(2, &size);
  if (size < 0 || argument_string(0, path, sizeof(path)) < 0)
  {
    return -1;
  }
  xv::fs::Transaction transaction;
  ReadlinkFlow::Context context{path, address, size};
  (void)ReadlinkFlow::Graph::run<0>(context);
  return context.result;
}
uint64 sys_lstat()
{
  char path[MAXPATH];
  uint64 address;
  argument_address(1, &address);
  if (argument_string(0, path, MAXPATH) < 0)
  {
    return -1;
  }
  xv::fs::Transaction transaction;
  xv::fs::Stat value;
  StatFlow::Context context{path, address, false, value};
  (void)StatFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::syscall
