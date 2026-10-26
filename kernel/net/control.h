#pragma once
#define NETCTL_VERSION 1U
#define NETCTL_LINK 1
#define NETCTL_ADDRESS 2
#define NETCTL_ROUTE 3
#define NETCTL_NEIGHBOR 4
#define NETCTL_SET_LINK 5
#define NETCTL_ADD_ADDRESS 6
#define NETCTL_DEL_ADDRESS 7
#define NETCTL_ADD_ROUTE 8
#define NETCTL_DEL_ROUTE 9
#define NETCTL_FLUSH_NEIGHBORS 10
#define NETCTL_BRIDGE_CREATE 11
#define NETCTL_BRIDGE_DELETE 12
#define NETCTL_BRIDGE_ADD_PORT 13
#define NETCTL_BRIDGE_DEL_PORT 14
#define NETCTL_BRIDGE_PORT 15
#define NETCTL_BRIDGE_FDB 16
#define NETCTL_BRIDGE_FLUSH_FDB 17
#define NETCTL_DHCP_START 18
#define NETCTL_DHCP_STOP 19
#define NETCTL_DHCP_STATUS 20
#define NETCTL_DHCP_RENEW 21
#define NET_CONFIG_DHCP 128U
#define DHCP_OFF 0U
#define DHCP_SELECTING 1U
#define DHCP_REQUESTING 2U
#define DHCP_BOUND 3U
#define DHCP_RENEWING 4U
#define DHCP_REBINDING 5U
#define DHCP_FAILED 6U
#define DHCP_STOPPING 7U
#define NET_LINK_BRIDGE 32U
#define NET_LINK_SLAVE 64U
#define NET_LINK_UP 1U
#define NET_LINK_CARRIER 2U
#define NET_LINK_MAC 4U
#define NET_LINK_TUNNEL 8U
#define NET_ROUTE_AUTOMATIC 16U
struct netctl_request
{
  unsigned version, size;
  unsigned ifindex, index;
  unsigned address, prefix, gateway, metric, source, flags;
  unsigned mtu, address_count;
  char name[16];
  unsigned char mac[6];
  unsigned char reserved[2];
  unsigned long long rx_packets, rx_bytes, rx_dropped;
  unsigned long long tx_packets, tx_bytes, tx_errors;
  unsigned long long expires_ms;
};

static_assert(sizeof(netctl_request) == 128);
