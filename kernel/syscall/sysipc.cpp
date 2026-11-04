module;
#include "kernel/net/socket/socket.h"
#include "kernel/io/posix_errno.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/mman.h"
#include "kernel/arch/riscv/register_constants.h"
module xv.syscall;
import :arguments;
import :handlers;
import xv.ipc;
import xv.fs;
import xv.process;
import xv.runtime;
import xv.memory;

namespace xv::syscall
{
namespace
{
xv::fs::File *
file_argument() {
  int descriptor;
  argument_integer(0, &descriptor);
  return xv::process::current()->files().get(descriptor);
}

int
vectors(unsigned long address, msghdr &message, iovec *iov) {
  auto &memory = xv::process::current()->memory();
  if (memory.copy_in(&message, address, sizeof(message)) < 0)
  {
    return -EFAULT;
  }
  if (message.msg_iovlen < 0 || message.msg_iovlen > int(xv::ipc::MaxVectors))
  {
    return -EMSGSIZE;
  }
  if (message.msg_iovlen &&
      memory.copy_in(iov, reinterpret_cast<unsigned long>(message.msg_iov),
                     message.msg_iovlen * sizeof(iovec)) < 0)
  {
    return -EFAULT;
  }
  return 0;
}
} // namespace

unsigned long
sys_socketpair() {
  int domain, type, protocol;
  unsigned long address;
  argument_integer(0, &domain);
  argument_integer(1, &type);
  argument_integer(2, &protocol);
  argument_address(3, &address);
  if (domain != AF_UNIX)
  {
    return -EAFNOSUPPORT;
  }
  unsigned options = type & (SOCK_NONBLOCK | SOCK_CLOEXEC);
  type &= ~(SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (protocol || (type != SOCK_STREAM && type != SOCK_DGRAM && type != SOCK_SEQPACKET))
  {
    return -EOPNOTSUPP;
  }
  xv::io::IoObject *objects[2]{};
  if (!xv::ipc::socket_pair(type, objects))
  {
    return -ENOMEM;
  }
  xv::fs::File *files[2]{};
  int descriptors[2]{-1, -1};
  int error = -ENFILE;
  for (unsigned i = 0; i < 2; ++i)
  {
    files[i] = xv::fs::files().allocate();
    if (!files[i])
    {
      break;
    }
    xv::fs::files().bind_object(*files[i], *objects[i], options & SOCK_NONBLOCK);
    xv::io::query<xv::ipc::LocalSocket>(objects[i])->attach_file(files[i]);
    descriptors[i] = xv::process::current()->files().install(
        files[i], options & SOCK_CLOEXEC ? FD_CLOEXEC : 0);
    if (descriptors[i] < 0)
    {
      error = -EMFILE;
      break;
    }
  }
  bool success = descriptors[0] >= 0 && descriptors[1] >= 0;
  if (success && xv::process::current()->memory().copy_out(address, descriptors,
                                                           sizeof(descriptors)) < 0)
  {
    success = false;
    error = -EFAULT;
  }
  if (!success)
  {
    for (unsigned i = 0; i < 2; ++i)
    {
      if (descriptors[i] >= 0)
      {
        xv::process::current()->files().close(descriptors[i]);
      }
      else if (files[i])
      {
        xv::fs::files().release(files[i]);
      }
      else
      {
        objects[i]->close();
      }
    }
  }
  return success ? 0 : error;
}

unsigned long
sys_sendmsg() {
  auto *file = file_argument();
  if (!file)
  {
    return -EBADF;
  }
  auto *socket = xv::io::query<xv::ipc::LocalSocket>(xv::fs::files().object(file));
  if (!socket)
  {
    return xv::fs::files().object(file)->capability(xv::io::CapabilityId::Socket)
               ? -EOPNOTSUPP
               : -ENOTSOCK;
  }
  unsigned long address;
  int flags;
  argument_address(1, &address);
  argument_integer(2, &flags);
  msghdr message;
  iovec iov[xv::ipc::MaxVectors];
  int error = vectors(address, message, iov);
  if (error < 0)
  {
    return error;
  }
  if (message.msg_name && message.msg_namelen)
  {
    return -EISCONN;
  }
  alignas(unsigned long) unsigned char
      control[CMSG_SPACE(xv::ipc::MaxRights * sizeof(int))]{};
  if (message.msg_controllen > sizeof(control))
  {
    return -EMSGSIZE;
  }
  xv::ipc::Rights rights;
  if (message.msg_controllen)
  {
    if (xv::process::current()->memory().copy_in(
            control, reinterpret_cast<unsigned long>(message.msg_control),
            message.msg_controllen) < 0)
    {
      return -EFAULT;
    }
    unsigned at = 0;
    while (at < message.msg_controllen)
    {
      if (message.msg_controllen - at < sizeof(cmsghdr))
      {
        return -EINVAL;
      }
      auto &header = *reinterpret_cast<const cmsghdr *>(control + at);
      if (header.cmsg_len < CMSG_LEN(0) ||
          header.cmsg_len > message.msg_controllen - at ||
          header.cmsg_level != SOL_SOCKET || header.cmsg_type != SCM_RIGHTS ||
          (header.cmsg_len - CMSG_LEN(0)) % sizeof(int))
      {
        return -EINVAL;
      }
      unsigned count = (header.cmsg_len - CMSG_LEN(0)) / sizeof(int);
      if (count > xv::ipc::MaxRights - rights.count)
      {
        return -EMSGSIZE;
      }
      auto *descriptors = reinterpret_cast<const int *>(CMSG_DATA(&header));
      for (unsigned i = 0; i < count; ++i)
      {
        auto *passed = xv::process::current()->files().get(descriptors[i]);
        if (!passed)
        {
          return -EBADF;
        }
        rights.files[rights.count++] = passed;
      }
      unsigned next = CMSG_ALIGN(header.cmsg_len);
      if (next > message.msg_controllen - at)
      {
        break;
      }
      at += next;
    }
  }
  if (file->nonblocking())
  {
    flags |= MSG_DONTWAIT;
  }
  return socket->send(iov, message.msg_iovlen, true, flags, rights);
}

unsigned long
sys_recvmsg() {
  auto *file = file_argument();
  if (!file)
  {
    return -EBADF;
  }
  auto *socket = xv::io::query<xv::ipc::LocalSocket>(xv::fs::files().object(file));
  if (!socket)
  {
    return xv::fs::files().object(file)->capability(xv::io::CapabilityId::Socket)
               ? -EOPNOTSUPP
               : -ENOTSOCK;
  }
  unsigned long address;
  int flags;
  argument_address(1, &address);
  argument_integer(2, &flags);
  msghdr message;
  iovec iov[xv::ipc::MaxVectors];
  int error = vectors(address, message, iov);
  if (error < 0)
  {
    return error;
  }
  if (file->nonblocking())
  {
    flags |= MSG_DONTWAIT;
  }
  xv::ipc::Rights rights;
  unsigned output_flags = 0;
  int result =
      socket->receive(iov, message.msg_iovlen, true, flags, rights, output_flags);
  if (result < 0)
  {
    xv::ipc::release_rights(rights);
    return result;
  }
  alignas(unsigned long) unsigned char
      control[CMSG_SPACE(xv::ipc::MaxRights * sizeof(int))]{};
  unsigned capacity = message.msg_controllen;
  unsigned fit =
      capacity >= CMSG_LEN(sizeof(int)) ? (capacity - CMSG_LEN(0)) / sizeof(int) : 0;
  if (fit > rights.count)
  {
    fit = rights.count;
  }
  int installed[xv::ipc::MaxRights]{};
  unsigned count = 0;
  for (; count < fit; ++count)
  {
    auto *owned = static_cast<xv::fs::File *>(rights.files[count]);
    installed[count] = xv::process::current()->files().install(
        owned, flags & MSG_CMSG_CLOEXEC ? FD_CLOEXEC : 0);
    if (installed[count] < 0)
    {
      break;
    }
    rights.files[count] = nullptr;
  }
  if (count < rights.count)
  {
    output_flags |= MSG_CTRUNC;
  }
  message.msg_flags = output_flags;
  message.msg_namelen = 0;
  message.msg_controllen = count ? CMSG_LEN(count * sizeof(int)) : 0;
  if (count)
  {
    auto &header = *reinterpret_cast<cmsghdr *>(control);
    header.cmsg_len = message.msg_controllen;
    header.cmsg_level = SOL_SOCKET;
    header.cmsg_type = SCM_RIGHTS;
    xv::runtime::move(CMSG_DATA(&header), installed, count * sizeof(int));
    if (xv::process::current()->memory().copy_out(
            reinterpret_cast<unsigned long>(message.msg_control), control,
            message.msg_controllen) < 0)
    {
      error = -EFAULT;
    }
  }
  if (!error &&
      xv::process::current()->memory().copy_out(address, &message, sizeof(message)) < 0)
  {
    error = -EFAULT;
  }
  for (unsigned i = 0; i < rights.count; ++i)
  {
    if (rights.files[i])
    {
      xv::fs::files().release(static_cast<xv::fs::File *>(rights.files[i]));
    }
  }
  if (error)
  {
    for (unsigned i = 0; i < count; ++i)
    {
      xv::process::current()->files().close(installed[i]);
    }
    return error;
  }
  return result;
}
} // namespace xv::syscall

namespace xv::syscall
{
namespace
{
int
shared_name(unsigned long address, char name[256]) {
  for (unsigned i = 0; i < 256; ++i)
  {
    if (address + i < address ||
        xv::process::current()->memory().copy_in(name + i, address + i, 1) < 0)
    {
      return -EFAULT;
    }
    if (!name[i])
    {
      return 0;
    }
  }
  return -ENAMETOOLONG;
}
} // namespace

unsigned long
sys_shm_open() {
  unsigned long address;
  int flags, mode;
  argument_address(0, &address);
  argument_integer(1, &flags);
  argument_integer(2, &mode);
  char name[256];
  int error = shared_name(address, name);
  if (error)
  {
    return error;
  }
  auto *object = xv::ipc::shared_open(name, flags, mode, error);
  if (!object)
  {
    return error;
  }
  auto *file = xv::fs::files().allocate();
  if (!file)
  {
    object->close();
    return -ENFILE;
  }
  xv::fs::files().bind_object(*file, *object, false, true,
                              (flags & O_ACCMODE) == O_RDWR);
  int descriptor = xv::process::current()->files().install(file, FD_CLOEXEC);
  if (descriptor < 0)
  {
    xv::fs::files().release(file);
    return -EMFILE;
  }
  return descriptor;
}

unsigned long
sys_shm_unlink() {
  unsigned long address;
  argument_address(0, &address);
  char name[256];
  int error = shared_name(address, name);
  return error ? error : xv::ipc::shared_unlink(name);
}

unsigned long
sys_mmap() {
  unsigned long hint, bytes, offset;
  int protection, flags, descriptor;
  argument_address(0, &hint);
  argument_address(1, &bytes);
  argument_integer(2, &protection);
  argument_integer(3, &flags);
  argument_integer(4, &descriptor);
  argument_address(5, &offset);
  if (!bytes || bytes > 256UL * 4096 || offset % 4096 ||
      (protection & ~(PROT_READ | PROT_WRITE)) ||
      (flags & ~(MAP_SHARED | MAP_PRIVATE)) ||
      ((flags & (MAP_SHARED | MAP_PRIVATE)) != MAP_SHARED &&
       (flags & (MAP_SHARED | MAP_PRIVATE)) != MAP_PRIVATE))
  {
    return -EINVAL;
  }
  auto *file = xv::process::current()->files().get(descriptor);
  if (!file)
  {
    return -EBADF;
  }
  auto *object = xv::io::query<xv::ipc::SharedMemory>(xv::fs::files().object(file));
  if (!object)
  {
    return -ENODEV;
  }
  if ((flags & MAP_SHARED) && (protection & PROT_WRITE) && !object->writable())
  {
    return -EACCES;
  }
  unsigned count = (bytes + 4095) / 4096;
  if (offset / 4096 > 256 || count > 256 - offset / 4096)
  {
    return -EINVAL;
  }
  void *pages[256];
  int error = object->pin(offset / 4096, count, pages);
  if (error)
  {
    return error;
  }
  unsigned permissions =
      protection ? PTE_U | PTE_R | ((protection & PROT_WRITE) ? PTE_W : 0) : 0;
  auto result = xv::process::current()->memory().map_shared_pages(
      pages, count, permissions, flags & MAP_PRIVATE, hint);
  for (unsigned i = 0; i < count; ++i)
  {
    xv::memory::allocator().release_page(pages[i]);
  }
  return result ? result : static_cast<unsigned long>(-ENOMEM);
}

unsigned long
sys_munmap() {
  unsigned long address, length;
  argument_address(0, &address);
  argument_address(1, &length);
  return xv::process::current()->memory().unmap_shared_pages(address, length);
}
} // namespace xv::syscall
