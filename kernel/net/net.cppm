module;
#include "kernel/net/control.h"
#include "kernel/net/filter/control.h"
export module xv.net;
export import :filter_hook;
export import :socket;
export import :ethernet_interface;
export import :loopback;
export import :tunnel;
export import :bridge;
export import :control;
export import :dhcp;

export namespace xv::net
{
void initialize(NetworkSetup *setup = nullptr);
int firewall_control(unsigned operation, firewall_request &request);
int control(unsigned operation, netctl_request &request);
const InterfaceRegistry &interfaces();
const RoutingTable &routing_table();
void timer_tick();
} // namespace xv::net
