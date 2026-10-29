module;
#include "kernel/net/errno.h"
#include "kernel/net/control.h"
module xv.net;
import :dhcp_command_flow;
import :dhcp_receive_flow;
import :dhcp_timing_flow;
import :dhcp;
import :wire;
import :platform;

namespace xv::net
{
void DhcpManager::initialize()
{
  ip_.attach_bootstrap(*this);
}

DhcpManager::Client *DhcpManager::find(NetworkInterface &interface)
{
  for (auto &client : clients_)
  {
    if (client.interface == &interface)
    {
      return &client;
    }
  }
  return nullptr;
}

void DhcpManager::select(Client &client, unsigned long now, unsigned delay)
{
  client.state = DHCP_SELECTING;
  client.xid =
      ++counter_ ^ static_cast<unsigned>(now) ^ wire::read32(client.mac.bytes + 2);
  ++client.serial;
  client.retries = 0;
  client.started = now;
  client.next = now + delay;
  client.lease = {};
  client.renewal = client.rebinding = client.expires = 0;
}

int DhcpManager::execute(unsigned operation, netctl_request &request)
{
  CommandFlow::Context ctx{*this, operation, request};
  (void)CommandFlow::Graph::run<0>(ctx);
  return ctx.result;
}

int DhcpManager::transmit(const Client &c, unsigned type)
{
  TransmitFlow::Context ctx{*this, c, type};
  (void)TransmitFlow::Graph::run<0>(ctx);
  return ctx.result;
}

bool DhcpManager::receive_bootstrap(NetworkInterface &interface, Packet &packet,
                                    const IpDatagram &d)
{
  ReceiveFlow::Context ctx{*this, interface, packet, d};
  (void)ReceiveFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void DhcpManager::tick(unsigned long now)
{
  TickFlow::Context ctx{*this, now};
  (void)TickFlow::Graph::run<0>(ctx);
}
} // namespace xv::net
