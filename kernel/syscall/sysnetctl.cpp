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
  bool query = operation == NETCTL_LINK || operation == NETCTL_ADDRESS || operation == NETCTL_ROUTE ||
               operation == NETCTL_NEIGHBOR || operation == NETCTL_BRIDGE_PORT || operation == NETCTL_BRIDGE_FDB || operation == NETCTL_DHCP_STATUS;
  if (!query && xv::process::current()->credentials().snapshot().effective_uid != 0) return -1;
  int result = xv::net::control(operation, request);
  if (result > 0 && memory.copy_out(address, &request, sizeof(request)) < 0) return -EFAULT;
  return result;
}
}
