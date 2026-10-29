module;
#include "kernel/net/control.h"
#include "kernel/net/errno.h"
#include "kernel/net/filter/control.h"
#include "kernel/support/flow.h"
module xv.syscall;
import :arguments;
import :handlers;
import xv.net;
import xv.process;
namespace xv::syscall
{
namespace
{
namespace flow = xv::flow;
struct FirewallControlFlow
{
  struct Context
  {
    xv::process::AddressSpace &memory;
    int operation;
    unsigned long address;
    firewall_request &request;
    int result = -1;
  };
  static bool permitted(Context &c) noexcept
  {
    return (c.operation == FW_CHAIN || c.operation == FW_RULE) ||
           xv::process::current()->credentials().snapshot().effective_uid == 0;
  }
  static bool copy_request(Context &c) noexcept
  {
    return c.memory.copy_in(&c.request, c.address, sizeof(c.request)) >= 0;
  }
  static void execute(Context &c) noexcept
  {
    c.result = xv::net::firewall_control(c.operation, c.request);
  }
  static bool has_output(Context &c) noexcept { return c.result > 0; }
  static bool copy_result(Context &c) noexcept
  {
    return c.memory.copy_out(c.address, &c.request, sizeof(c.request)) >= 0;
  }
  static void bad_copy(Context &c) noexcept { c.result = -EFAULT; }
  struct Permission;
  struct CopyRequest;
  struct Execute;
  struct CopyResult;
  struct BadCopy;
  using Graph = flow::graph<
      Permission,
      flow::types<flow::node<Permission, flow::noop,
                             flow::branch<flow::condition<&permitted>,
                                          flow::go<CopyRequest>, flow::stop>>,
                  flow::node<CopyRequest, flow::noop,
                             flow::branch<flow::condition<&copy_request>,
                                          flow::go<Execute>, flow::go<BadCopy>>>,
                  flow::node<Execute, flow::action<&execute>,
                             flow::branch<flow::condition<&has_output>,
                                          flow::go<CopyResult>, flow::stop>>,
                  flow::node<CopyResult, flow::noop,
                             flow::branch<flow::condition<&copy_result>, flow::stop,
                                          flow::go<BadCopy>>>,
                  flow::node<BadCopy, flow::action<&bad_copy>, flow::stop>>>;
};
struct NetworkControlFlow
{
  struct Context
  {
    xv::process::AddressSpace &memory;
    int operation;
    unsigned long address;
    netctl_request &request;
    int result = -1;
  };
  static bool permitted(Context &c) noexcept
  {
    return (c.operation == NETCTL_LINK || c.operation == NETCTL_ADDRESS ||
            c.operation == NETCTL_ROUTE || c.operation == NETCTL_NEIGHBOR ||
            c.operation == NETCTL_BRIDGE_PORT || c.operation == NETCTL_BRIDGE_FDB ||
            c.operation == NETCTL_DHCP_STATUS) ||
           xv::process::current()->credentials().snapshot().effective_uid == 0;
  }
  static bool copy_request(Context &c) noexcept
  {
    return c.memory.copy_in(&c.request, c.address, sizeof(c.request)) >= 0;
  }
  static void execute(Context &c) noexcept
  {
    c.result = xv::net::control(c.operation, c.request);
  }
  static bool has_output(Context &c) noexcept { return c.result > 0; }
  static bool copy_result(Context &c) noexcept
  {
    return c.memory.copy_out(c.address, &c.request, sizeof(c.request)) >= 0;
  }
  static void bad_copy(Context &c) noexcept { c.result = -EFAULT; }
  struct CopyRequest;
  struct Permission;
  struct Execute;
  struct CopyResult;
  struct BadCopy;
  using Graph = flow::graph<
      CopyRequest,
      flow::types<flow::node<CopyRequest, flow::noop,
                             flow::branch<flow::condition<&copy_request>,
                                          flow::go<Permission>, flow::go<BadCopy>>>,
                  flow::node<Permission, flow::noop,
                             flow::branch<flow::condition<&permitted>,
                                          flow::go<Execute>, flow::stop>>,
                  flow::node<Execute, flow::action<&execute>,
                             flow::branch<flow::condition<&has_output>,
                                          flow::go<CopyResult>, flow::stop>>,
                  flow::node<CopyResult, flow::noop,
                             flow::branch<flow::condition<&copy_result>, flow::stop,
                                          flow::go<BadCopy>>>,
                  flow::node<BadCopy, flow::action<&bad_copy>, flow::stop>>>;
};

} // namespace

unsigned long sys_netctl()
{
  int operation;
  unsigned long address;
  argument_integer(0, &operation);
  argument_address(1, &address);
  auto &memory = xv::process::current()->memory();
  if (operation >= FW_CHAIN && operation <= FW_ZERO)
  {
    firewall_request request;
    FirewallControlFlow::Context context{memory, operation, address, request};
    (void)FirewallControlFlow::Graph::run<0>(context);
    return context.result;
  }
  netctl_request request;
  NetworkControlFlow::Context context{memory, operation, address, request};
  (void)NetworkControlFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::syscall
