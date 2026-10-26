module;
#include "kernel/net/control.h"
#include "kernel/net/errno.h"
module xv.syscall;
import :arguments;
import :handlers;
import xv.net;
import xv.process;
namespace xv::syscall
{
unsigned long sys_netctl()
{
  int operation; unsigned long address;
  argument_integer(0, &operation); argument_address(1, &address);
  netctl_request request;
  auto &memory = xv::process::current()->memory();
  if (memory.copy_in(&request, address, sizeof(request)) < 0) return -EFAULT;
  int result = xv::net::control(operation, request);
  if (result > 0 && memory.copy_out(address, &request, sizeof(request)) < 0) return -EFAULT;
  return result;
}
}
