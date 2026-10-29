module;
#include "kernel/net/control.h"
#include "kernel/net/errno.h"
module xv.net;
import :control_detail_flows;
import :control_execute_flow;
import :control;

namespace xv::net
{
int NetworkManager::inspect(unsigned operation, netctl_request &request)
{
  InspectFlow::Context ctx{*this, operation, request};
  (void)InspectFlow::Graph::run<0>(ctx);
  return ctx.result;
}

int NetworkManager::mutate(unsigned operation, const netctl_request &r,
                           NetworkInterface &interface)
{
  MutationFlow::Context ctx{*this, operation, r, interface};
  (void)MutationFlow::Graph::run<0>(ctx);
  return ctx.result;
}

int NetworkManager::execute(unsigned operation, netctl_request &request)
{
  ExecuteFlow::Context ctx{*this, operation, request};
  (void)ExecuteFlow::Graph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::net
