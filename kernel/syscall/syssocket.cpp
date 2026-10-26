module;
#include "kernel/net/socket/socket.h"
#include "kernel/net/errno.h"
module xv.syscall;
import :arguments;
import :handlers;
import xv.net;
import xv.process;
import xv.fs;
namespace xv::syscall
{
namespace
{
xv::net::Socket *socket_argument(xv::fs::File **file = nullptr)
{
  int fd; argument_integer(0, &fd);
  auto *f = xv::process::current()->files().get(fd);
  if (file) *file = f;
  return xv::io::query<xv::net::Socket>(xv::fs::files().object(f));
}
int install(xv::net::Socket *socket, bool nonblocking)
{
  if (!socket) return -1;
  auto *file = xv::fs::files().allocate();
  if (!file) { socket->close(); return -1; }
  xv::fs::files().bind_object(*file, *socket, nonblocking);
  int fd = xv::process::current()->files().install(file);
  if (fd < 0) xv::fs::files().release(file);
  return fd;
}
bool input_address(unsigned long address, unsigned length, xv::net::Endpoint &endpoint)
{
  sockaddr_in value;
  if (length < sizeof(value) || xv::process::current()->memory().copy_in(&value, address, sizeof(value)) < 0 ||
      value.sin_family != AF_INET) return false;
  endpoint = {ntohl(value.sin_addr.s_addr), ntohs(value.sin_port)}; return true;
}
int output_address(unsigned long address, unsigned long length, xv::net::Endpoint endpoint)
{
  if (!address) return 0;
  socklen_t capacity;
  auto &memory = xv::process::current()->memory();
  if (memory.copy_in(&capacity, length, sizeof(capacity)) < 0) return -1;
  sockaddr_in value{}; value.sin_family = AF_INET; value.sin_port = htons(endpoint.port);
  value.sin_addr.s_addr = htonl(endpoint.address);
  socklen_t actual = sizeof(value);
  if (capacity > actual) capacity = actual;
  return memory.copy_out(address, &value, capacity) < 0 || memory.copy_out(length, &actual, sizeof(actual)) < 0 ? -1 : 0;
}
}
unsigned long sys_socket()
{
  int domain, type, protocol;
  argument_integer(0, &domain); argument_integer(1, &type); argument_integer(2, &protocol);
  if (domain != AF_INET || (type & ~(SOCK_NONBLOCK|3))) return -1;
  return install(xv::net::create_socket(type & ~SOCK_NONBLOCK, protocol), type&SOCK_NONBLOCK);
}
unsigned long sys_sockbind()
{
  auto *socket = socket_argument(); unsigned long address; int length;
  argument_address(1, &address); argument_integer(2, &length);
  xv::net::Endpoint local;
  return socket && length >= 0 && input_address(address, length, local) ? socket->bind(local) : -1;
}
unsigned long sys_connect()
{
  xv::fs::File *file;
  auto *socket = socket_argument(&file); unsigned long address; int length;
  argument_address(1, &address); argument_integer(2, &length);
  xv::net::Endpoint peer;
  return socket && length >= 0 && input_address(address, length, peer) ? socket->connect(peer, file->nonblocking()) : -1;
}
unsigned long sys_listen()
{
  auto *socket = socket_argument(); int backlog; argument_integer(1, &backlog);
  return socket ? socket->listen(backlog) : -1;
}
unsigned long sys_accept()
{
  xv::fs::File *file; auto *socket = socket_argument(&file);
  unsigned long address, length; argument_address(1, &address); argument_address(2, &length);
  if (!socket) return -1;
  int error;
  auto *child = socket->accept(file->nonblocking(), error);
  if (!child) return error;
  if (output_address(address, length, child->peer()) < 0) { child->close(); return -1; }
  return install(child, false);
}
unsigned long sys_sendto()
{
  xv::fs::File *file; auto *socket = socket_argument(&file);
  unsigned long data, address; int length, flags, address_length;
  argument_address(1, &data); argument_integer(2, &length); argument_integer(3, &flags);
  argument_address(4, &address); argument_integer(5, &address_length);
  if (!socket || length < 0 || (flags & ~MSG_DONTWAIT) || address_length < 0) return -1;
  xv::net::Endpoint peer;
  if (address && !input_address(address, address_length, peer)) return -1;
  return socket->send_to({data, file->nonblocking() || bool(flags&MSG_DONTWAIT)}, length, address ? &peer : nullptr);
}
unsigned long sys_recvfrom()
{
  xv::fs::File *file; auto *socket = socket_argument(&file);
  unsigned long data, address, address_length; int length, flags;
  argument_address(1, &data); argument_integer(2, &length); argument_integer(3, &flags);
  argument_address(4, &address); argument_address(5, &address_length);
  if (!socket || length < 0 || (flags & ~MSG_DONTWAIT)) return -1;
  xv::net::Endpoint peer;
  int result = socket->receive_from({data, file->nonblocking() || bool(flags&MSG_DONTWAIT)}, length, &peer);
  if (result >= 0 && output_address(address, address_length, peer) < 0) return -1;
  return result;
}
unsigned long sys_shutdown()
{
  auto *socket = socket_argument(); int how; argument_integer(1, &how);
  return socket ? socket->shutdown(how) : -1;
}
unsigned long sys_getsockname()
{
  auto *socket = socket_argument(); unsigned long address, length;
  argument_address(1, &address); argument_address(2, &length);
  return socket && address ? output_address(address, length, socket->local()) : -1;
}
unsigned long sys_getpeername()
{
  auto *socket = socket_argument(); unsigned long address, length;
  argument_address(1, &address); argument_address(2, &length);
  if (!socket || !address) return -1;
  auto peer = socket->peer(); return peer.port ? output_address(address, length, peer) : -ENOTCONN;
}
unsigned long sys_getsockopt()
{
  auto *socket = socket_argument(); int level, option; unsigned long value, length;
  argument_integer(1, &level); argument_integer(2, &option);
  argument_address(3, &value); argument_address(4, &length);
  unsigned capacity; auto &memory = xv::process::current()->memory();
  if (!socket || level != SOL_SOCKET || option != SO_ERROR ||
      memory.copy_in(&capacity, length, sizeof(capacity)) < 0 || capacity < sizeof(int)) return -1;
  int error = socket->error(); capacity = sizeof(error);
  return memory.copy_out(value, &error, sizeof(error)) < 0 || memory.copy_out(length, &capacity, sizeof(capacity)) < 0 ? -1 : 0;
}
}
