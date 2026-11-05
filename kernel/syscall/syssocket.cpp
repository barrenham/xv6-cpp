module;
#include "kernel/net/errno.h"
#include "kernel/io/posix_errno.h"
#include "kernel/fs/fcntl.h"
#include "kernel/net/socket/socket.h"
#include "kernel/support/flow.h"
module xv.syscall;
import :arguments;
import :handlers;
import xv.net;
import xv.ipc;
import xv.process;
import xv.fs;
namespace xv::syscall
{
namespace
{
xv::net::Socket *socket_argument(xv::fs::File **file = nullptr)
{
  int fd;
  argument_integer(0, &fd);
  auto *f = xv::process::current()->files().get(fd);
  if (file)
  {
    *file = f;
  }
  return xv::io::query<xv::net::Socket>(xv::fs::files().object(f));
}

xv::ipc::LocalSocket *
local_argument() {
  int descriptor;
  argument_integer(0, &descriptor);
  return xv::io::query<xv::ipc::LocalSocket>(
      xv::fs::files().object(xv::process::current()->files().get(descriptor)));
}

int
socket_argument_error() {
  int descriptor;
  argument_integer(0, &descriptor);
  return xv::process::current()->files().get(descriptor) ? -ENOTSOCK : -EBADF;
}

int
unix_path(unsigned long address, int length, sockaddr_un &value) {
  if (length <= int(sizeof(value.sun_family)) || length > int(sizeof(value)))
  { return -EINVAL; }
  value = {};
  if (xv::process::current()->memory().copy_in(&value, address, unsigned(length)) < 0)
  { return -EFAULT; }
  if (value.sun_family != AF_UNIX) { return -EAFNOSUPPORT; }
  if (!value.sun_path[0]) { return -EOPNOTSUPP; } // Abstract names are not pathname sockets.
  for (unsigned i = 0; i < sizeof(value.sun_path); ++i)
  { if (!value.sun_path[i]) { return 0; } }
  return -ENAMETOOLONG;
}

int
unix_name(unsigned long address, unsigned long length, xv::ipc::LocalSocket &socket,
          bool peer) {
  socklen_t capacity;
  auto &memory = xv::process::current()->memory();
  if (!address || !length || memory.copy_in(&capacity, length, sizeof(capacity)) < 0)
  { return -EFAULT; }
  sockaddr_un value{};
  value.sun_family = AF_UNIX;
  int error = socket.address(peer, value.sun_path);
  if (error) { return error; }
  socklen_t actual = sizeof(value.sun_family);
  if (value.sun_path[0])
  {
    unsigned bytes = 0;
    while (bytes < sizeof(value.sun_path) && value.sun_path[bytes]) { ++bytes; }
    actual += bytes + 1;
  }
  if (capacity > actual) { capacity = actual; }
  if (memory.copy_out(address, &value, capacity) < 0 ||
      memory.copy_out(length, &actual, sizeof(actual)) < 0)
  { return -EFAULT; }
  return 0;
}

int
install_local(xv::io::IoObject *object, unsigned options) {
  auto *file = xv::fs::files().allocate();
  if (!file) { object->close(); return -ENFILE; }
  xv::fs::files().bind_object(*file, *object, options & SOCK_NONBLOCK);
  xv::io::query<xv::ipc::LocalSocket>(object)->attach_file(file);
  int descriptor = xv::process::current()->files().install(
      file, options & SOCK_CLOEXEC ? FD_CLOEXEC : 0);
  if (descriptor < 0) { xv::fs::files().release(file); return -EMFILE; }
  return descriptor;
}
namespace flow = xv::flow;
struct SocketInstallFlow
{
  struct Context
  {
    xv::net::Socket *socket;
    bool nonblocking;
    xv::fs::File *file = nullptr;
    int descriptor = -1;
  };

  static bool allocate(Context &c) noexcept
  {
    c.file = xv::fs::files().allocate();
    return c.file;
  }
  static void close(Context &c) noexcept { c.socket->close(); }
  static void bind(Context &c) noexcept
  {
    xv::fs::files().bind_object(*c.file, *c.socket, c.nonblocking);
  }
  static bool install(Context &c) noexcept
  {
    c.descriptor = xv::process::current()->files().install(c.file);
    return c.descriptor >= 0;
  }
  static void release(Context &c) noexcept { xv::fs::files().release(c.file); }
  struct Allocate;
  struct Bind;
  struct Install;
  struct Release;
  struct Close;
  using Graph =
      flow::graph<Allocate,
                  flow::types<flow::node<Allocate, flow::noop,
                                         flow::branch<flow::condition<&allocate>,
                                                      flow::go<Bind>, flow::go<Close>>>,
                              flow::node<Bind, flow::action<&bind>, flow::go<Install>>,
                              flow::node<Install, flow::noop,
                                         flow::branch<flow::condition<&install>,
                                                      flow::stop, flow::go<Release>>>,
                              flow::node<Release, flow::action<&release>, flow::stop>,
                              flow::node<Close, flow::action<&close>, flow::stop>>>;
};
struct SocketAddressFlow
{
  struct Context
  {
    xv::process::AddressSpace &memory;
    unsigned long address, length;
    xv::net::Endpoint endpoint;
    socklen_t &capacity;
    sockaddr_in &value;
    socklen_t actual = sizeof(sockaddr_in);
    int result = -1;
  };

  static bool capacity(Context &c) noexcept
  {
    return c.memory.copy_in(&c.capacity, c.length, sizeof(c.capacity)) >= 0;
  }
  static void prepare(Context &c) noexcept
  {
    c.value = {};
    c.value.sin_family = AF_INET;
    c.value.sin_port = htons(c.endpoint.port);
    c.value.sin_addr.s_addr = htonl(c.endpoint.address);
    if (c.capacity > c.actual)
    {
      c.capacity = c.actual;
    }
  }
  static bool copy_address(Context &c) noexcept
  {
    return c.memory.copy_out(c.address, &c.value, c.capacity) >= 0;
  }
  static bool copy_length(Context &c) noexcept
  {
    return c.memory.copy_out(c.length, &c.actual, sizeof(c.actual)) >= 0;
  }
  static void success(Context &c) noexcept { c.result = 0; }
  struct Capacity;
  struct Prepare;
  struct Address;
  struct Length;
  struct Success;
  using Graph = flow::graph<
      Capacity,
      flow::types<flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&capacity>, flow::go<Prepare>,
                                          flow::stop>>,
                  flow::node<Prepare, flow::action<&prepare>, flow::go<Address>>,
                  flow::node<Address, flow::noop,
                             flow::branch<flow::condition<&copy_address>,
                                          flow::go<Length>, flow::stop>>,
                  flow::node<Length, flow::noop,
                             flow::branch<flow::condition<&copy_length>,
                                          flow::go<Success>, flow::stop>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

int install(xv::net::Socket *socket, bool nonblocking)
{
  if (!socket)
  {
    return -1;
  }
  SocketInstallFlow::Context context{socket, nonblocking};
  (void)SocketInstallFlow::Graph::run<0>(context);
  return context.descriptor;
}
bool input_address(unsigned long address, unsigned length, xv::net::Endpoint &endpoint)
{
  sockaddr_in value;
  if (length < sizeof(value) ||
      xv::process::current()->memory().copy_in(&value, address, sizeof(value)) < 0 ||
      value.sin_family != AF_INET)
  {
    return false;
  }
  endpoint = {ntohl(value.sin_addr.s_addr), ntohs(value.sin_port)};
  return true;
}
int output_address(unsigned long address, unsigned long length,
                   xv::net::Endpoint endpoint)
{
  if (!address)
  {
    return 0;
  }
  socklen_t capacity;
  auto &memory = xv::process::current()->memory();
  sockaddr_in value;
  SocketAddressFlow::Context context{memory,   address,  length,
                                     endpoint, capacity, value};
  (void)SocketAddressFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace
namespace
{
struct SocketBindFlow
{
  struct Context
  {
    xv::net::Socket *socket;
    unsigned long address;
    int length;
    xv::net::Endpoint &local;
    int result = -1;
  };

  static bool copy(Context &c) noexcept
  {
    return input_address(c.address, c.length, c.local);
  }
  static bool permitted(Context &c) noexcept
  {
    return !(c.local.port && c.local.port < 1024 &&
             xv::process::current()->credentials().snapshot().effective_uid != 0);
  }
  static void bind(Context &c) noexcept { c.result = c.socket->bind(c.local); }
  struct Copy;
  struct Permission;
  struct Bind;
  using Graph = flow::graph<
      Copy, flow::types<flow::node<Copy, flow::noop,
                                   flow::branch<flow::condition<&copy>,
                                                flow::go<Permission>, flow::stop>>,
                        flow::node<Permission, flow::noop,
                                   flow::branch<flow::condition<&permitted>,
                                                flow::go<Bind>, flow::stop>>,
                        flow::node<Bind, flow::action<&bind>, flow::stop>>>;
};
struct SocketConnectFlow
{
  struct Context
  {
    xv::net::Socket *socket;
    xv::fs::File *file;
    unsigned long address;
    int length;
    xv::net::Endpoint &peer;
    int result = -1;
  };

  static bool copy(Context &c) noexcept
  {
    return input_address(c.address, c.length, c.peer);
  }
  static void connect(Context &c) noexcept
  {
    c.result = c.socket->connect(c.peer, c.file->nonblocking());
  }
  struct Copy;
  struct Connect;
  using Graph =
      flow::graph<Copy,
                  flow::types<flow::node<Copy, flow::noop,
                                         flow::branch<flow::condition<&copy>,
                                                      flow::go<Connect>, flow::stop>>,
                              flow::node<Connect, flow::action<&connect>, flow::stop>>>;
};
struct SocketAcceptFlow
{
  struct Context
  {
    xv::net::Socket *socket;
    xv::fs::File *file;
    unsigned long address, length;
    xv::net::Socket *child = nullptr;
    int error = 0;
    int result = -1;
  };

  static bool accept(Context &c) noexcept
  {
    c.child = c.socket->accept(c.file->nonblocking(), c.error);
    return c.child;
  }
  static void rejected(Context &c) noexcept { c.result = c.error; }
  static bool copy(Context &c) noexcept
  {
    return output_address(c.address, c.length, c.child->peer()) >= 0;
  }
  static void close(Context &c) noexcept { c.child->close(); }
  static void install(Context &c) noexcept
  {
    c.result = xv::syscall::install(c.child, false);
  }
  struct Accept;
  struct Copy;
  struct Install;
  struct Close;
  struct Rejected;
  using Graph = flow::graph<
      Accept, flow::types<flow::node<Accept, flow::noop,
                                     flow::branch<flow::condition<&accept>,
                                                  flow::go<Copy>, flow::go<Rejected>>>,
                          flow::node<Copy, flow::noop,
                                     flow::branch<flow::condition<&copy>,
                                                  flow::go<Install>, flow::go<Close>>>,
                          flow::node<Install, flow::action<&install>, flow::stop>,
                          flow::node<Close, flow::action<&close>, flow::stop>,
                          flow::node<Rejected, flow::action<&rejected>, flow::stop>>>;
};
struct SocketSendFlow
{
  struct Context
  {
    xv::net::Socket *socket;
    xv::fs::File *file;
    unsigned long data, address;
    int length, flags, address_length;
    xv::net::Endpoint &peer;
    int result = -1;
  };

  static bool addressed(Context &c) noexcept { return c.address != 0; }
  static bool copy(Context &c) noexcept
  {
    return input_address(c.address, c.address_length, c.peer);
  }
  static void send(Context &c) noexcept
  {
    c.result = c.socket->send_to(
        {c.data, c.file->nonblocking() || bool(c.flags & MSG_DONTWAIT)}, c.length,
        c.address ? &c.peer : nullptr);
  }
  struct Addressed;
  struct Copy;
  struct Send;
  using Graph = flow::graph<
      Addressed,
      flow::types<
          flow::node<Addressed, flow::noop,
                     flow::branch<flow::condition<&addressed>, flow::go<Copy>,
                                  flow::go<Send>>>,
          flow::node<Copy, flow::noop,
                     flow::branch<flow::condition<&copy>, flow::go<Send>, flow::stop>>,
          flow::node<Send, flow::action<&send>, flow::stop>>>;
};
struct SocketReceiveFlow
{
  struct Context
  {
    xv::net::Socket *socket;
    xv::fs::File *file;
    unsigned long data, address, address_length;
    int length, flags;
    xv::net::Endpoint &peer;
    int result = -1;
  };

  static void receive(Context &c) noexcept
  {
    c.result = c.socket->receive_from(
        {c.data, c.file->nonblocking() || bool(c.flags & MSG_DONTWAIT)}, c.length,
        &c.peer);
  }
  static bool received(Context &c) noexcept { return c.result >= 0; }
  static bool copy(Context &c) noexcept
  {
    return output_address(c.address, c.address_length, c.peer) >= 0;
  }
  static void bad_copy(Context &c) noexcept { c.result = -1; }
  struct Receive;
  struct Copy;
  struct BadCopy;
  using Graph = flow::graph<
      Receive, flow::types<flow::node<Receive, flow::action<&receive>,
                                      flow::branch<flow::condition<&received>,
                                                   flow::go<Copy>, flow::stop>>,
                           flow::node<Copy, flow::noop,
                                      flow::branch<flow::condition<&copy>, flow::stop,
                                                   flow::go<BadCopy>>>,
                           flow::node<BadCopy, flow::action<&bad_copy>, flow::stop>>>;
};
struct SocketOptionFlow
{
  struct Context
  {
    xv::net::Socket *socket;
    int level, option;
    unsigned long value, length;
    unsigned &capacity;
    xv::process::AddressSpace &memory;
    int error = 0, result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.socket && c.level == SOL_SOCKET && c.option == SO_ERROR &&
           c.memory.copy_in(&c.capacity, c.length, sizeof(c.capacity)) >= 0 &&
           c.capacity >= sizeof(int);
  }
  static void consume(Context &c) noexcept
  {
    c.error = c.socket->error();
    c.capacity = sizeof(c.error);
  }
  static bool copy_value(Context &c) noexcept
  {
    return c.memory.copy_out(c.value, &c.error, sizeof(c.error)) >= 0;
  }
  static bool copy_length(Context &c) noexcept
  {
    return c.memory.copy_out(c.length, &c.capacity, sizeof(c.capacity)) >= 0;
  }
  static void success(Context &c) noexcept { c.result = 0; }
  struct Validate;
  struct Consume;
  struct Value;
  struct Length;
  struct Success;
  using Graph = flow::graph<
      Validate,
      flow::types<flow::node<Validate, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Consume>,
                                          flow::stop>>,
                  flow::node<Consume, flow::action<&consume>, flow::go<Value>>,
                  flow::node<Value, flow::noop,
                             flow::branch<flow::condition<&copy_value>,
                                          flow::go<Length>, flow::stop>>,
                  flow::node<Length, flow::noop,
                             flow::branch<flow::condition<&copy_length>,
                                          flow::go<Success>, flow::stop>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

} // namespace

unsigned long sys_socket()
{
  int domain, type, protocol;
  argument_integer(0, &domain);
  argument_integer(1, &type);
  argument_integer(2, &protocol);
  if (domain == AF_UNIX)
  {
    unsigned options = type & (SOCK_NONBLOCK | SOCK_CLOEXEC);
    type &= ~(SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (protocol || (type != SOCK_STREAM && type != SOCK_SEQPACKET))
    { return -EOPNOTSUPP; }
    auto *object = xv::ipc::local_socket(type);
    return object ? install_local(object, options) : -ENOMEM;
  }
  if (domain != AF_INET || (type & ~(SOCK_NONBLOCK | 3)))
  {
    return -1;
  }
  return install(xv::net::create_socket(type & ~SOCK_NONBLOCK, protocol),
                 type & SOCK_NONBLOCK);
}
unsigned long sys_sockbind()
{
  auto *socket = socket_argument();
  unsigned long address;
  int length;
  argument_address(1, &address);
  argument_integer(2, &length);
  if (auto *local = local_argument())
  {
    sockaddr_un value;
    int error = unix_path(address, length, value);
    return error ? error : local->bind_path(value.sun_path);
  }
  xv::net::Endpoint local;
  if (!socket || length < 0)
  {
    return !socket ? socket_argument_error() : -EINVAL;
  }
  SocketBindFlow::Context context{socket, address, length, local};
  (void)SocketBindFlow::Graph::run<0>(context);
  return context.result;
}
unsigned long sys_connect()
{
  xv::fs::File *file;
  auto *socket = socket_argument(&file);
  unsigned long address;
  int length;
  argument_address(1, &address);
  argument_integer(2, &length);
  if (auto *local = local_argument())
  {
    sockaddr_un value;
    int error = unix_path(address, length, value);
    return error ? error : local->connect_path(value.sun_path, file->nonblocking());
  }
  xv::net::Endpoint peer;
  if (!socket || length < 0)
  {
    return !socket ? socket_argument_error() : -EINVAL;
  }
  SocketConnectFlow::Context context{socket, file, address, length, peer};
  (void)SocketConnectFlow::Graph::run<0>(context);
  return context.result;
}
unsigned long sys_listen()
{
  auto *socket = socket_argument();
  int backlog;
  argument_integer(1, &backlog);
  if (auto *local = local_argument()) { return local->listen(backlog); }
  return socket ? socket->listen(backlog) : socket_argument_error();
}
unsigned long sys_accept()
{
  xv::fs::File *file;
  auto *socket = socket_argument(&file);
  unsigned long address, length;
  argument_address(1, &address);
  argument_address(2, &length);
  if (auto *local = local_argument())
  {
    if (address && !length) { return -EFAULT; }
    int error = 0;
    auto *child = local->accept(file->nonblocking(), error);
    if (!child) { return error; }
    if (address)
    {
      error = unix_name(address, length, *xv::io::query<xv::ipc::LocalSocket>(child), true);
      if (error) { child->close(); return error; }
    }
    return install_local(child, 0);
  }
  if (!socket)
  {
    return socket_argument_error();
  }
  SocketAcceptFlow::Context context{socket, file, address, length};
  (void)SocketAcceptFlow::Graph::run<0>(context);
  return context.result;
}
unsigned long sys_sendto()
{
  xv::fs::File *file;
  auto *socket = socket_argument(&file);
  unsigned long data, address;
  int length, flags, address_length;
  argument_address(1, &data);
  argument_integer(2, &length);
  argument_integer(3, &flags);
  argument_address(4, &address);
  argument_integer(5, &address_length);
  if (auto *local = local_argument())
  {
    if (length < 0)
    {
      return -EINVAL;
    }
    if (address || address_length)
    {
      return -EISCONN;
    }
    if (file->nonblocking())
    {
      flags |= MSG_DONTWAIT;
    }
    iovec vector{reinterpret_cast<void *>(data), unsigned(length)};
    return local->send(&vector, 1, true, flags, {});
  }
  if (!socket || length < 0 || (flags & ~MSG_DONTWAIT) || address_length < 0)
  {
    return !socket ? socket_argument_error() : -EINVAL;
  }
  xv::net::Endpoint peer;
  SocketSendFlow::Context context{socket, file,  data,           address,
                                  length, flags, address_length, peer};
  (void)SocketSendFlow::Graph::run<0>(context);
  return context.result;
}
unsigned long sys_recvfrom()
{
  xv::fs::File *file;
  auto *socket = socket_argument(&file);
  unsigned long data, address, address_length;
  int length, flags;
  argument_address(1, &data);
  argument_integer(2, &length);
  argument_integer(3, &flags);
  argument_address(4, &address);
  argument_address(5, &address_length);
  if (auto *local = local_argument())
  {
    if (length < 0)
    {
      return -EINVAL;
    }
    if (file->nonblocking())
    {
      flags |= MSG_DONTWAIT;
    }
    iovec vector{reinterpret_cast<void *>(data), unsigned(length)};
    xv::ipc::Rights discarded;
    unsigned output_flags = 0;
    int result = local->receive(&vector, 1, true, flags, discarded, output_flags);
    xv::ipc::release_rights(discarded);
    if (result >= 0 && address && unix_name(address, address_length, *local, true) < 0)
    {
      return -EFAULT;
    }
    return result;
  }
  if (!socket || length < 0 || (flags & ~MSG_DONTWAIT))
  {
    return !socket ? socket_argument_error() : -EINVAL;
  }
  xv::net::Endpoint peer;
  SocketReceiveFlow::Context context{socket,         file,   data,  address,
                                     address_length, length, flags, peer};
  (void)SocketReceiveFlow::Graph::run<0>(context);
  return context.result;
}
unsigned long sys_shutdown()
{
  auto *socket = socket_argument();
  int how;
  argument_integer(1, &how);
  if (auto *local = local_argument())
  {
    return local->shutdown(how);
  }
  return socket ? socket->shutdown(how) : socket_argument_error();
}
unsigned long sys_getsockname()
{
  auto *socket = socket_argument();
  unsigned long address, length;
  argument_address(1, &address);
  argument_address(2, &length);
  if (auto *local = local_argument())
  {
    return unix_name(address, length, *local, false);
  }
  return !socket ? socket_argument_error()
         : !address ? -EFAULT : output_address(address, length, socket->local());
}
unsigned long sys_getpeername()
{
  auto *socket = socket_argument();
  unsigned long address, length;
  argument_address(1, &address);
  argument_address(2, &length);
  if (auto *local = local_argument())
  {
    return unix_name(address, length, *local, true);
  }
  if (!socket || !address)
  {
    return !socket ? socket_argument_error() : -EFAULT;
  }
  auto peer = socket->peer();
  return peer.port ? output_address(address, length, peer) : -ENOTCONN;
}
unsigned long sys_getsockopt()
{
  auto *socket = socket_argument();
  int level, option;
  unsigned long value, length;
  argument_integer(1, &level);
  argument_integer(2, &option);
  argument_address(3, &value);
  argument_address(4, &length);
  unsigned capacity;
  auto &memory = xv::process::current()->memory();
  if (auto *local = local_argument())
  {
    if (level != SOL_SOCKET || (option != SO_ERROR && option != SO_TYPE))
    { return -EOPNOTSUPP; }
    if (memory.copy_in(&capacity, length, sizeof(capacity)) < 0) { return -EFAULT; }
    int result = option == SO_TYPE ? local->type() : 0;
    socklen_t actual = sizeof(result);
    if (capacity > actual) { capacity = actual; }
    return memory.copy_out(value, &result, capacity) < 0 ||
           memory.copy_out(length, &actual, sizeof(actual)) < 0 ? -EFAULT : 0;
  }
  if (!socket) { return socket_argument_error(); }
  SocketOptionFlow::Context context{socket, level,    option, value,
                                    length, capacity, memory};
  (void)SocketOptionFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::syscall
