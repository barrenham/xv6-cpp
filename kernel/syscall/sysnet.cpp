module xv.syscall;
import :arguments;
import :handlers;
import xv.net;
namespace xv::syscall
{
unsigned long sys_bind()
{
  int port;
  argument_integer(0, &port);
  return port > 0 && port <= 65535 ? xv::net::bind(port) : -1;
}
unsigned long sys_unbind()
{
  int port;
  argument_integer(0, &port);
  return port > 0 && port <= 65535 ? xv::net::unbind(port) : -1;
}
unsigned long sys_send()
{
  int source, destination, port, length;
  unsigned long data;
  argument_integer(0, &source);
  argument_integer(1, &destination);
  argument_integer(2, &port);
  argument_address(3, &data);
  argument_integer(4, &length);
  if (source <= 0 || source > 65535 || port <= 0 || port > 65535 || length < 0) return -1;
  return xv::net::send(source, static_cast<unsigned>(destination), port, data, length);
}
unsigned long sys_recv()
{
  int port, capacity;
  unsigned long source, source_port, data;
  argument_integer(0, &port);
  argument_address(1, &source);
  argument_address(2, &source_port);
  argument_address(3, &data);
  argument_integer(4, &capacity);
  if (port <= 0 || port > 65535 || capacity < 0) return -1;
  return xv::net::receive(port, source, source_port, data, capacity);
}
}
